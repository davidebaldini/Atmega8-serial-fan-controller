all:
	avr-g++ -O2 -mmcu=atmega8 serial_fan.cpp
	avr-objcopy -j .text -j .data -O ihex a.out serial_fan.hex
	# program flash: avrdude -c usbtiny -p atmega8 -U flash:w:serial_fan.hex
	# set 2 MHz:     avrdude -c usbtiny -p atmega8 -U lfuse:w:0b11100010:m
