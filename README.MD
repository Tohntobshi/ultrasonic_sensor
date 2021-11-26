# auxillary controller for rcontrol_2

it sends time measurements from ultrasonic sensor via spi

## Compiling
packages necessary for compiling
```
gcc-avr avr-libc
```

to compile run compile script

## Flashing

to flash using raspbery pi (my case)

install packages
```
sudo apt install uisp avrdude
```

enable spi interface

find avrdude.conf and uncomment and modify block


	programmer
	  id    = "linuxgpio";
	  desc  = "Use the Linux sysfs interface to bitbang GPIO lines";
	  type  = "linuxgpio";
	  reset = 4;
	  sck   = 11;
	  mosi  = 10;
	  miso  = 9;
	;

connect wires as in the picture

check connection
```
sudo avrdude -c linuxgpio -p atmega168p -v
```

flash
```
sudo avrdude -F -V -c linuxgpio -p atmega168p -U flash:w:main.hex
```