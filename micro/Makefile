# AVR-GCC Makefile
PROJECT=serial
SOURCES=serial.c
CC=avr-gcc
OBJCOPY=avr-objcopy
MMCU=atmega16

CFLAGS=-mmcu=$(MMCU) -Wall -T /usr/x86_64-pc-linux-gnu/avr/lib/ldscripts/avr5.x

$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -j .text -O ihex $(PROJECT).out $(PROJECT).hex

$(PROJECT).out: $(SOURCES)
	$(CC) $(CFLAGS) -I./ -o $(PROJECT).out $(SOURCES) -Os

program: $(PROJECT).hex
	avrdude -P usb -p m16 -c avrispmkII -e -U $(PROJECT).hex

clean:
	rm -f $(PROJECT).out
	rm -f $(PROJECT).hex

