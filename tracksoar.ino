/* trackuino copyright (C) 2010  EA5HAV Javi
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

// Mpide 22 fails to compile Arduino code because it stupidly defines ARDUINO 
// as an empty macro (hence the +0 hack). UNO32 builds are fine. Just use the
// real Arduino IDE for Arduino builds. Optionally complain to the Mpide
// authors to fix the broken macro.
#if (ARDUINO + 0) == 0
#error "Oops! We need the real Arduino IDE (version 22 or 23) for Arduino builds."
#error "See trackuino.pde for details on this"

// Refuse to compile on arduino version 21 or lower. 22 includes an 
// optimization of the USART code that is critical for real-time operation
// of the AVR code.
#elif (ARDUINO + 0) < 22
#error "Oops! We need Arduino 22 or 23"
#error "See trackuino.pde for details on this"

#endif

// Trackuino custom libs
#include "config.h"
#include "afsk_avr.h"
#include "afsk_pic32.h"
#include "aprs.h"
#include "gps.h"
#include "pin.h"
#include "power.h"
#include "sensors_avr.h"
#include "sensors_pic32.h"
#include <avr/wdt.h>

// Arduino/AVR libs
#if (ARDUINO + 1) >= 100
#  include <Arduino.h>
#else
#  include <WProgram.h>
#endif
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
// Module constants
static const uint32_t VALID_POS_TIMEOUT = 2000;  // ms

// Though not used here, we need to include Wire.h in this file for other code:
#include <Wire.h>
// Same is true for SPI.h
#include <SPI.h>

// Module variables
static int32_t last_beacon_time = 0;
static int32_t last_beacon_course = 0;
static int32_t beacon_rate = 0;
int valid_pos = 0;

void setup()
{
  pinMode(LED_PIN, OUTPUT);
  pin_write(LED_PIN, LOW);
  
  watchdogSetup();
  
  // deactivate internal pull-ups for twi
  // as per note from atmega8 manual pg167
  cbi(PORTC, 4);
  cbi(PORTC, 5);

  Serial.begin(GPS_BAUDRATE);

  afsk_setup();
  gps_setup();
  sensors_setup();
}

void get_pos()
{
  // Get a valid position from the GPS
  uint32_t timeout = millis();
  do {
    if (Serial.available())
      valid_pos = gps_decode(Serial.read());
  } while ( (millis() - timeout < VALID_POS_TIMEOUT) && ! valid_pos) ;
}

void loop()
{
  wdt_reset();
  get_pos();

  if (valid_pos) {

    // Moving slowly or not moving at all
    if (gps_speed < BEACON_SLOW_SPEED) {
      beacon_rate = BEACON_SLOW_RATE;
    } 
    
    else {
      
      // Moving VERY fast
      if (gps_speed > BEACON_FAST_SPEED) {
        beacon_rate = BEACON_FAST_RATE;
      }
      // Moving medium speed
      else {
        beacon_rate = BEACON_FAST_RATE * BEACON_FAST_SPEED / gps_speed;
      }

      // Corner pegging
      if ( abs(last_beacon_course - gps_course) > BEACON_MIN_TURN_ANGLE + BEACON_TURN_SLOPE / gps_speed &&
        millis() - last_beacon_time > BEACON_MIN_TURN_TIME) {

        last_beacon_time = beacon_rate;
      }
    }

    if (millis() - last_beacon_time > beacon_rate) {
      last_beacon_time = millis();
      last_beacon_course = gps_course;
      aprs_send();
      wdt_reset();
      while (afsk_flush()) {
        power_save();
        wdt_reset();
      }
    }
  }
}

void watchdogSetup(void)
{
  cli();
  wdt_reset();
  /*
  WDTCSR configuration:
  WDIE = 1: Interrupt Enable
  WDE = 1 :Reset Enable
  See table for time-out variations:
  WDP3 = 0 :For 1000ms Time-out
  WDP2 = 1 :For 1000ms Time-out
  WDP1 = 1 :For 1000ms Time-out
  WDP0 = 0 :For 1000ms Time-out
  */
  // Enter Watchdog Configuration mode:
  WDTCSR |= (1<<WDCE) | (1<<WDE);
  // Set Watchdog settings:
  WDTCSR = (0<<WDIE) | (1<<WDE) | (1<<WDP3) | (0<<WDP2) | (0<<WDP1) | (1<<WDP0);
  Serial.println("WDT reset");
  sei();
}
