#include <Arduino.h>
#include <Scheduler.h>

#include "device.h"
#include "wordclock_config.h"
#include "wifi_portal.h"
#include "led_strip.h"

#include "tasks/blink.h"

Device device;
LedStrip ledStrip;
WifiPortal wifiPortal;

void setup() {
  device.init();
  ledStrip.init();
  wifiPortal.init(device.getName());
  device.initClock();
  device.initOta();

  // Setup blink task
  blink_task.setLedStrip(ledStrip);
  Scheduler.start(&blink_task);

  Scheduler.begin();
}

void loop() {
  if (!wifiPortal.isConnected()) {
    Serial.println("Wifi not connected, restarting");
    ESP.restart();
    delay(2000);
    return;
  }

  device.handleOta();
}
