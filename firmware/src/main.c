/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <pico/stdlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardware/adc.h"

#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"

#include "../include/tiny-json.h"
#include "../include/stringmap.h"

#define TU_BIT(n)              (1UL << (n))

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

#define BUF_SIZE 1024
char buf[BUF_SIZE];
int counter = 0;

char default_json[] = "{ \
    \"btn6\": \"dpad_u\", \
    \"btn7\": \"dpad_r\", \
    \"btn8\": \"dpad_d\", \
    \"btn9\": \"dpad_l\", \
    \"btn5\": \"l3\", \
    \"btn4\": \"a\", \
    \"btn3\": \"b\", \
    \"btn0\": \"select\", \
    \"btn1\": \"start\" \
}";

const int pins[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
int keymap[15] = {-1};
int dpad[4] = {-1}; // Up, right, down, left

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

void hid_task(void);
void cdc_task(void);
int setup_from_json(char* buf);

/*------------- MAIN -------------*/
int main(void)
{
  for (int i = 0; i < sizeof(pins)/sizeof(pins[0]); i++){
    gpio_init(pins[i]);
    gpio_set_dir(pins[i], GPIO_IN);
    gpio_pull_up(pins[i]);
  }

  board_init();
  tusb_init();
  stdio_init_all();

  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(27);

  if (setup_from_json(default_json) != 0) {
    tud_cdc_write("Invalid JSON!", 13);
    tud_cdc_write_flush();
  }

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

int get_dpad_dir(){
  int output[4] = {0}; // Up, right, down, left

  for (int i = 0; i < 4; i++){
    if (dpad[i] > -1){
      output[i] = !gpio_get(dpad[i]); // Invert because of pull-up
    }
  }

  // Diagonal directions
  if (output[0] && output[1]) return 2; // Up-Right
  if (output[2] && output[1]) return 4; // Down-Right
  if (output[2] && output[3]) return 6; // Down-Left
  if (output[0] && output[3]) return 8; // Up-Left

  // Straight directions
  if (output[0]) return 1; // Up
  if (output[1]) return 3; // Right
  if (output[2]) return 5; // Down
  if (output[3]) return 7; // Left

  return 0; // No direction pressed
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

  adc_select_input(1);
  report.x = -(adc_read() >> 4) + 128; // TODO: Add a deadzone
  adc_select_input(0);
  report.y = (adc_read() >> 4) - 128;
  report.hat = get_dpad_dir();
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
        if (key > -1) dpad[key] = pin;
      }
    }
    return 0;
  }
}

void cdc_task(void){
  if (tud_cdc_available()) {
    int count = tud_cdc_read(&buf[counter], 1); // TODO: Test bigger read sizes

    if (buf[counter] == 0x04){
      if (setup_from_json(buf) != 0) {
        tud_cdc_write("Invalid JSON!", 13);
        tud_cdc_write_flush();
      }
      memset(buf, 0, sizeof(buf));
      counter = 0;
      // tud_cdc_read_flush(); // TODO: test this
    }
    else{
      counter += count; // TODO: Test bigger read sizes
    }

    if (counter + 1 >= BUF_SIZE){
      *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004; // Reset itself, don't know how to handle it
    }
  }
}

// TODO Test this and clean it

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
// void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
// {
//   (void) instance;
//   (void) len;

//   uint8_t next_report_id = report[0] + 1;

//   if (next_report_id < REPORT_ID_COUNT)
//   {
//     send_hid_report(get_all_buttons());
//   }
// }

// This is needed even tho it doesn't do anything due to callbacks
// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

// This is needed even tho it doesn't do anything due to callbacks
// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) bufsize;
}

// Invoked when cdc when line state changed e.g connected/disconnected
void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts) 
{
	(void)itf;
	(void)rts; // TODO: Try resetting

	if (dtr) {
		tud_cdc_write_str("Connected\n");
    tud_cdc_write_flush();
    counter = 0;
	}
}