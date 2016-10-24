#include<avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "uart.h"

#define USART_BAUD 	115200UL
#define BAUDRATE	(F_CPU/(8UL*USART_BAUD))-1

void uart_init()
{
	/* Set baud rate */
	UBRR0H = (uint8_t) ((BAUDRATE) >> 8);
	UBRR0L = (uint8_t) BAUDRATE;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0);// | (1 << RXCIE0);
	UCSR0C = (3 << UCSZ00);
	UCSR0A = 1 << U2X0;
}


void putch(uint8_t c)
{
	while (!(UCSR0A & (1 << UDRE0))) ;
	UDR0 = c;
}

uint8_t getch(void)
{
				while (!(UCSR0A & (1 << RXC0)));
				return UDR0;
}

