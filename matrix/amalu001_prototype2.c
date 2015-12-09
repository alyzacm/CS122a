#include <avr/io.h>
#include "scheduler.h"    
#include "usart_ATmega1284.h"

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

//pattern displays partition of matrix into 4 rooms as shown below
//room layout
	//1 | 2
	//  |
	//	|
	//|||||
	//|||||
	//  |
	//  |
	//3 | 4
char col_arr [] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
char pat_arr [] = {~0x04, ~0x04, ~0x04, ~0x1F, ~0x1F, ~0x04, ~0x04, ~0x04};
unsigned char column; 			//column select
unsigned char pattern;			//pattern to display on row
unsigned char cnt = 0;			//counter for lcd display
unsigned char roomData = 0x84;	//data received via USART from uc1
unsigned char roomNum;			//display led in room # quadrant
unsigned char roomLock;			//if(1){led off} else{led off}
unsigned char isReceived = 0;	//bool isReceived 1, data; isReceived 0, no data
unsigned char goBack = 0;		//flag to go back to receive
enum roomState {init, receive, input, display};
int RoomFct(int state) {


	// Transitions
	switch (state) {
		case init:
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

		case display:
			if(USART_HasReceived(0)){
				roomData = USART_Receive(0);
				USART_Flush(0);
				roomNum = roomData & 0x0F;
				roomLock = (roomData & 0x80) >> 7;
				if(roomNum == 1)
				{
					if(roomLock)
					{
						pat_arr[7] = pat_arr[7]  & ~0x05;
					}
					else
					{
						pat_arr[7] = pat_arr[7] | ~0xEF; 
					}
				}
				else if(roomNum == 2)
				{
					if(roomLock)
					{
						pat_arr[7] = pat_arr[7] & ~0x14;
					}
					else
					{
						pat_arr[7] |= ~0x14;
					}
				}
				else if(roomNum == 3)
				{
					if(roomLock)
					{
						pat_arr[0] = pat_arr[0] & ~0x05;
					}
					else
					{
						pat_arr[0] |= ~0x04;
					}
				}
				else if(roomNum == 4)
				{
					if(roomLock)
					{
						pat_arr[0] = pat_arr[0] & ~0x14;
					}
					else
					{
						pat_arr[0] |= ~0x14;
					}
				}
			}
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

	initUSART(0);

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
	tasks[i].TickFct = &RoomFct;

	TimerSet(1); // value set should be GCD of all tasks
	TimerOn();

	while(1) {} // task scheduler will be called by the hardware interrupt

}