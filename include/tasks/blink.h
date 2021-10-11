#include "Scheduler.h"
#include "led_strip.h"

class BlinkTask : public Task {
    public:
        void setLedStrip(LedStrip strip) {
            ledStrip = strip;
        }

    protected:
        void setup() {

        }

        void loop() {
            ledStrip.flash();
            Serial.println("Flash!");
            delay(1000);
        }

    private:
        LedStrip ledStrip;

} blink_task;
