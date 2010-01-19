#define FOSC 1843200// Clock Speed
#define BAUD 600
#define MYUBRR FOSC/16/BAUD-1

#define F_CPU 3686400

#include <avr/io.h>
#include <avr/delay.h>

void USART_Init(unsigned int ubrr);
void USART_Transmit( unsigned char data );

int main( void )
{
	DDRC = 0xff;
	USART_Init( MYUBRR );
	PORTC = 2;
	while(1) {
		USART_Transmit(0xff);
		_delay_ms(500);
	}
	
	return 0;
}
void USART_Init( unsigned int ubrr)
{
	/* Set baud rate */
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	/* Enable receiver and transmitter */
	UCSRB = (1<<RXEN)|(1<<TXEN);
	/* Set frame format: 8data, 2stop bit */
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
}

void USART_Transmit( unsigned char data )
{
	//PORTC = 1;
	/* wait for empty transmit buffer */
	while ( !( UCSRA & (1<<UDRE) ) );

	UDR = data;

	while ( !( UCSRA & (1<< TXC) ) );
	PORTC = ~PORTC;
}
