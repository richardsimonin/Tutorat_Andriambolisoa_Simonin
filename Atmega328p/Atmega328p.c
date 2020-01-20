#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>
// for the input/output registers

// For the serial port

#define CPU_FREQ        16000000L       // Assume a CPU frequency of 16Mhz

void init_serial(int speed)
{
/* Set baud rate */
UBRR0 = CPU_FREQ/(((unsigned long int)speed)<<4)-1;

/* Enable transmitter & receiver */
UCSR0B = (1<<TXEN0 | 1<<RXEN0);

/* Set 8 bits character and 1 stop bit */
UCSR0C = (1<<UCSZ01 | 1<<UCSZ00);

/* Set off UART baud doubler */
UCSR0A &= ~(1 << U2X0);
}

void send_serial(unsigned char c){
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
}

unsigned char get_serial(void) {
	loop_until_bit_is_set(UCSR0A, RXC0);
	return UDR0;
}

// For I/O handling (examples for PIN 8 as output and PIN 2 as input)

void output_init(void){
	DDRB |= 0x3f; // les pins 8,9,10,11,12,13 du port B = LED : PIN as output
}

/*void output_set(int led, int etat){
if(etat==0) PORTB &= led; else PORTB |= led;
}*/

/* void output_set(unsigned char value,uint8_t port){
	 if(value==0) PORTB &= (0xff^(port)); else PORTB |= port;
 }
 */

void input_init(void){
	DDRD &= 0x83;  // PIN 2-3-4-5-6 as input
	PORTD |= 0x7c; // Pull-up activated on PIN 2
}

unsigned char input_get(void){
	//return ((PIND&0x04)!=0)?1:0;
	return ((PIND&0x7c)!=0x7c)?1:0;
}

unsigned char port_serie_libre(void){

	return (UCSR0A & (1<<RXC0)) !=0?1:0;
}

void LED(unsigned char car){
	/*unsigned char value;
	uint8_t tab[6]={1,2,4,8,16,32};
	*/
  if(car >= 'A' && car <= 'F'){
    car -= 'A';
    car = 0x01 << car;
    PORTB |= car;
  }
	//on test si le caractère reçue est une majuscule ou une minuscule 
  	//et si le caractère est compris entre A et F et on adapte le port B en fonction

  if(car >= 'a' && car <= 'f'){
    car -= 'a';
    car = 0xff - (0x01 << car);
    PORTB &= car;
    //value=0;
    //output_set(value,tab[c])
  }
}


int main(void){
	init_serial(9600);
	input_init();
	output_init();
	unsigned char char_recu;
    PORTB &= ~0x3f;
    PORTC = 0xFF;
    unsigned char message_out;
    send_serial(message_out);

	while(1)
	{
		if(input_get()){  //si on reçoit les informations des boutons
			message_out=PIND & 0b01111100;
			send_serial(message_out >> 2|0b00100000);
			//on envoie 0 0 1 puis notre etat des boutons
		}
		
        if(port_serie_libre()) LED(get_serial());            
	 	_delay_ms(20);
	}
	return 0;
}
