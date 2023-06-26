/*
 * frequency_generator.cpp
 *
 * Created: 6/26/2023 8:14:20 PM
 * Author : ThangNguyen
 */ 

#include <avr/io.h>
#include <util/delay.h> 
#include <avr/interrupt.h>

#define DOWN_PIN     ((PORTC & _BV(PC5)) >> PC5)//PORTC.5
#define UP_PIN       ((PORTC & _BV(PC4)) >> PC4)//PORTC.4
#define REST_PIN     ((PORTB & _BV(PB0)) >> PB0)//PORTB.0
#define RANGE_1_PIN  ((PORTB & _BV(PB3)) >> PB3)//PORTB.3
#define RANGE_2_PIN  ((PORTB & _BV(PB4)) >> PB4)//PORTB.4

#define PWM_POUT    DDRB1
#define LED7A       DDRD4
#define LED7B       DDRD5
#define LED7C       DDRD6
#define LED7D       DDRD1
#define LED7E       DDRD0
#define LED7F       DDRD3
#define LED7G       DDRD2
#define LED7DP      DDRD7
#define LED7_1      DDRC3
#define LED7_2      DDRC2
#define LED7_3      DDRC1
#define LED7_4      DDRC0

//magic number define
#define TIMER0_PRESCALER      (1 << CS01) | (1 << CS00) // clkI/O/64 (From prescaler)

// global variable define
unsigned int freq = 0;
unsigned int maxRange = 0;
unsigned int displayNumber, digitVal_1, digitVal_2, digitVal_3, digitVal_4 = 9;
unsigned int digitIdx = 1; // range in 1-4
/*
    7 6 5 4 3 2 1 0
    DP C B A F G D E
    0   1 0 0 0 0 1 0 0  -> 0x84
    1	1 0 0 1 1 1 1 1  -> 0x9F
    2   1 1 0 0 1 0 0 0  -> 0xC8
    3   1 0 0 0 1 0 0 1  -> 0x89
    4   1 0 0 1 0 0 1 1  -> 0x93
    5   1 0 1 0 0 0 0 1  -> 0xA1
    6   1 0 1 0 0 0 0 0  -> 0xA0
    7   1 0 0 0 1 1 1 1  -> 0x8F
    8   1 0 0 0 0 0 0 0  -> 0x80
    9   1 0 0 0 0 0 0 1  -> 0x81
*/
unsigned int led7[10] = { 0x84, 0x9F, 0xC8, 0x89, 0x93, 0xA1, 0xA0, 0x8F, 0x80, 0x81 };

void init_TC0(void);
void init_TC2(void);
void init_GPIO(void);
void readButton(void);
void display(void);
void pulseOutput(void);

void setDisplayNumber(unsigned int number);
void increaseFreq(void);
void decreaseFreq(void);
void resetFreq(void);
void updateRange(void);

int main(void)
{
	init_GPIO();
	init_TC0(); //init timer1 for display
	init_TC2(); //init timer for pulse output
	    
	//enable interrupt
	sei();
		
    /* Replace with your application code */
    while (1) 
    {
		readButton();
		_delay_ms(100);
    }
}

void init_TC0(void)
{
	TIMSK |= (1 << TOIE0);        // interrupt enable - here overflow
	TCCR0 |= TIMER0_PRESCALER;    // use defined prescaler value
}

void init_TC2(void)
{
}

void init_GPIO(void)
{
	//set RB1 as output
	DDRB |= 1 << DDRB1;	
	//set RC0, RC1, RC2, RC3 as output
	DDRC |= 0x0F;
	// set RDx as output
	DDRD = 0xFF;
}

void readButton(void)
{
	
}

void display(void)
{
	PORTC |= 0x0F;
	switch (digitIdx) {
		case 1: {
			PORTD = led7[digitVal_1];
			PORTC &= ~(1<<PORTC3);
			break;
		}
		case 2: {
			PORTD = led7[digitVal_2];
			PORTC &= ~(1<<PORTC2);
			break;
		}
		case 3: {
			PORTD = led7[digitVal_3];
			PORTC &= ~(1<<PORTC1);
			break;
		}
		case 4: {
			PORTD = led7[digitVal_4];
			PORTC &= ~(1<<PORTC0);		
			break;
		}
	}
	
	if (digitIdx == 4) {
		digitIdx = 1;
	} else {
		digitIdx++;
	}
}

void setDisplayNumber(unsigned int number)
{
	displayNumber = number;
	digitVal_1 = displayNumber % 10;
	digitVal_2 = (displayNumber /10) % 10;
	digitVal_3 = (displayNumber /100) % 10;
	digitVal_4 = (displayNumber /1000) % 10;
}

void pulseOutput(void)
{
	
}

void increaseFreq(void)
{
	freq += 10;
}

void decreaseFreq(void)
{
	freq -= 10;
}

void resetFreq(void)
{
	freq = 0; //min range
}

void updateRange(void)
{
	if (RANGE_1_PIN && RANGE_2_PIN){
		maxRange = 1000;
	} else if (RANGE_1_PIN)
	{
		maxRange = 100;
	} else if (RANGE_2_PIN)
	{
		maxRange = 10;
	} else {
		maxRange = 1;
	}
}

ISR (TIMER0_OVF_vect)
{
	display();
}

ISR (TIMER2_COMP_vect)
{
	pulseOutput();
}

