#include <avr/io.h>
#include <util/delay.h>
#include <compat/twi.h>
#include <avr/interrupt.h>

volatile uint8_t regToGet = 0;
volatile uint16_t elapsed = 0;
volatile uint8_t transferStarted = 0;
volatile uint8_t readyToTrigger = 1;

// i2c interrupts
ISR(TWI_vect)
{
    uint8_t status = TWSR & 0b11111000;
    if (status == TW_SR_DATA_ACK || status == TW_SR_DATA_NACK) {
        regToGet = TWDR; // not needed now
    }
    if (status == TW_ST_SLA_ACK) {
        TWDR = (uint8_t)(elapsed);
        transferStarted = 1;
    }
    if (status == TW_ST_DATA_ACK) {
        TWDR = (uint8_t)(elapsed >> 8);
        transferStarted = 0;
    }
    TWCR |= (1 << TWINT);
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
    
    TWAR = (0x13 << 1); // i2c address is set to 0x13
    TWDR = 0x00; // default i2c data

    TCCR1B |= (1 << CS11);  // timer with clock/8 prescaling 1000000 per second

    TWCR = (1 << TWINT) | (1 << TWEA) | (1 << TWEN) | (1 << TWIE); // enable i2c

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