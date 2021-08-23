# esp8266-wordclock

## Required libraries

- Adafruit_NeoPixel
- ESP8266WiFi
- ESP8266WebServer
- ESP8266mDNS
- WiFiManager

## References

### NTP Time Sync

https://github.com/esp8266/Arduino/blob/master/libraries/esp8266/examples/NTP-TZ-DST/NTP-TZ-DST.ino

Really clear example on how to setup NTP time synchronisation and what options are available. Not a lot is required at all, see bottom of example sketch. We should update our `sntp_startup_delay_MS_rfc_not_less_than_60000` implementation to not be below 60 seconds. Also the `esp8266::polledTimeout::periodicMs` is interesting, see below.

### Polled Timeout

https://github.com/esp8266/Arduino/blob/master/cores/esp8266/PolledTimeout.h

Noticed in the NTP example this exists. Interisting functions to trigger things periodically, see bottom of file for available functions. Might come in handy, not a lot of documentation available apart from the source code itself.