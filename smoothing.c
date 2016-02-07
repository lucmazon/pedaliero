#include "smoothing.h"

void initSmoothing(uint8_t *readings) {
    for (int thisReading = 0; thisReading < SMOOTHING_BUFFER_SIZE; thisReading++) {
        readings[thisReading] = 0;
    }
}

uint8_t averageAnalogRead(uint8_t pin, uint8_t *readings, uint8_t *readIndex, uint16_t *total) {
    // subtract the last reading:
    *total = *total - readings[*readIndex];
    // read from the sensor:
    readings[*readIndex] = analogRead(pin) / 4;

    // add the reading to the total:
    *total = *total + readings[*readIndex];
    // advance to the next position in the array:
    *readIndex = *readIndex + 1;

    // if we're at the end of the array...
    if (*readIndex >= SMOOTHING_BUFFER_SIZE) {
        // ...wrap around to the beginning:
        *readIndex = 0;
    }

    // calculate the average:
    return *total / SMOOTHING_BUFFER_SIZE;
}
