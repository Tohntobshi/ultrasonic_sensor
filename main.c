#include <avr/io.h>
#include <util/delay.h>
#include <compat/twi.h>
#include <avr/interrupt.h>

volatile uint8_t regToGet = 0;
volatile uint16_t elapsed = 0;
volatile uint8_t transferStarted = 0;

ISR(TWI_vect)
{
    cli();
    uint8_t status = TWSR & 0b11111000;
    if (status == TW_SR_DATA_ACK || status == TW_SR_DATA_NACK) {
        regToGet = TWDR;
    }
    if (status == TW_ST_SLA_ACK || status == TW_ST_DATA_ACK) {
        switch (regToGet)
        {
        case 1:
            TWDR = (uint8_t)elapsed;
            transferStarted = 0;
            break;
        default:
            TWDR = (uint8_t)(elapsed >> 8);
            transferStarted = 1;
            break;
        }
    }
    // if (status != TW_SR_SLA_ACK && status != TW_ST_SLA_ACK && status != TW_ST_ARB_LOST_SLA_ACK && status != TW_ST_DATA_ACK) {
    //     TWCR |= (1<<TWEA);
    // }
    TWCR |= (1<<TWINT);
    sei();
}

int main (void)
{
    DDRB |= (1 << 3); // sensor trig
    DDRB &= ~(1 << 4); // sensor echo
    
    TWAR = (0x13 << 1); // i2c address is set to 0x13
    TWDR = 0x00; // default i2c data

    TCCR1B |= (1 << CS11);  // timer with clock/8 prescaling 1000000 per second

    TWCR = (1<<TWINT) | (1<<TWEA) | (1<<TWEN) | (1<<TWIE); // enable i2c

    sei();
    
    while(1) {
        // trigger sensor
        PORTB |= (1 << 3); 
        _delay_ms(10);
        PORTB &= ~(1 << 3);
        // wait for 1 on echo pin
        while (!(PINB & (1 << 4))) {}

        // todo start timer
        TCNT1 = 0;

        // wait for 0 on echo pin
        while (PINB & (1 << 4)) {}

        // end timer and write result
        cli();
        if (!transferStarted)
        {
            elapsed = TCNT1;
        }
        sei();
    }
    return 0;
}