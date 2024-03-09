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

/*
 * Modification of dev_hid_composite example to create a simple joystick.
 * 2 axes and 8 buttons. Compatible with the Xbox Adaptive Controller.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#include "hardware/gpio.h"
#include "hardware/adc.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms	 : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
	BLINK_NOT_MOUNTED = 250,
	BLINK_MOUNTED = 1000,
	BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

const int keys[9] = {
	5,2,8,7,6,9,10, // 1-7
	4,3             // START,SELECT
};

const int map1[9] = {
	0,0,0,0,0,0,0,
	1,1
};

const int map2[9] = {
	0,1,2,3,4,5,6,
	0,1
};

const int scr[2] = {
	11,12
};

const int adc[4] = {
    26, 27, 28, 29
};

int scr_mode = 0;

/*------------- MAIN -------------*/
int main(void) {
    stdio_init_all();
	board_init();
	tusb_init();
    adc_init();
	
	for(int i = 0; i < 9; i++) {
		gpio_init(keys[i]);
		gpio_set_dir(keys[i], GPIO_IN);
		gpio_pull_up(keys[i]);
	}

    for(int i = 0; i < 4; i++) {
        adc_gpio_init(adc[i]);
    }

	gpio_init(scr[0]);
	gpio_set_dir(scr[0], GPIO_IN);
	gpio_pull_up(scr[0]);
	
	gpio_init(scr[1]);
	gpio_set_dir(scr[1], GPIO_IN);
	gpio_pull_up(scr[1]);
	
	scr_mode = !gpio_get(keys[8]) ? 1 : 0;

	while (1) {
		tud_task(); // tinyusb device task
		led_blinking_task();
		hid_task();
	}

	return 0;
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void) {
	blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void) {
	blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us	to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en) {
	(void) remote_wakeup_en;
	blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void) {
	blink_interval_ms = BLINK_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

int digi_sc = 1;
int prev_sc = 0;
int digi_ctimer = 0;
int digi_vtimer = 0;

uint8_t ana_sc = 0;
uint8_t prev_a = 255;

void scr_check() {
	if(ana_sc != prev_sc) {
		if(ana_sc > prev_sc) {
			if(ana_sc - prev_sc > 127) {
				digi_sc = 0;
			} else {
				digi_sc = 2;
			}
		} else {
			if(prev_sc - ana_sc  > 127) {
				digi_sc = 2;
			} else {
				digi_sc = 0;
			}
		}
		digi_vtimer = 0;
	} else {
		if(digi_vtimer >= 20) {
			digi_sc = 1;
		} else {
			digi_vtimer++;
		}
	}
	
	prev_sc = ana_sc;
}

void hid_task(void) {
	const uint32_t interval_ms = 1;
	static uint32_t start_ms = 0;
	static HID_JoystickReport_Data_t report;

	if ((board_millis() - start_ms) < interval_ms) return; // not enough time
	start_ms = board_millis() + interval_ms;
	
	// Remote wakeup
	if (tud_suspended()) {
		// Wake up host if we are in suspend mode
		// and REMOTE_WAKEUP feature is enabled by host
		tud_remote_wakeup();
	}

	report.xAxis = 0;
	report.yAxis = 0;
	report.buttons[0] = 0;
	report.buttons[1] = 0;
	report.buttons[2] = 0;
	
	for(int i = 0; i < 9; i++) {
		report.buttons[map1[i]] |= gpio_get(keys[i]) ? 0 : (1 << map2[i]);
	}

    adc_select_input(0);
    report.xAxis = (adc_read() >> 4) - 128;
    adc_select_input(1);
    report.yAxis = (adc_read() >> 4) - 128;

    /*
	uint8_t now_a = !gpio_get(12) ? 1 : 0;
	uint8_t now_b = !gpio_get(11) ? 1 : 0;
	
	if(prev_a != 255) {
		if(now_a != prev_a) {
			if(now_a == now_b) {
				ana_sc++;
			}
			if(now_a != now_b) {
				ana_sc--;
			}
		}
	}
	
	prev_a = !gpio_get(12) ? 1 : 0;

	//if(board_millis() - digi_ctimer > 1) {
		digi_ctimer = board_millis();
		scr_check();
	//}

	if(!scr_mode) {
		report.xAxis = ((int)ana_sc - 128);
	} else {
		report.yAxis = digi_sc == 1 ? 0 : digi_sc == 2 ? 127 : -128;
	}
    */

	/*------------- Joystick -------------*/
	if (tud_hid_ready()) {
		tud_hid_report(0x00, &report, sizeof(report));
	}
}


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
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
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
	// TODO set LED based on CAPLOCK, NUMLOCK etc...
	(void) instance;
	(void) report_id;
	(void) report_type;
	(void) buffer;
	(void) bufsize;
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void) {
	static uint32_t start_ms = 0;
	static bool led_state = false;

	// Blink every interval ms
	if ((board_millis() - start_ms) < blink_interval_ms) return; // not enough time
	start_ms = board_millis() + blink_interval_ms;

	board_led_write(led_state);
	led_state = 1 - led_state; // toggle
}