#include "device.h"

void Device::init() {
      // setup serial to print messages to
      Serial.begin(115200);

      // Set onboard LED as output
      pinMode(LED_BUILTIN, OUTPUT);

      // generate device name based on chip id
      generateName();

};

char *Device::getName() {
  return name;
}

// Generate device name based on template + chipId
void Device::generateName() {
  sprintf(name, DEVICE_NAME_TEMPLATE, ESP.getChipId());
  Serial.printf("Device name generated: %s\n", name);
}

// Setup time sync using NTP
void Device::initClock() {
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

// Configure over the air (OTA) updates
void Device::initOta() {
  ArduinoOTA.setHostname(name);

  ArduinoOTA.onStart([]() { Serial.println("OTA Start"); });
  ArduinoOTA.onStart([]() { Serial.println("OTA End"); });

  ArduinoOTA.onProgress([] (unsigned int progress, unsigned int total) {
    char buf[32];
    memset(buf, '\0', sizeof(buf));
    snprintf(buf, sizeof(buf) - 1, "Upgrade - %02u%%\n", (progress / (total / 100)));
    Serial.println(buf);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.println("Error - ");

    if (error == OTA_AUTH_ERROR)
      Serial.println("Auth Failed\n");
    else if (error == OTA_BEGIN_ERROR)
      Serial.println("Begin Failed\n");
    else if (error == OTA_CONNECT_ERROR)
      Serial.println("Connect Failed\n");
    else if (error == OTA_RECEIVE_ERROR)
      Serial.println("Receive Failed\n");
    else if (error == OTA_END_ERROR)
      Serial.println("End Failed\n");
  });

  ArduinoOTA.begin();
}

// Handle OTA updates
void Device::handleOta() {
  ArduinoOTA.handle();
}
