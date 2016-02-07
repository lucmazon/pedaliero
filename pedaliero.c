#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_rawhid.h"
#include "smoothing.h"

#define CPU_PRESCALE(n)    (CLKPR = 0x80, CLKPR = (n))

#define ADC_MUX_PIN_F0    0x00

#define OFFSET 2

volatile int8_t do_output = 0;
uint8_t buffer[64];
uint8_t b_pin_index[8] = {0, 1, 2, 3, 15, 16, 17, 4};
uint8_t c_pin_index[8] = {-1, -1, -1, -1, -1, -1, 9, 10};
uint8_t d_pin_index[8] = {5, 6, 7, 8, 12, 11, 13, 14};
uint8_t f_pin_index[8] = {23, 22, -1, -1, 21, 20, 19, 18};

int main(void) {
    uint8_t i;
    uint16_t count = 0;

    uint8_t readings[SMOOTHING_BUFFER_SIZE];      // the readings from the analog input
    uint8_t readIndex = 0;                        // the index of the current reading
    uint16_t total = 0;                           // the running total

    // init readings
    initSmoothing(&readings);

    // set for 16 MHz clock
    CPU_PRESCALE(0);

    // Initialize the USB, and then wait for the host to set configuration.
    // If the Teensy is powered without a PC connected to the USB port,
    // this will wait forever.
    usb_init();
    while (!usb_configured()) /* wait */ ;

    // Wait an extra second for the PC's operating system to load drivers
    // and do whatever it does to actually be ready for input
    _delay_ms(1000);

    // Configure timer 0 to generate a timer overflow interrupt every
    // 256*1024 clock cycles, or approx 61 Hz when using 16 MHz clock
    TCCR0A = 0x00;
    TCCR0B = 0x05;
    TIMSK0 = (1 << TOIE0);

    // Set all pins to inputs
    DDRB = 0x00;
    DDRC = 0x00;
    DDRD = 0x00;
    DDRF = 0x00;

    // with pull up resistor
    PORTB = 0xFF;
    PORTC = 0xFF;
    PORTD = 0xFF;
    // except for F0 in normal mode
    PORTF = 0xFE;

    while (1) {
        // if time to send output, transmit something interesting
        if (do_output) {
            do_output = 0;
            // send a packet, first 2 bytes 0xABCD
            buffer[0] = 0xAB;
            buffer[1] = 0xCD;

            for (i = 0; i < 8; i++) {
                // Pin B (all pins used, no need to further check)
                if (PINB & (1 << i))
                    buffer[b_pin_index[i] + OFFSET] = 0;
                else
                    buffer[b_pin_index[i] + OFFSET] = 1;

                // Pin C
                if (c_pin_index[i] != -1) {
                    if (PINC & (1 << i))
                        buffer[c_pin_index[i] + OFFSET] = 0;
                    else
                        buffer[c_pin_index[i] + OFFSET] = 1;
                }

                // Pin D
                if (d_pin_index[i] != -1) {
                    if (PIND & (1 << i))
                        buffer[d_pin_index[i] + OFFSET] = 0;
                    else
                        buffer[d_pin_index[i] + OFFSET] = 1;
                }

                // Pin F0
                if (f_pin_index[i] != -1) {
                    if (i == 0) {
                        buffer[f_pin_index[0] + OFFSET] = averageAnalogRead(ADC_MUX_PIN_F0, &readings, &readIndex, &total);
                    } else if (PINF & (1 << i)) {
                        buffer[f_pin_index[i] + OFFSET] = 0;
                    } else {
                        buffer[f_pin_index[i] + OFFSET] = 1;
                    }
                }
            }

            // most of the packet filled with zero
            for (i = 50; i < 62; i++) {
                buffer[i] = 0;
            }

            // put a count in the last 2 bytes
            buffer[62] = count >> 8;
            buffer[63] = count & 255;
            // send the packet
            usb_rawhid_send(buffer, 50);
            count++;
        }
    }
}

// This interrupt routine is run approx 61 times per second.
ISR(TIMER0_OVF_vect)
        {
                static uint8_t count=0;
                if (++count > -1){
                    count = 0;
                    do_output = 1;
                }
        }



