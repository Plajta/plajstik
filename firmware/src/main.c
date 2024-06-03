#include <pico/stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "hardware/adc.h"
#include "hardware/flash.h"

// USB related includes
#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

// Usage specific
#include "maps.h"
#include "utils.h"
#include "tusb_callbacks.h"

// Pointer to the flash space reserved for persistent JSON storage
extern uint32_t ADDR_PERSISTENT[];
#define PERSISTENT_BASE_ADDR (uint32_t)(ADDR_PERSISTENT)
#define PERSISTENT_TARGET_ADDR (PERSISTENT_BASE_ADDR-XIP_BASE)

// USB and storage buffer size, FLASH_PAGE_SIZE is 256
#define BUF_SIZE FLASH_PAGE_SIZE*4

static_assert(BUF_SIZE <= 4096, "BUF_SIZE too big!"); // Ensure that BUF_SIZE is never more than 4K

/*
BUTTON                 BIT SHIFT
---------------------------
BUTTON_A               0
BUTTON_B               1
BUTTON_X               3
BUTTON_Y               4

BUTTON_L1              6
BUTTON_R1              7
BUTTON_L2              8
BUTTON_R2              9

BUTTON_SELECT          10
BUTTON_START           11

BUTTON_L3              13
BUTTON_R3              14
*/

char buf[BUF_SIZE];
char *persistent_json = (char *)PERSISTENT_BASE_ADDR;
char runtime_json[BUF_SIZE];
int counter = 0;

char default_json[] = "{\"version\":1,\"buttons\":{\"select\":0,\"start\":1,\"b\":3,\"a\":4,\"l3\":5,\"dpad_u\":6,\"dpad_r\":7,\"dpad_d\":8,\"dpad_l\":9},\"deadzone\":16.0,\"axes\":{\"x\":1,\"y\":0}}";

static_assert(sizeof(default_json) / sizeof(default_json[0]) < BUF_SIZE, "BUF_SIZE mismatch!"); // Ensure that default_json never is more than 1024 otherwise risk flash damage from constant rewriting

int8_t keymap[15] = {[0 ... 14] = -1};
int8_t dpad_keymap[4] = {[0 ... 3] = -1}; // Up, right, down, left

int8_t x_adc = -1, y_adc = -1;

double deadzone = 0.;

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

void hid_task(void);
void cdc_task(void);

/*------------- MAIN -------------*/
int main(void)
{
  // LED mostly for debugging purposes
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  memcpy(runtime_json, persistent_json, BUF_SIZE);
  runtime_json[BUF_SIZE-1] = '\0';

  if (json_validity(runtime_json) != 0){ // If json in flash is not valid, replace it with default_json
    save_string(PERSISTENT_TARGET_ADDR, default_json, BUF_SIZE);
    gpio_put(LED_PIN, 1);
    sleep_ms(2000);
    *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004; // Reset after saving default
  }

  memcpy(runtime_json, persistent_json, BUF_SIZE); // Copy again because TinyJSON destorys the original for some reason
  runtime_json[BUF_SIZE-1] = '\0';
  json_setup(runtime_json, keymap, dpad_keymap, &deadzone, &x_adc, &y_adc);

  for (int i = 0; i < sizeof(keymap)/sizeof(keymap[0]); i++){
    if (keymap[i] > -1){
      gpio_init(keymap[i]);
      gpio_set_dir(keymap[i], GPIO_IN);
      gpio_pull_up(keymap[i]);
    }
  }

  for (int i = 0; i < sizeof(dpad_keymap)/sizeof(dpad_keymap[0]); i++){
    if (dpad_keymap[i] > -1){
      gpio_init(dpad_keymap[i]);
      gpio_set_dir(dpad_keymap[i], GPIO_IN);
      gpio_pull_up(dpad_keymap[i]);
    }
  }

  board_init();
  tusb_init();

  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(27);

  while (1){
    tud_task();
    cdc_task();
    hid_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  // TODO: Implement sleep
  (void) remote_wakeup_en;
}

uint32_t get_all_buttons(){
  uint32_t output = 0;
  for (int i = 0; i < sizeof(keymap)/sizeof(keymap[0]); i++){
    if (keymap[i] > -1) {
      output |= (!gpio_get(keymap[i]) << i); // Negate because of pull ups on the pins
    }
  }
  return output;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  hid_gamepad_report_t report =
  {
    .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
    .hat = 0, .buttons = 0
  };

  uint8_t dpad_out[4] = {0}; // Up, right, down, left

  for (uint8_t i = 0; i < 4; i++){
    if (dpad_keymap[i] > -1){
      dpad_out[i] = !gpio_get(dpad_keymap[i]); // Invert because of pull-up
    }
  }

  int8_t x = 0, y = 0;
  if (x_adc > -1){
    adc_select_input(x_adc);
    x = (adc_read() >> 4) - 128;
  }
  if (y_adc > -1){
    adc_select_input(y_adc);
    y = (adc_read() >> 4) - 128;
  }

  if (sqrt((x*x) + (y*y)) > deadzone){
    report.x = x;
    report.y = y;
  }
  else
    report.x = report.y = 0;

  report.hat = find_dpad_dir(dpad_out);
  report.buttons = btn;
  tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
}


// Every 10ms, we will sent 1 report for each HID profile (keyboard, mouse etc ..)
// tud_hid_report_complete_cb() is used to send the next report after previous one is complete
void hid_task(void)
{
  // Poll every 10ms
  const uint32_t interval_ms = 10;
  static uint32_t start_ms = 0;

  if ( board_millis() - start_ms < interval_ms) return; // not enough time
  start_ms += interval_ms;

  uint32_t const btn = get_all_buttons();

  // Remote wakeup
  if ( tud_suspended() && btn > 0 )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(btn);
  }
}

void cdc_task(void){
  if (tud_cdc_available()) {
    int count = tud_cdc_read(&buf[counter], 1); // TODO: Test bigger read sizes

    if (buf[counter] == 0x04){
      strcpy(runtime_json,buf); // Copy buf to another place in memory because TinyJSON somehow destroys the original data
      if (json_validity(runtime_json) != 0) {
        tud_cdc_write_str("Invalid JSON!\n\r");
        tud_cdc_write_flush();
      }
      else{
        save_string(PERSISTENT_TARGET_ADDR, buf, BUF_SIZE);
        tud_cdc_write_str("Saving to persistent storage!\n\r");
        tud_cdc_write_flush();
        *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004; // Reset to load new pins
      }
      memset(buf, 0, sizeof(buf));
      counter = 0;
    }
    else{
      counter += count; // TODO: Test bigger read sizes
    }

    if (counter + 1 >= BUF_SIZE){
      *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004; // Reset itself, don't know how to handle it
    }
  }
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) 
{
	(void)itf;
	(void)rts;

	if (dtr) {
      tud_cdc_write_str("Connected\n\r");
      tud_cdc_write_str(persistent_json);
      tud_cdc_write_str("\n\r\x04");
      tud_cdc_write_flush();
      counter = 0;
	}
}
