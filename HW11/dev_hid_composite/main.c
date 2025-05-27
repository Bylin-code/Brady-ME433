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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"

#include "bsp/board_api.h"
#include "tusb.h"

#include "usb_descriptors.h"

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */

// Define GPIO pins for buttons
#define LEFT_BTN 18
#define RIGHT_BTN 19
#define UP_BTN 20
#define DOWN_BTN 21
#define MODE_BTN 17

// Global variables for button states
bool left_btn_state = false;
bool right_btn_state = false;
bool up_btn_state = false;
bool down_btn_state = false;
bool mode_btn_state = false;
bool mode_btn_prev_state = false;

// Mouse mode: 0 = normal (button control), 1 = circular movement
int mouse_mode = 0;

// Variables to track button press duration
absolute_time_t left_btn_press_time = 0;
absolute_time_t right_btn_press_time = 0;
absolute_time_t up_btn_press_time = 0;
absolute_time_t down_btn_press_time = 0;
bool left_btn_prev_state = false;
bool right_btn_prev_state = false;
bool up_btn_prev_state = false;
bool down_btn_prev_state = false;


enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;

void led_blinking_task(void);
void hid_task(void);

/*------------- MAIN -------------*/
int main(void)
{
  board_init();
  stdio_init_all();

  // init device stack on configured roothub port
  tud_init(BOARD_TUD_RHPORT);

  // Initialize GPIO pins as inputs with pull-ups enabled
  gpio_init(LEFT_BTN);
  gpio_set_dir(LEFT_BTN, GPIO_IN);
  gpio_pull_up(LEFT_BTN);
  
  gpio_init(RIGHT_BTN);
  gpio_set_dir(RIGHT_BTN, GPIO_IN);
  gpio_pull_up(RIGHT_BTN);
  
  gpio_init(UP_BTN);
  gpio_set_dir(UP_BTN, GPIO_IN);
  gpio_pull_up(UP_BTN);
  
  gpio_init(DOWN_BTN);
  gpio_set_dir(DOWN_BTN, GPIO_IN);
  gpio_pull_up(DOWN_BTN);
  
  gpio_init(MODE_BTN);
  gpio_set_dir(MODE_BTN, GPIO_IN);
  gpio_pull_up(MODE_BTN);

  if (board_init_after_tusb) {
    board_init_after_tusb();
  }

  while (1)
  {
    // Update global button states
    left_btn_state = !gpio_get(LEFT_BTN);   // Invert so 1 = pressed
    right_btn_state = !gpio_get(RIGHT_BTN); // Invert so 1 = pressed
    up_btn_state = !gpio_get(UP_BTN);       // Invert so 1 = pressed
    down_btn_state = !gpio_get(DOWN_BTN);   // Invert so 1 = pressed
    mode_btn_state = !gpio_get(MODE_BTN);   // Invert so 1 = pressed
    
    // Track button press durations
    absolute_time_t current_time = get_absolute_time();
    
    // Left button press tracking
    if (left_btn_state && !left_btn_prev_state) {
        left_btn_press_time = current_time; // Record press start time
    }
    left_btn_prev_state = left_btn_state;
    
    // Right button press tracking
    if (right_btn_state && !right_btn_prev_state) {
        right_btn_press_time = current_time; // Record press start time
    }
    right_btn_prev_state = right_btn_state;
    
    // Up button press tracking
    if (up_btn_state && !up_btn_prev_state) {
        up_btn_press_time = current_time; // Record press start time
    }
    up_btn_prev_state = up_btn_state;
    
    // Down button press tracking
    if (down_btn_state && !down_btn_prev_state) {
        down_btn_press_time = current_time; // Record press start time
    }
    down_btn_prev_state = down_btn_state;
    
    // Mode button press tracking - toggle mouse mode when pressed
    if (mode_btn_state && !mode_btn_prev_state) {
        // Toggle between normal mode (0) and circle mode (1)
        mouse_mode = 1 - mouse_mode;
        // Turn on LED to indicate current mode
        board_led_write(mouse_mode);
    }
    mode_btn_prev_state = mode_btn_state;

    tud_task(); // tinyusb device task

    hid_task();
  }
}

//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = tud_mounted() ? BLINK_MOUNTED : BLINK_NOT_MOUNTED;
}

//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

static void send_hid_report(uint8_t report_id, uint32_t btn)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;

  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;

      if ( btn )
      {
        uint8_t keycode[6] = { 0 };
        keycode[0] = HID_KEY_A;

        tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        has_keyboard_key = true;
      }else
      {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_MOUSE:
    {
      int8_t dx = 0;
      int8_t dy = 0;
      int8_t base_gain = 3;
      int8_t left_right_gain = base_gain;
      int8_t up_down_gain = base_gain;
      absolute_time_t current_time = get_absolute_time();
      
      // Check which mode we're in - normal or circular
      if (mouse_mode == 0) {
        // Normal mode - use button controls with dynamic gain
        // Calculate dynamic gain based on button press duration
        if (left_btn_state) {
          uint32_t hold_time_ms = absolute_time_diff_us(left_btn_press_time, current_time) / 1000;
          left_right_gain = base_gain + (hold_time_ms / 100); // Increase gain by 1 for every 100ms held
          if (left_right_gain > 30) left_right_gain = 30; // Cap at 30
        }
        
        if (right_btn_state) {
          uint32_t hold_time_ms = absolute_time_diff_us(right_btn_press_time, current_time) / 1000;
          left_right_gain = base_gain + (hold_time_ms / 100); // Increase gain by 1 for every 100ms held
          if (left_right_gain > 30) left_right_gain = 30; // Cap at 30
        }
        
        if (up_btn_state) {
          uint32_t hold_time_ms = absolute_time_diff_us(up_btn_press_time, current_time) / 1000;
          up_down_gain = base_gain + (hold_time_ms / 100); // Increase gain by 1 for every 100ms held
          if (up_down_gain > 30) up_down_gain = 30; // Cap at 30
        }
        
        if (down_btn_state) {
          uint32_t hold_time_ms = absolute_time_diff_us(down_btn_press_time, current_time) / 1000;
          up_down_gain = base_gain + (hold_time_ms / 100); // Increase gain by 1 for every 100ms held
          if (up_down_gain > 30) up_down_gain = 30; // Cap at 30
        }
        
        // Calculate movement with dynamic gain
        dx = (left_btn_state - right_btn_state) * -1 * left_right_gain;
        dy = (up_btn_state - down_btn_state) * -1 * up_down_gain;
      } else {
        // Circle mode - move mouse in a slow circle
        static uint32_t circle_position = 0;
        
        // Calculate position along the circle using sine and cosine
        // Use circle_position as the angle (in degrees)
        // Convert to radians for sin/cos functions
        float angle_rad = (circle_position % 360) * (3.14159f / 180.0f);
        
        // Calculate x and y coordinates on the circle
        // Scale determines the size of the circle
        float scale = 10.0f;  // Adjust for desired circle size
        dx = (int8_t)(sin(angle_rad) * scale);
        dy = (int8_t)(cos(angle_rad) * scale);
        
        // Increment position for next time (controls speed of rotation)
        circle_position += 2;  // Smaller increment = slower movement
      }

      // no button, calculated movement, no scroll, no pan
      tud_hid_mouse_report(REPORT_ID_MOUSE, 0x00, dx, dy, 0, 0);
    }
    break;

    case REPORT_ID_CONSUMER_CONTROL:
    {
      // use to avoid send multiple consecutive zero report
      static bool has_consumer_key = false;

      if ( btn )
      {
        // volume down
        uint16_t volume_down = HID_USAGE_CONSUMER_VOLUME_DECREMENT;
        tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &volume_down, 2);
        has_consumer_key = true;
      }else
      {
        // send empty key report (release key) if previously has key pressed
        uint16_t empty_key = 0;
        if (has_consumer_key) tud_hid_report(REPORT_ID_CONSUMER_CONTROL, &empty_key, 2);
        has_consumer_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_gamepad_key = false;

      hid_gamepad_report_t report =
      {
        .x   = 0, .y = 0, .z = 0, .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, .buttons = 0
      };

      if ( btn )
      {
        report.hat = GAMEPAD_HAT_UP;
        report.buttons = GAMEPAD_BUTTON_A;
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));

        has_gamepad_key = true;
      }else
      {
        report.hat = GAMEPAD_HAT_CENTERED;
        report.buttons = 0;
        if (has_gamepad_key) tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        has_gamepad_key = false;
      }
    }
    break;

    default: break;
  }
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

  uint32_t const btn = board_button_read();

  // Remote wakeup
  if ( tud_suspended() && btn )
  {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  }else
  {
    // Send the 1st of report chain, the rest will be sent by tud_hid_report_complete_cb()
    send_hid_report(REPORT_ID_MOUSE, btn);
  }
}

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;

  uint8_t next_report_id = report[0] + 1u;

  if (next_report_id < REPORT_ID_COUNT)
  {
    send_hid_report(next_report_id, board_button_read());
  }
}

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

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      }else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}

//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}
