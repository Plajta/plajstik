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

int keys[][2] = {{5, 13}, {4, 0}}; // Pin, bit shift
int dpad[] = {6, 7, 8, 9}; // Up, right, down, left

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

void hid_task(void);

/*------------- MAIN -------------*/
int main(void)
{
  for (int i = 0; i < sizeof(keys)/sizeof(keys[0]); i++){
    gpio_init(keys[i][0]);
    gpio_set_dir(keys[i][0], GPIO_IN);
    gpio_pull_up(keys[i][0]);
  }

  for (int i = 0; i < 4; i++){
    gpio_init(dpad[i]);
    gpio_set_dir(dpad[i], GPIO_IN);
    gpio_pull_up(dpad[i]);
  }

  board_init();
  tusb_init();
  stdio_init_all();

  adc_init();
  adc_gpio_init(26);
  adc_gpio_init(27);

  while (1)
  {
    tud_task(); // tinyusb device task
    // tud_cdc_write_str("Test\n");

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
  for (int i = 0; i < sizeof(keys)/sizeof(keys[0]); i++){
    output |= (!gpio_get(keys[i][0]) << keys[i][1]);
  }
  return output; // Negate because of pull ups on the pins
}

int get_dpad_dir(){
  int output[4]; // Up, right, down, left

  for (int i = 0; i < 4; i++){
    output[i] = !gpio_get(dpad[i]); // Invert because of pull-up
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

    adc_select_input(0);
    report.x = (adc_read() >> 4) - 128;
    adc_select_input(1);
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

// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}

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
	(void)rts;

	if (dtr) {
		tud_cdc_write_str("Connected\n");
	}
}