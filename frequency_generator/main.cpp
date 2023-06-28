/*
 * frequency_generator.cpp
 *
 * Created: 6/26/2023 8:14:20 PM
 * Author : ThangNguyen
 */ 

#include <avr/io.h>
#include <util/delay.h> 
#include <avr/interrupt.h>

#define isDownPress  ((PINC & (1<<PINC5)) == 0)
#define isUpPress    ((PINC & (1<<PINC4)) == 0)
#define isRestPress  ((PINB & (1<<PINB0)) == 0)
#define RANGE_1      ((PINB & (1<<PINB3)) == 0)
#define RANGE_2      ((PINB & (1<<PINB4)) == 0)

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
unsigned int maxRange = 1000;
unsigned int minRange = 100;
unsigned int freq = 100;
unsigned int factor = 1;
uint8_t digitVal_1, digitVal_2, digitVal_3, digitVal_4 = 0;
uint8_t pointIdx = 0;
uint8_t digitIdx = 1; // range in 1-4
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
uint8_t led7[10] = { 0x84, 0x9F, 0xC8, 0x89, 0x93, 0xA1, 0xA0, 0x8F, 0x80, 0x81 };

void init_TC0(void);
void init_TC1(void);
void init_GPIO(void);
void readButton(void);
void display(void);

void setDisplay(unsigned int number, unsigned int f);
void increaseFreq(void);
void decreaseFreq(void);
void resetFreq(void);
void readRange(void);
void update(void);
void setPulse(unsigned int _freq, unsigned int _factor);

int main(void)
{
	init_GPIO();
	init_TC0(); //init timer1 for display
	init_TC1(); //init timer for pulse output
	    
	//enable interrupt
	sei();
	
	readRange();
	update();
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

void init_TC1(void)
{
	// set freq
	OCR1A = 0x00FF;
	//0x01FF: 500Hz
		
	TCCR1A |= (1 << COM1A0);    // set none-inverting mode
	TCCR1A |= (1 << WGM11) | (1 << WGM10);    //Mode 11: PWM, Phase Correct === TOP: OCR1A === PWM_frequency = clock_speed / (2 * Prescaller_value * TOP_value )
	TCCR1B |= (1 << WGM13);
	
	TCCR1B |= (1 << CS11); // set prescaler to 8 and starts PWM
}

void init_GPIO(void)
{
	//set RB1 as output
	DDRB |= (1 << DDRB1);	
	//set RC0, RC1, RC2, RC3 as output
	DDRC |= 0x0F;
	// set RDx as output
	DDRD = 0xFF;
	
	//set PC4, PC5 as input
	DDRC &= ~((1 << DDRC5) | (1 << DDRC4));
	PORTC |= (1 << PORTC5) | (1 << PORTC4);
}

void readButton(void)
{
	readRange();
	if (isDownPress) {
		_delay_ms(50);
		int cnt = 0;
		bool isLongPress = false;
		while (isDownPress && (cnt < 100)) {
			cnt++;
			if (cnt == 100){
				isLongPress = true;
			}
			_delay_ms(50);
		}
		
		if (isLongPress){
			cnt = 0;
			while(isDownPress) {
				cnt++;
				if (cnt % 10 == 0) {
					decreaseFreq();
				}
				_delay_ms(50);
			}
		} else if(!isDownPress) {
			decreaseFreq();
		}
	}
	
	if (isUpPress) {
		_delay_ms(50);
		int cnt = 0;
		bool isLongPress = false;
		while (isUpPress && (cnt < 100)) {
			cnt++;
			if (cnt == 100){
				isLongPress = true;
			}
			_delay_ms(50);
		}
			
		if (isLongPress){
			cnt = 0;
			while(isUpPress) {
				cnt++;
				if (cnt % 10 == 0) {
					increaseFreq();
				}
				_delay_ms(50);
			}
			} else if(!isUpPress) {
			increaseFreq();
		}
	}
	
	if (isRestPress) {
		_delay_ms(50);
		while (isRestPress){
			_delay_ms(50);
		}
		resetFreq();
	}
}

void display(void)
{
	PORTC |= 0x0F;
	switch (digitIdx) {
		case 1: {
			PORTD = led7[digitVal_1];
			if (pointIdx == 1){
				PORTD &= ~(1<<PORTD7);
			}
			PORTC &= ~(1<<PORTC3);
			break;
		}
		case 2: {
			PORTD = led7[digitVal_2];
			if (pointIdx == 2){
				PORTD &= ~(1<<PORTD7);
			}
			PORTC &= ~(1<<PORTC2);
			break;
		}
		case 3: {
			PORTD = led7[digitVal_3];
			if (pointIdx == 3){
				PORTD &= ~(1<<PORTD7);
			}
			PORTC &= ~(1<<PORTC1);
			break;
		}
		case 4: {
			if (digitVal_4 == 10) break;
			PORTD = led7[digitVal_4];
			if (pointIdx == 4){
				PORTD &= ~(1<<PORTD7);
			}
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

void setDisplay(unsigned int number, unsigned int f)
{
	digitVal_1 = number % 10;
	digitVal_2 = (number /10) % 10;
	digitVal_3 = (number /100) % 10;
	if ((number < 1000) && (f !=1000)){
		digitVal_4 = 10; // mean do not display
	} else{
		digitVal_4 = (number /1000) % 10;
	}
	
	if (f == 1){
		pointIdx = 0;
	} else if (f == 10) {
		pointIdx = 2;
	} else if (f == 100) {
		pointIdx = 3;
	} else if (f == 1000) {
		pointIdx = 4;
	}
}

void increaseFreq(void)
{
	if (freq < 1000) {
		freq += 10;
		update();
	}
}

void decreaseFreq(void)
{
	if (freq > 100) {
		freq -= 10;
		update();
	}
}

void resetFreq(void)
{
	freq = minRange;
	update();
}

void readRange(void)
{
	unsigned int newFactor;
	if (RANGE_1 && RANGE_2){
		newFactor = 1000;
	} else if (RANGE_1)
	{
		newFactor = 100;
	} else if (RANGE_2)
	{
		newFactor = 10;
	} else {
		newFactor = 1;
	}
	
	if (newFactor != factor) {
		unsigned int newFreq;
		if (newFactor > factor) {
			newFreq = freq * (newFactor / factor);
		} else {
			newFreq = freq * newFactor / factor;
		}
		
		if ((newFreq < minRange) || (maxRange < newFreq)) {
			newFreq = minRange;
		}
		freq = newFreq;
		factor = newFactor;
		update();
	}
}

void update(void)
{
	setDisplay(freq, factor);
	setPulse(freq, factor);
}

void setPulse(unsigned int _freq, unsigned int _factor)
{
	TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));
	// f = (8*10^6) / (4 * scale * topValue)
	if (_factor == 1) {
		unsigned int k = (_freq * 10) / 100;
		unsigned int top = (20000 / k) * 10;
		OCR1A = top;
		TCCR1B |= (1 << CS10); //No Prescaling
	} else if (_factor == 10) {
		unsigned int k = (_freq * 10) / 100;
		unsigned int top = (25000 / k) * 10;
		OCR1A = top;
		TCCR1B |= (1 << CS11); //Clock / 8
		
	} else if (_factor == 100) {
		unsigned int k = (_freq * 10) / 100;
		unsigned int top = (31250 / k) * 10;
		OCR1A = top;
		TCCR1B |= (1 << CS11) | ((1 << CS10)); //Clock / 64		
	} else if (_factor == 1000) {
		unsigned int k = (_freq * 10) / 100;
		unsigned int top = (19531 / k) * 10;
		OCR1A = top;
		TCCR1B |= (1 << CS12) | ((1 << CS10)); //Clock / 1024
	}
}

ISR (TIMER0_OVF_vect)
{
	display();
}

