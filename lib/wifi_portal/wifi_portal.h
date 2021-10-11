#ifndef wifi_portal_h
#define wifi_portal_h

#include <Arduino.h>
#include <WiFiManager.h>

class WifiPortal {
    public:
        void init(const char *apName);
        static bool isConnected();

    private:
        WiFiManager wifiManager;
};

#endif
