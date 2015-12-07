#include <avr/io.h>
#include "scheduler.h"
#include "bit.h"

// Ensure DDRC is setup as output
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

char col_arr [] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
char pat_arr [] = {~0x04, ~0x04, ~0x04, ~0x1F, ~0x1F, ~0x04, ~0x04, ~0x04};
unsigned char column;
unsigned char pattern;
unsigned char cnt = 0;
unsigned char roomData = 0x84;
unsigned char roomNum;
unsigned char roomLock;

//room layout
	//1 | 2
	//  |
	//	|
	//|||||
	//|||||
	//  |
	//  |
	//3 | 4
enum roomState {init, receive, display};
int RoomFct(int state) {


	// Transitions
	switch (state) {
		case init:
			state = receive;
			break;
		
		case receive:
			state = display;
			break;
			
		case display:
			state = display;
			break;
		
		default:
			state = init;
			break;
	}
	
	// Actions
	switch (state) {
		case init:
			cnt = 0;
			break;
		
		case receive:
			roomNum = roomData & 0x0F;
			roomLock = (roomData & 0x80) >> 7;
			if(roomNum == 1)
			{
				if(roomLock)
				{
					pat_arr[7] = ~0x05;
				}
				else
				{
					pat_arr[7] = ~0x04;
				}
			}
			else if(roomNum == 2)
			{
				if(roomLock)
				{
					pat_arr[7] = ~0x14;
				}
				else
				{
					pat_arr[7] = ~0x04;
				}
			}
			else if(roomNum == 3)
			{
				if(roomLock)
				{
					pat_arr[0] = ~0x14;
				}
				else
				{
					pat_arr[0] = ~0x04;
				}
			}
			else if(roomNum == 4)
			{
				if(roomLock)
				{
					pat_arr[0] = ~0x14;
				}
				else
				{
					pat_arr[0] = ~0x04;
				}
			}
			break;
			
		case display:
			if(cnt < 8){
				column = col_arr[cnt];
				pattern = pat_arr[cnt];
				++cnt;
			}
			else{
				cnt = 0;
			}

			break;
		
		default:
		break;
	}
	
	PORTB = column;	
	PORTA = pattern;		
	
	return state;
}


int main(void) {

	DDRB = 0xFF; PORTB = 0x00;
	DDRA = 0xFF; PORTA = 0x00;

	tasksNum = 1; // declare number of tasks
	task tsks[tasksNum]; // initialize the task array
	tasks = tsks; // set the task array

	// define tasks
	unsigned char i=0; // task counter
	tasks[i].state = -1;
	tasks[i].period = 1;
	tasks[i].elapsedTime = tasks[i].period;
	tasks[i].TickFct = &RoomFct;

	TimerSet(1); // value set should be GCD of all tasks
	TimerOn();

	while(1) {} // task scheduler will be called by the hardware interrupt

}