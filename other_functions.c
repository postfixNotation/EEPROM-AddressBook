#include "I2C_Header.h"

void setupLED(void) {
    LED_PIN = OUTPUT;
    LED = HIGH;
}

void delay(void) {
    uint16_t count = 0xCFFF;
    while (0 < count) {
        count--;
    }
}

void blinkLED(uint8_t times) {
    for (int i = 0; i < times; i++) {
        LED = LOW;
        delay();
        LED = HIGH;
        delay();
    }
}