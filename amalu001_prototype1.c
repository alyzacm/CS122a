/*
 * amalu001_lcd_keypad.c
 * prototype 1
 * Created: 11/18/2015 10:31:58 AM
 *  Author: Alyza Malunao
 */ 


#include <avr/io.h>
#include "keypad.h"
#include "lcd.h"
#include "scheduler.h"
#include "usart_ATmega1284.h"
#include <stdio.h>
 #include <string.h>

unsigned char go = 0;					//global variable
unsigned char i = 0;					//count button presses
unsigned char cnt = 0;					//counter for lcd display
unsigned char isPasswordValid = -1;		//bool isPasswordValid,1 valid; isPasswordValid, 0 invalid
unsigned char setPassword[4] = {'1','2','3','4'};
unsigned char password[4];				//user input password
unsigned char input;					//init with GetKeypad()
int cursor = 10;						//cursor for lcd display
unsigned char isLock = -1;				//bool isLock,1 lock; isLock,0 unlock
unsigned char data = 0x00;				//char to send with USART to uc2
unsigned char isSent = 0;				//flag to signal when matrix receives data
unsigned char bluetoothDone = 0;	 	//flag to signal when done transmitting to bluetooth
unsigned char sendString[80];
int b = 0;
int sizeArr = 0;

enum state{wait, readPW, checkPW, validPW, invalidPW};
int PasswordFct(int state)
{
	switch(state)
	{
		case wait:
			if(go == 0)
				state = readPW;
			break;
		case readPW:
			if(i == 0)
			{
				isPasswordValid = -1;
				cursor = 10;
				cnt = 0;
			}
			else if(i == 4) //user done entering
			{
				state = checkPW;
				i = 0;
			}
			break;
		case checkPW:

			if(isPasswordValid == 1)
			{
				state = validPW;
			}
			else if(isPasswordValid == 0)
			{
				state = invalidPW;
			}

			break;
		case validPW:
			if(i == 5)
			{
				i = 0;
				go = 1;
				state = wait;
			}
			break;
		case invalidPW: 
			if(i == 5) //for lcd display
			{
				i = 0;
				state = readPW;
			}
			break;
		
		default:
			state = wait;
			break;

	}
	switch(state)
	{
		case wait:
			break;
		case readPW:
		
			LCD_DisplayString(1,"Enter PW ");
			input = GetKeypadKey();
			if(input != '\0')
			{
				password[i] = input;
				++i;
				LCD_Cursor(cursor++);
				LCD_WriteData(input);
			}
			
			break;
		case checkPW:
			if(atoi(setPassword) == atoi(password)) //check password arrays
			{
				isPasswordValid = 1;
			}
			else
			{
				isPasswordValid = 0;
			}
			break;
		case validPW:
			LCD_DisplayString(1,"Valid PW!");
			++i;
			break;
		case invalidPW:
			LCD_DisplayString(1,"Invalid PW!");
			++i;
			break;
		default:
			state = wait;
			break;

	}
	return state;

}

enum menuState {waitMenu, printMenu, getRoom, roomStatus, send, bluetooth, quit};
int MenuFct(int state)
{
	switch(state)
	{
		case waitMenu:
			if(go == 1)
			{
				cnt = 0;
				state = printMenu;
			}
			break;
		
		case printMenu:
			if(input != '\0')
			{
				if(input == 'A')
				{
					isLock = 1;
					state = getRoom;
				}
				else if(input == 'B')
				{
					isLock = 0;
					state = getRoom;
				}
				else if(input == 'C')
				{
					state = quit;
				}
				else
				{
					state = printMenu;
				}
			}
			break;
		
		case getRoom:
			if(input == '1' || input == '2' || input == '3' || input == '4')
			{
				state = roomStatus;
			}
			break;
		case roomStatus:
		if(cnt == 10)
		{
			cnt = 0;
			state = send;
		}
			break;
		case send:
			if(isSent){
				state = bluetooth;
				isSent = 0;
			}
			else
			{
				state = send;
			}
			break;
		case bluetooth:
			if(bluetoothDone){
				state = waitMenu;
				bluetoothDone = 0;
			}
			break;
		case quit:
		//if(cnt == 15)
		//{
			cnt = 0;
			state = waitMenu;
			break;
		//}
		default:
			state = waitMenu;
			break;
	}
	switch(state)
	{
		case waitMenu:
			break;
		
		case printMenu:
			cnt = 0;
			LCD_DisplayString(1,"lock/unlock/Quit A  /  B   / C  ");
			input = GetKeypadKey();	
			break;
		
		case getRoom:
			if(isLock)
			{
				LCD_DisplayString(1,"lock room:");
				input = GetKeypadKey();
				if(input != '\0')
				{
					LCD_Cursor(11);
					LCD_WriteData(input);
				}
			}
			else if(!isLock)
			{
				LCD_DisplayString(1,"unlock room:");
				input = GetKeypadKey();
				if(input != '\0')
				{
					LCD_Cursor(13);
					LCD_WriteData(input);
					LCD_Cursor(32);
				}
			}
			
			break;
		case roomStatus:
			if(isLock)
			{		
				LCD_DisplayString(1,"room   is now     locked!");
				LCD_Cursor(6);
				LCD_WriteData(input);
				LCD_Cursor(32);
				if(input == '1'){
					strcpy(sendString, "Room 1 is locked!\n");
					data = 0x81;
				}
				else if(input == '2'){
					strcpy(sendString, "Room 2 is locked!\n");
					data = 0x82;
				}
				else if(input == '3'){
					strcpy(sendString, "Room 3 is locked!\n");
					data = 0x83;
				}
				else if(input == '4'){
					strcpy(sendString, "Room 4 is locked!\n");
					data = 0x84;
				}
				sizeArr = 17;
			}
			else
			{
				LCD_DisplayString(1,"room   is now     unlocked!");
				LCD_Cursor(6);
				LCD_WriteData(input);
				LCD_Cursor(32);
				if(input == '1'){
					strcpy(sendString, "Room 1 is unlocked!\n");
					data = 0x01;
				}
				else if(input == '2'){
					strcpy(sendString, "Room 2 is unlocked!\n");
					data = 0x02;
				}
				else if(input == '3'){
					strcpy(sendString, "Room 3 is unlocked!\n");
					data = 0x03;
				}
				else if(input == '4'){
					strcpy(sendString, "Room 4 is unlocked!\n");
					data = 0x04;
				}
				sizeArr = 19;
				
			}

			cnt++;

			break;
		case send:
			if(USART_IsSendReady(0)){
				USART_Send(data, 0);
				isSent = 1;
			}
			break;
		case bluetooth:
						
			if(b>sizeArr){
				memset(&sendString[0], 0, sizeArr);
				bluetoothDone = 1;
				b = 0;
			}
			else{
				if(USART_IsSendReady(1)){
					USART_Send(sendString[b], 1);
					b++;
				}
			}
			break;

		case quit:
			LCD_ClearScreen();
			go = 0;
			break;
		
		default:
			state = waitMenu;
			break;
	}
	return state;
}

int main(void)
{
	initUSART(0);
	USART_Flush(0);
	initUSART(1);
	USART_Flush(1);

	DDRA = 0xFF; PORTA = 0x00; //lcd data_bus
	DDRB = 0xFF; PORTB = 0x00; //lcd control_bus
	DDRC = 0xF0; PORTC = 0x0F; //keypad
	
	tasksNum = 2;
	task function[tasksNum];
	tasks = function;
	
	unsigned char a=0; //task counter
	tasks[a].state = wait;
	tasks[a].period = 150;
	tasks[a].elapsedTime = tasks[a].period;
	tasks[a].TickFct = &PasswordFct;
	a++;
	
	tasks[a].state = waitMenu;
	tasks[a].period = 150;
	tasks[a].elapsedTime = tasks[a].period;
	tasks[a].TickFct = &MenuFct;
	
	TimerSet(150);
	TimerOn();
	
	LCD_init();
	
    while(1)
    {}
}