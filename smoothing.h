#include <stdint.h>
#include "analog.h"

// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
#define SMOOTHING_BUFFER_SIZE 10

void initSmoothing(uint8_t *readings);
uint8_t averageAnalogRead(uint8_t pin, uint8_t *readings, uint8_t *readIndex, uint16_t *total);
