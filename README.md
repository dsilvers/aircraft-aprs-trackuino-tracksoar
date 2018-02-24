## Trackuino/Tracksoar Enhanced Firmware
Modified Trackuino/Tracksoar firmware for tracking aircraft and other vehicles.

Tested on a Tracksoar v1.2 device. We are using it with a small aircraft, therefore we need a bunch of these enhancements.

# Select improvements:
- [SmartBeaconing and CornerPegging](http://www.hamhud.net/hh2/smartbeacon.html). [More details here](http://info.aprs.net/index.php?title=SmartBeaconing).
- [Configure APRS symbols](https://github.com/wb2osz/direwolf/blob/612c2dc92887f393c84215d07bf374e045e569b6/symbols-new.txt) in config.cpp instead of hardcoding them in aprs.cpp.
- Limit sending sensor data and comments every 5 or 10 minutes.
- Config option for disabling GPS power saving mode.
- Removed buzzer code.
- Removed APRS radio slotting.
