#include <avr/io.h>
#include "scheduler.h"  

// Ensure DDRB is setup as output
void transmit_data(unsigned char data) {
	unsigned char i;
	for (i = 0; i < 8 ; ++i) {
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTB = 0x08;
		// set SER = next bit of data to be sent.
		PORTB |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTB |= 0x04;
	}
	// set RCLK = 1. Rising edge copies data from the ???Shift??? register to the ???Storage??? register
	PORTB |= 0x02;
	// clears all lines in preparation of a new transmission
	PORTB = 0x00;
}

enum state{init, display};
int TickFct(int state){
	switch(state){
		case init:
			break;
		case display:
			state = display;
			break;
		default:
			state = init;
			break;
	}
	switch(state){
		case init:
			break;
		case display:
			PORTB = 0x80;
			PORTA = ~0x04;
			break;
		default:
			break;
	}
}

int main(void) {
	DDRB = 0xFF; PORTB = 0x00; //column select LED matrix 5x8
	DDRA = 0xFF; PORTA = 0x00; //pattern display LED matrix 5x8

	tasksNum = 1; // declare number of tasks
	task tsks[tasksNum]; // initialize the task array
	tasks = tsks; // set the task array

	// define tasks
	unsigned char i=0; // task counter
	tasks[i].state = -1;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &TickFct;
		PORTB = 0x80;
		PORTA = ~0x04;
		while(1){}
}