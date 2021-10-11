#ifndef device_h
#define device_h

#include <Arduino.h>
#include <ArduinoOTA.h>
#include "wordclock_config.h"

struct device_info_t {
  char name[20];
};

class Device {
  public:
    void init();
    char *getName();
    void initClock();
    void initOta();
    void handleOta();
  private:
    char name[20];
    void generateName();
};

#endif
