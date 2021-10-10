# esp8266-wordclock

## Possible improvements

- Move to platform.io and start using C++
- Move code into seperate files / classes
- Make some sort of interface whereby the implementations will be triggered every second by the `PolledTimeout`
    - One implementation would be to check the displayed time needs updating (True every minute)
    - Another implementation would be to check if the christmas tree needs to show up (On whole hours, from december X until second christmas day)
    - Another implementation would be to display the new year animation (At 00:00 on new year's eve or maybe every hour on new year's eve)
- Check if the webserver can send a response to the client before it starts doing the triggered action

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
Possible use case would be to check every second if the displayed time needs updating.