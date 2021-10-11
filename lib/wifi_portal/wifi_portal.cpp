#include "Arduino.h"
#include "wifi_portal.h"

void WifiPortal::init(const char *apName) {
  if (!wifiManager.autoConnect(apName)) {
    Serial.println("Failed to connect and hit timeout, restarting");
    ESP.restart();
    delay(1000);
  }

  Serial.printf("IP address of device: ");
  Serial.println(WiFi.localIP());
}

bool WifiPortal::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}
