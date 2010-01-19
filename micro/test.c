#ifndef F_CPU
#define F_CPU 3686400
#endif

#include <avr/io.h>          // (1)
#include <util/delay.h>

int main (void) {            // (2)
	DDRB  = 0xff;             // (3)
	PORTB = 0x03;             // (4)
	uint8_t x = 0;
	while(1) {                // (5a)
	/* "leere" Schleife*/  // (5b)
		PORTB = 1 << x;
		x += 1;
		if(x > 3)
			x = 0;
		_delay_ms(1);
	}                         // (5c)

	/* wird nie erreicht */
	return 0;                 // (6)
}
