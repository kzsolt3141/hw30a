MCU=atmega8
F_CPU=8000000
CC=avr-gcc
OBJCOPY=avr-objcopy
MEMCHECK=avr-size
IFLAGS=-I../uart/ -I../adc/ -I../timer/ -I../

CFLAGS=-std=c11 -Wall -g -O3 -mmcu=${MCU} -DF_CPU=${F_CPU} ${IFLAGS}
TARGET=main
SRCS=main.c ../uart/uart.c ../adc/adc.c

all:
	${CC} ${CFLAGS} -o ${TARGET}.bin ${SRCS}
	${OBJCOPY} -j .text -j .data -O ihex ${TARGET}.bin ${TARGET}.hex
	${MEMCHECK} -C --mcu=${MCU} ${TARGET}.bin

clean:
	rm -f *.bin *.hex