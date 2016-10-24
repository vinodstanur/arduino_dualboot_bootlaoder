###################################################################
# Makefile for arduino_dualboot hack
# visit http://blog.vinu.co.in for more details
###################################################################

PROJECT=arduino_dualboot
SOURCES=main.c i2c.c uart.c
CC=avr-gcc
OBJCOPY=avr-objcopy
MMCU=atmega328p
LD_FLAGS = -Wl,--section-start=.text=0x7800
C_FLAGS = -mmcu=$(MMCU) -Os -Wall -std=c99
PROGRAMMER=usbasp
BURN=avrdude
MCU=m328p


$(PROJECT).hex: $(PROJECT).out
	$(OBJCOPY) -j .text -j .data -O ihex $(PROJECT).out $(PROJECT).hex
	

$(PROJECT).out: $(SOURCES)
	$(CC) $(C_FLAGS) $(LD_FLAGS) -I./ -o $(PROJECT).out $(SOURCES) 
	avr-size $(PROJECT).out
	
p: $(PROJECT).hex
	$(BURN) -c $(PROGRAMMER) -p $(MCU) -U flash:w:$(PROJECT).hex

f:
	$(BURN) -c $(PROGRAMMER) -p $(MCU) -U lfuse:w:0xef:m -U hfuse:w:0xda:m -U efuse:w:0xfd:m 

clean:
	rm -f *.hex *.out
