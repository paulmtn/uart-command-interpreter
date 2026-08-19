# Config
MCU = atmega328p
F_CPU = 16000000UL
PORT = /dev/ttyUSB0
BAUD = 115200
PROGRAMMER = arduino

# Toolchain
CC = avr-gcc
OBJCOPY = avr-objcopy
AVRDUDE = avrdude
SIZE = avr-size

# Paths to header files
AVR_INCLUDE = /usr/lib/avr/include

# Flags
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -Os -Wall -Wextra -std=c99
CFLAGS += -I$(AVR_INCLUDE)

# Files
TARGET = main
SRCS = main.c uart.c
ELF = $(TARGET).elf
HEX = $(TARGET).hex

all: $(HEX) size

$(ELF): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS)

$(HEX): $(ELF)
	$(OBJCOPY) -O ihex $< $@

size: $(ELF)
	$(SIZE) --format=avr --mcu=$(MCU) $(ELF)

flash: $(HEX)
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -P $(PORT) -b $(BAUD) -U flash:w:$(HEX)

clean:
	rm -f $(ELF) $(HEX)

.PHONY: all size flash clean
