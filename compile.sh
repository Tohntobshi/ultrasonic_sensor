#!/usr/bin/bash
avr-gcc -w -Os -DF_CPU=8000000UL -mmcu=atmega168p -c -o main.o main.c
avr-gcc -w -mmcu=atmega168p main.o -o main
avr-objcopy -O ihex -R .eeprom main main.hex