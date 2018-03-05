# PLEASE NOTE:

- Never actually worked as tested. The tracksoar's uBlox GPS does NOT have a battery backup and a very tiny passive antenna, and takes anywhere from 10 to 30 minutes to acquire a GPS lock. Even then the antenna is very bad at keeping a lock. I scrapped the tracksoar and went with a Raspberry Pi based solution with a better GPS chip.
- Repository mostly just exists for my archive purposes.


## Trackuino/Tracksoar Enhanced Firmware
Modified Trackuino/Tracksoar firmware for tracking aircraft and other vehicles.

Developed for a Tracksoar v1.2 device. 

# Select improvements:
- [SmartBeaconing and CornerPegging](http://www.hamhud.net/hh2/smartbeacon.html). [More details here](http://info.aprs.net/index.php?title=SmartBeaconing).
- [Configure APRS symbols](https://github.com/wb2osz/direwolf/blob/612c2dc92887f393c84215d07bf374e045e569b6/symbols-new.txt) in config.cpp instead of hardcoding them in aprs.cpp.
- Limit sending sensor data and comments every 5 or 10 minutes.
- Config option for disabling GPS power saving mode.
- Removed buzzer code.
- Removed APRS radio slotting.
