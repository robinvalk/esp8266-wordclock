#include "led_strip.h"

void LedStrip::init() {
    ledStrip.begin();
    ledStrip.setBrightness(brightness);
    ledStrip.show(); // Turn OFF all pixels ASAP
};

void LedStrip::flash() {
    int wait = 100;

    ledStrip.fill(ledStrip.Color(0, 0, 0, ledStrip.gamma8(255)));
    ledStrip.show();
    delay(wait);

    ledStrip.clear();
    ledStrip.show();
    delay(wait);
}
