/*
 * frequency_generator.cpp
 *
 * Created: 6/26/2023 8:14:20 PM
 * Author : ThangNguyen
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define DOWN_PIN     DDRC5  
#define UP_PIN       DDRC4
#define REST_PIN     DDRB0
#define RANGE_1_PIN  DDRB3
#define RANGE_2_PIN  DDRB4

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
#define refreshTime = 1000;
// TIMER0 with prescaler clkI/O/1024
#define TIMER0_PRESCALER      (1 << CS02) | (1 << CS00)

// global variable define
unsigned int displayNumber = 0;

void init_TC0(void);
void init_TC1(void);
void init_GPIO(void);
void readButton(void);
void display(void);
void pwmOut(void);

int main(void)
{
	init_GPIO();
	
	//call TMR0 initialization function
	init_TC0();
	    
	//enable interrupt
	sei();
		
    /* Replace with your application code */
    while (1) 
    {
		readButton();
    }
}

void init_TC0(void)
{
	TIMSK |= (1 << TOIE0);        // interrupt enable - here overflow
	TCCR0 |= TIMER0_PRESCALER;    // use defined prescaler value
}

void init_GPIO(void)
{
	//set RB5 as output
	DDRB |= 1 << DDRB5;	
}

void readButton(void)
{
	
}

void display(void)
{
	
}

void pwmOut(void)
{
	
}

ISR (TIMER1_COMPA_vect)
{
	display();
}

ISR (TIMER2_COMP_vect)
{
	pwmOut();
}

