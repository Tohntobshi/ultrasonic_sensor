#include <avr/io.h>
#include <util/delay.h>
#include <compat/twi.h>
#include <avr/interrupt.h>

volatile uint8_t regToGet = 0;
volatile uint16_t elapsed = 0;
volatile uint8_t transferStarted = 0;
volatile uint8_t readyToTrigger = 1;

// spi interrupts
ISR(SPI_STC_vect)
{
    uint8_t inputData = SPDR;
    if (inputData == 1) {
        SPDR = (uint8_t)(elapsed >> 8);
        transferStarted = 1;
    } else if (inputData == 2) {
        SPDR = (uint8_t)(elapsed);
        transferStarted = 0;
    }
}

// echo pin change interrupt
ISR(INT0_vect) {
    if (PIND & (1 << 2)) {
         // reset timer
        TCNT1 = 0;
    }
    else {
        // write result
        if (!transferStarted) {
            elapsed = TCNT1;
        }
        // trigger overflow
        TCNT1 = 0xFF;
    }
}

// timer overflow interrupt
ISR(TIMER1_OVF_vect) {
    readyToTrigger = 1;
}

int main (void)
{
    DDRD |= (1 << 3); // sensor trig
    DDRD &= ~(1 << 2); // sensor echo

    TCCR1B |= (1 << CS11);  // timer with clock/8 prescaling 1000000 per second

    SPCR = (1 << SPE) | (1 << SPIE); // Enable SPI and corresponding interrupt
    DDRB = (1 << 4); // Set MISO output

    // enable interrupts on int0 pin
    EICRA = (1 << ISC00);
    EIMSK |= (1 << INT0);

    // enable interrupts on timer1 overflow
    TIMSK1 = (1 << TOIE1);

    sei();
    
    while(1) {
        if (readyToTrigger) {
            readyToTrigger = 0;
            PORTD |= (1 << 3); 
            _delay_ms(1);
            PORTD &= ~(1 << 3);
        }
    }
    return 0;
}