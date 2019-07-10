/*
 * test.c
 *
 * Created: 2019/7/6 18:06:56
 * Author : Coco
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
	TCCR1B = 0x0B;
	OCR1A = 125;
	TIMSK1 = 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff(){
	TCCR1B = 0x00;
}

void TimerISR(){
	TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if(_avr_timer_cntcurr ==0){
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

void TimerSet(unsigned long M){
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
void set_PWM(double frequency){
	static double current_frequency;
	if(frequency != current_frequency){
		if(!frequency){TCCR3B &= 0x08;}
		else{TCCR3B |=0x03;}
		if(frequency < 0.954){OCR3A =0xFFFF;}
		else if(frequency > 31250){OCR3A =0x0000;}
		else{OCR3A =(short)(8000000/(128*frequency))-1;}
		
		TCNT3 =0;
		current_frequency=frequency;
	}
}

void PWM_on(){
	TCCR3A = (1<<COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31)|(1<<CS30);
	set_PWM(0);
}

void PWM_off(){
	TCCR3A =0x00;
	TCCR3B = 0x00;
}
double C = 2610.63;
double D = 2930.66;
double E = 3290.63;
double F = 3490.23;
double G = 3920.00;
double A = 4400.00;
double B = 4930.88;
double C5 = 5230.25;
double nodes[8] = { 2610.63,2930.66,3290.63,3490.23,3920.00,4400.00,4930.88,5230.25};
unsigned char i;
enum states {start, wait, pressA0,pressing,waitforreleasing} state;
unsigned int count;
unsigned char on;
const unsigned int timer = 30;
void tick(){
	unsigned char A = ~PINA;
	unsigned char A0 = ~PINA &0x01;
	unsigned char A1 = ~PINA & 0x02;
	unsigned char A2 = ~PINA & 0x04;
	switch(state){
		case start:
		state = wait;
		break;
		case wait:
		if(A0)
		state = pressA0;
		else state = wait;
		break;
		case pressA0:
		state = pressing;
		break;
		case pressing:
		if(i > 0 || count > 0) state = pressing;
		else if(A0) state = waitforreleasing;
		else state = wait;
		break;
		case waitforreleasing:
		if(A) state = waitforreleasing;
		else state = wait;
		break;
		default:
		state =start;
		break;
	}
	switch(state){
		case start:
		break;
		case wait:
		break;
		case pressA0:
		PWM_on();
		i = 4;
		set_PWM(nodes[i]);
		count = 30;
		break;
		case pressing:
		if(count >0)
			count --;
		else{
		set_PWM(nodes[--i]);
		count = 30;
		}
		if(i == 0 && count == 0)
			PWM_off();
		break;
		case waitforreleasing:
		break;
		default:
		break;
	}
}
int main(void) {
	/* Insert DDR and PORT initializations */
	DDRA = 0x00;PORTA = 0xFF;
	DDRB = 0xFF;PORTB = 0x00;
	/* Insert your solution below */
	TimerSet(1);
	TimerOn();
	count = 0;
	state = start;
	while (1) {
		tick();
		while(!TimerFlag){}
		TimerFlag = 0;
	}
	return 1;
}