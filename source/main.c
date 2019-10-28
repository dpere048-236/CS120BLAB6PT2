/*	Author: David Perez  dpere048
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab6 #  Exercise #1
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif
volatile unsigned char TimerFlag = 0; //TimerISR() sets this to 1, C program clears it to 0

//variables for mapping AVR TSR
unsigned long _avr_timer_M = 1; //start count from here down to zero, default = 1ms
unsigned long _avr_timer_cntcurr = 0; //internal count of ticks (1ms)
unsigned char tmpA = 0X00L //INT variable for input
unsigned char tmpB = 0x00; //initializes variable
enum States{start, lightA, waitA, lightB, waitB, lightC, waitC}state;

void TimerOn() {
	//avr timer/cnter
	TCCR1B = 0x0B; //BIT3 = 0: CTC Mode (clears time on compare)
	//0x0B = 00001011, 8MHz clock or 8,000,000/64 = 125k ticks
	//TCNT1 register will count 125k ticks
	//AVR OUTPUT COMPARE REGISTER
	OCR1A = 125; //Timer interrupt will be generated when TCNT1=OCR1A
	//We want a 1ms tick. .001s*125k = 125
	//when TCNT1 register = 125, 1ms has passed, so we comparte 10 124
	//AVR TIMER INTERRUP MASK REGISTER
	TIMSK1	 = 0x02; //bit 1: OC1E1A--- enables compare match interrupt

	//initialize avr counter
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _Avr_timer_cntcurr ms
	
	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}
void TimerOff() {
	TCCR1B = 0x00; //b3b1b0 = 000, timer off
}
void TimerISR() {
	TimerFlag = 1;
}
//C programmer does not touch this ISR but rather TIMERISR
ISR(TIMER1_COMPA_vect) {
	//CPU calls when TCNT1 == OCR1 (every 1ms when TimerOn settings
	_avr_timer_cntcurr--; //count down to 0 rathen tahn up to TOP
	if (_avr_timer_cntcurr == 0){ //more effiecient compare
		TimerISR(); //calls ISR that user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}
// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
void lightTick(){
	switch(state)
	{
		case start:
			state = lightA;
			break;
		case lightA:
			if(tmpA){
				state = waitA; //if button is pressed during light A
			}
			else{ //if no button is pressed light switches
			state = lightB;
			}
			break;
		case waitA:
			if(tmpA){
				state = lightA;
			}
			else{
				state = waitA;
			}
			break;
		case lightB:
			if(tmpA){
				state = waitB; //if button pressed
			}
			else{
				state = lightC;
			}
			break;
		case waitB:
			if(tmpA){
				state = lightB;
			}
			else{
				state = waitB;
			}
			break;
		case light C:
			if(tmpA){
				state = waitC;
			}
			else{ //starts back at light 1
			state = lightA;
			}
			break;
	}
	switch(state)
	{
		case lightA:
			tmpB = 0x01;
			break;
		case waitA:
			tmpB = 0x01;
			break;
		case lightB:
			tmpB = 0x02;
			break;
		case waitB:
			tmpB = 0x02;
			break;
		case lightC:
			tmpB = 0x04;
			break;
		case waitC:
			tmpB = 0x04;
			break;
	}
}
int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF; //initialize DDRA as inputs
	DDRB = 0xFF; PORTB = 0x00; //init port B to 0s and outputs
	TimerSet(300); //leds timer light for 300ms
	TimerOn();
	state = start;
    /* Insert your solution below */
    while (1) {
	tmpA = ~PINA & 0x08;
	lightTick();
	while (!TimerFlag); //wait 1 seec
	TimerFlag = 0;
	PORTB = tmpB;
    }
    return 0;
}
