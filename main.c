#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

volatile uint16_t result = 0;
volatile uint8_t msb = 0;
volatile uint8_t lsb = 0;
volatile uint8_t chksum = 0;
volatile uint8_t chksum2 = 0;
volatile uint8_t samplingStarted = 0;

// spi interrupts
ISR(SPI_STC_vect)
{
    uint8_t inputData = SPDR;
    if (inputData == 0) {
        // start transaction, prepare data
        msb = (uint8_t)(result >> 8);
        lsb = (uint8_t)(result);
        chksum = (msb >> 4) + (msb & 0b00001111) + (lsb >> 4) + (lsb & 0b00001111);
        chksum2 = (~msb) + (~lsb);
    } else if (inputData == 1) {
        SPDR = msb;
    } else if (inputData == 2) {
        SPDR = lsb;
    } else if(inputData == 3) {
        SPDR = chksum;
    } else if(inputData == 4) {
        SPDR = chksum2;
    }
}

// echo pin change interrupt
ISR(INT0_vect) {
    if (PIND & (1 << 2)) {
         // reset timer
        TCNT1 = 0;
        samplingStarted = 1;
    }
    else {
        result = ((uint32_t)TCNT1 * 2 + (uint32_t)result * 6) >> 3;
        samplingStarted = 0;
    }
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


    sei();
    
    while(1) {
        if (samplingStarted == 0) {
            _delay_us(10);
            PORTD |= (1 << 3); 
            _delay_us(10);
            PORTD &= ~(1 << 3);
        }
        _delay_us(100);
    }
    return 0;
}