// clang-format off

#ifndef _LOPY_H
#define _LOPY_H

#include <stdint.h>

// Hardware related definitions for Pycom LoPy Board (NOT LoPy4)

#define HAS_LORA 1       // comment out if device shall not send data via LoRa
#define CFG_sx1272_radio 1
#define HAS_LED NOT_A_PIN // LoPy4 has no on board mono LED, we use on board RGB LED
#define HAS_RGB_LED (0)  // WS2812B RGB LED on GPIO0

// Note: Pins for LORA chip SPI interface come from board file pins_arduino.h

// select WIFI antenna (internal = onboard / external = u.fl socket)
#define HAS_ANTENNA_SWITCH  (16)      // pin for switching wifi antenna
#define WIFI_ANTENNA 0              // 0 = internal, 1 = external

// uncomment this only if your LoPy runs on a PYTRACK BOARD
#define HAS_GPS 1
#define GPS_I2C GPIO_NUM_25, GPIO_NUM_26 // SDA (P22), SCL (P21)
#define GPS_ADDR 0x10

// uncomment this only if your LoPy runs on a EXPANSION BOARD
//#define HAS_LED (12) // use if LoPy is on Expansion Board, this has a user LED
//#define LED_ACTIVE_LOW 1 // use if LoPy is on Expansion Board, this has a user LED
//#define HAS_BUTTON (13) // user button on expansion board
//#define BUTTON_PULLUP 1  // Button need pullup instead of default pulldown
//#define HAS_BATTERY_PROBE ADC1_GPIO39_CHANNEL // battery probe GPIO pin -> ADC1_CHANNEL_7
//#define BATT_FACTOR 2 // voltage divider 1MOhm/1MOhm -> expansion board 3.0
//#define BATT_FACTOR 4 // voltage divider 115kOhm/56kOhm -> expansion board 2.0

#endif
