#ifndef led_strip_h
#define led_strip_h

#include <Adafruit_NeoPixel.h>

#define LED_COUNT 114
#define LED_DATA_PIN D6

class LedStrip {
    public:
        void init();
        void flash();

    private:
        int brightness = 60;
        Adafruit_NeoPixel ledStrip = Adafruit_NeoPixel(LED_COUNT, LED_DATA_PIN, NEO_GRBW + NEO_KHZ800);
};

#endif
