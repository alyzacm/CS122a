DEVICE = atmega1284
PROGRAMMER = avrisp2
PORT = usb
FILENAME = amalu001_prototype1
COMPILEO = avr-gcc -Os -mmcu=$(DEVICE)
COMPILE = avr-gcc -mmcu=$(DEVICE)

all: build upload clean

build:
	$(COMPILEO) -c $(FILENAME).c
	$(COMPILE) -o $(FILENAME).elf $(FILENAME).o
	avr-objcopy -j .text -j .data -O ihex $(FILENAME).elf $(FILENAME).hex

upload:
	$avrdude -P $(PORT) -p $(DEVICE) -c $(PROGRAMMER) -e -U flash:w:$(FILENAME).hex

clean:
	rm $(FILENAME).o
	rm $(FILENAME).elf
	rm $(FILENAME).hex