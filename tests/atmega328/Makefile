# Project name
PROJECT=example

# Object files (one for each source file)
OBJECTS=main.o uart.o

# AVR fuses: external crystal (see https://www.engbedded.com/fusecalc/)
FUSES=e2 d9 ff

# Microcontroller
MCU_GCC=atmega328p
MCU_AVRDUDE=atmega328p

# Microcontroller speed, in Hz. Used by avr-libc for delays.
F_CPU=8000000UL

# Serial speed, in baud. Used by avr-libc for UART speed calculation.
BAUD=38400

#
# Rules
#

CC=avr-gcc
WARNINGS=-Wall -Wextra \
	 -Wformat=2 -Wno-unused-parameter -Wshadow \
	 -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
	 -Wredundant-decls -Wnested-externs -Wmissing-include-dirs -Wjump-misses-init -Wlogical-op
CPPFLAGS=-std=c11 ${WARNINGS} -Os -DF_CPU=${F_CPU} -DBAUD=${BAUD} -mmcu=${MCU_GCC} -ffunction-sections -fdata-sections -mcall-prologues

all: ${PROJECT}.hex

${PROJECT}.hex: ${PROJECT}.elf
	avr-objcopy -j .text -j .data -O ihex $< $@
	arm-linux-gnueabihf-size ${PROJECT}.hex

${PROJECT}.elf: ${OBJECTS}
	$(CC) -mmcu=${MCU_GCC} -o $@ $^ -Wl,--gc-sections
	avr-size -C --mcu=${MCU_GCC} ${PROJECT}.elf	
test-connection:
	remoteprog test -c avr -p ${MCU_AVRDUDE}

upload: ${PROJECT}.hex
	remoteprog upload -c avr -p ${MCU_AVRDUDE} $<

fuse:
	remoteprog fuse -p ${MCU_AVRDUDE} ${FUSES}

size: ${PROJECT}.elf
	avr-size -C --mcu=${MCU_GCC} ${PROJECT}.elf	

talk:
	@echo "Press Ctrl+A and then Shift+K to exit."
	@sleep 1
	screen /dev/ttyAMA0 38400

clean:
	rm -f ${OBJECTS} ${PROJECT}.elf ${PROJECT}.hex

# vim: set ts=8 sts=8 sw=8 noexpandtab:
