#include <pico/stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"
#include "hardware/flash.h"
#include "hardware/sync.h"

// USB related includes
#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

// Usage specific
#include "tiny-json.h"
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

char default_json[] = "{\"btn6\": \"dpad_u\", \"btn7\": \"dpad_r\", \"btn8\": \"dpad_d\", \"btn9\": \"dpad_l\", \
\"btn5\": \"l3\", \"btn4\": \"a\", \"btn3\": \"b\", \"btn0\": \"select\", \"btn1\": \"start\"}";

static_assert(sizeof(default_json) / sizeof(default_json[0]) < BUF_SIZE, "BUF_SIZE mismatch!"); // Ensure that default_json never is more than 1024 otherwise risk flash damage from constant rewriting

const int pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int keymap[15] = {-1};
int dpad_keymap[4] = {-1}; // Up, right, down, left

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

void hid_task(void);
void cdc_task(void);
int setup_from_json(char* buf);

/*------------- MAIN -------------*/
int main(void)
{
  // LED mostly for debugging purposes
  const uint LED_PIN = PICO_DEFAULT_LED_PIN;
  gpio_init(LED_PIN);
  gpio_set_dir(LED_PIN, GPIO_OUT);

  memcpy(runtime_json, persistent_json, BUF_SIZE);
  runtime_json[BUF_SIZE-1] = '\0';

  if (setup_from_json(runtime_json) != 0){ // If json in flash is not valid, replace it with default_json
    save_string(PERSISTENT_TARGET_ADDR, default_json, BUF_SIZE);
    gpio_put(LED_PIN, 1);
    sleep_ms(2000);
    *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004; // Reset after saving default
  }

  for (int i = 0; i < sizeof(pins)/sizeof(pins[0]); i++){
    gpio_init(pins[i]);
    gpio_set_dir(pins[i], GPIO_IN);
    gpio_pull_up(pins[i]);
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

  int8_t adc_out = 0;
  uint8_t deadzone = 32;
  adc_select_input(1);
  adc_out = (adc_read() >> 4) - 128;
  report.x = (((adc_out > -deadzone) && (adc_out < deadzone)) ? 0 : adc_out);
  adc_select_input(0);
  adc_out = (adc_read() >> 4) - 128;
  report.y = (((adc_out > -deadzone) && (adc_out < deadzone)) ? 0 : adc_out);
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

// Parse JSON from string and map keys to pins
int setup_from_json(char* buf){
  json_t mem[32]; // This buffer size should be enough because the RP2040 only has 30 IO pins
  json_t const* json = json_create(buf, mem, sizeof mem / sizeof *mem );
  if ( !json ) {
    return 1;
  }
  else{
    json_t const* child;
    for(child = json_getChild( json ); child != 0; child = json_getSibling( child )) {
      int pin = get_pin(json_getName(child));
      int key = find_mapping(json_getValue(child));
      if (key > -1) keymap[key] = pin;
      else {
        int key = find_dpad(json_getValue(child));
        if (key > -1) dpad_keymap[key] = pin;
      }
    }
    return 0;
  }
}

void cdc_task(void){
  if (tud_cdc_available()) {
    int count = tud_cdc_read(&buf[counter], 1); // TODO: Test bigger read sizes

    if (buf[counter] == 0x04){
      strcpy(runtime_json,buf); // Copy buf to another place in memory because setup_from_json somehow destroys the original data
      if (setup_from_json(runtime_json) != 0) {
        tud_cdc_write_str("Invalid JSON!\n");
        tud_cdc_write_flush();
      }
      else{
        save_string(PERSISTENT_TARGET_ADDR, buf, BUF_SIZE);
        tud_cdc_write_str("Saving to persistent storage!\n");
        tud_cdc_write_flush();
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
      tud_cdc_write_str("Connected\n");
      tud_cdc_write_str(persistent_json);
      tud_cdc_write_flush();
      counter = 0;
	}
}
