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

#include "config.h"
#include "ax25.h"
#include "gps.h"
#include "aprs.h"
#include "sensors_avr.h"
#include "sensors_pic32.h"
#include <stdio.h>
#include <stdlib.h>
#if (ARDUINO + 1) >= 100
#  include <Arduino.h>
#else
#  include <WProgram.h>
#endif


#define _ENABLE_BME280_PRESSURE  
#define _ENABLE_BME280_TEMPERATURE
#define _ENABLE_BME280_HUMIDITY

// Module variables
static int32_t last_telemetry_time = 0;
static int32_t last_comment_time = 0;

// Module functions
float meters_to_feet(float m)
{
  // 10000 ft = 3048 m
  return m / 0.3048;
}

// Exported functions

// Send an APRS packet
// Example line looks like something like:
//    KD9KEO-9>APZ001,WIDE1-1,WIDE2-2:/235619h4304.95N/08912.63W>000/003/A=000859/Pa=98573/Rh=69.98/Ti=23.43 Tracksoar v1.2
void aprs_send()
{
  char temp[12];                   // Temperature (int/ext)
  const struct s_address addresses[] = { 
    {D_CALLSIGN, D_CALLSIGN_ID},  // Destination callsign
    {S_CALLSIGN, S_CALLSIGN_ID},  // Source callsign (-11 = balloon, -9 = car)
#ifdef DIGI_PATH1
    {DIGI_PATH1, DIGI_PATH1_TTL}, // Digi1 (first digi in the chain)
#endif
#ifdef DIGI_PATH2
    {DIGI_PATH2, DIGI_PATH2_TTL}, // Digi2 (second digi in the chain)
#endif
  };

  ax25_send_header(addresses, sizeof(addresses)/sizeof(s_address));
  ax25_send_byte('/');                // Report w/ timestamp, no APRS messaging. $ = NMEA raw data
  // ax25_send_string("021709z");     // 021709z = 2nd day of the month, 17:09 zulu (UTC/GMT)
  ax25_send_string(gps_time);         // 170915 = 17h:09m:15s zulu (not allowed in Status Reports)
  ax25_send_byte('h');
  ax25_send_string(gps_aprs_lat);     // Lat: 38deg and 22.20 min (.20 are NOT seconds, but 1/100th of minutes)
  ax25_send_byte(APRS_SYMBOL1);                // Symbol table
  ax25_send_string(gps_aprs_lon);     // Lon: 000deg and 25.80 min
  ax25_send_byte(APRS_SYMBOL2);                // Symbol: O=balloon, -=QTH
  snprintf(temp, 4, "%03d", (int)(gps_course + 0.5)); 
  ax25_send_string(temp);             // Course (degrees)
  ax25_send_byte('/');                // and
  snprintf(temp, 4, "%03d", (int)(gps_speed + 0.5));
  ax25_send_string(temp);             // speed (knots)
  ax25_send_string("/A=");            // Altitude (feet). Goes anywhere in the comment area
  snprintf(temp, 7, "%06ld", (long)(meters_to_feet(gps_altitude) + 0.5));
  ax25_send_string(temp);

  // Only send sensor data when we need to
  if (millis() / 1000 - APRS_SENSOR_DATA_INTERVAL > last_telemetry_time) {

#ifdef _ENABLE_BME280_PRESSURE  
  // Pressure: "/Pa=12345"
  ax25_send_string("/Pa=");
  snprintf(temp, 6, "%ld", sensors_pressure());
  ax25_send_string(temp);
#endif  //_ENABLE_BME280_PRESSURE

#ifdef _ENABLE_BME280_HUMIDITY
  // Humidity: "/Rh=84.56"
  ax25_send_string("/Rh=");
  dtostrf(sensors_humidity(), -1, 2, temp);
  ax25_send_string(temp);
#endif  //_ENABLE_BME280_HUMIDITY  

#ifdef _ENABLE_BME280_TEMPERATURE
  // Temperature
  // "Ti=-8.70"
  ax25_send_string("/Ti=");
  dtostrf(sensors_temperature(), -1, 2, temp);
  ax25_send_string(temp);
#endif  //_ENABLE_BME280_TEMPERATURE


    last_telemetry_time = millis() / 1000;
  }

  if (millis() / 1000 - APRS_COMMENT_INTERVAL > last_comment_time) {
    ax25_send_byte(' ');
    ax25_send_string(APRS_COMMENT);     // Comment

    last_comment_time = millis() / 1000;
  }
  
  ax25_send_footer();
  ax25_flush_frame();                 // Tell the modem to go
}
