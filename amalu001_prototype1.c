/* Partner(s) Name & E­mail: Alyza Malunao amalu001@ucr.edu
				consultant: Shilpa Chirackel schir001@ucr.edu

 * Lab Section: 21

 * Assignment: Lab # 10 Custom Lab

 * Exercise Description: A home security system that allows the user to enter a password using 
  the Keypad then enable an alarm in different rooms of the house. If you enter an incorrect 
  password more than 3 times, a message alerting you of the incident is sent to your phone. LED
  matrix is sectioned off into 4 quadrants, where each quadrant corresponds to different rooms 
  1-4. A lit LED within a quadrant represents the security system being enabled for that 
  corresponding room. A message is sent to an iPhone via bluetooth whenever the security alarm 
  is disabled/enables in a particular room. 

Keypad, LCD Display, USART, Bluetooth
enter password, password validation, menu, choose lock/unlock, choose room 1-4, 
send info to second microcontroller using USART, send alert message using bluetooth 
 *

 * I acknowledge all content contained herein, excluding template or example

 * code, is my own original work.

 */

#include <avr/io.h>
#include "keypad.h"
#include "lcd.h"
#include "scheduler.h"
#include "usart_ATmega1284.h"
#include <string.h>

unsigned char go = 0;					//global variable
//password variables
unsigned char i = 0;					//Password count key presses
unsigned char isPasswordValid = -1;		//bool isPasswordValid,1 valid; isPasswordValid, 0 invalid
unsigned char setPassword[4] = {'1','2','3','4'}; //set password
unsigned char password[4];				//user input password
unsigned char input;					//init with GetKeypad()
int cursor = 10;						//cursor for lcd display
unsigned char alertDone = 0;			//flag to signal when done transmitting alert to bluetooth
unsigned char fails = 0;				//counter for # failed pw attempts
unsigned char alertCnt = 0;				//alertBluetooth counter used to send string with bluetooth
unsigned char alertString[30];			//alert string to send with bluetooth
//menu variables
unsigned char cnt = 0;					//Menu counter for lcd display
unsigned char isLock = -1;				//bool isLock,1 lock; isLock,0 unlock
unsigned char data = 0x00;				//char to send with USART to matrix
unsigned char isSent = 0;				//flag to signal when matrix receives data
unsigned char bluetoothDone = 0;	 	//flag to signal when done transmitting to bluetooth
unsigned char sendString[80];			//string to send to bluetooth
int b = 0;								//Menu-Bluetooth counter used to send string with bluetooth
int sizeArr = 0;						//Menu-size of sendString used in memset

enum state{wait, readPW, checkPW, validPW, invalidPW, alert, alertBluetooth};
int PasswordFct(int state)
{

	//transitions PasswordFct
	switch(state)
	{
		case wait:
			if(go == 0)
			{
				state = readPW;
				fails = 0;
				alertCnt = 0;	
			}
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
			if(i == 5) //for lcd display
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
			if(fails == 3)
			{
				state = alert;
				fails = 0;
			}
			
			break;
		
		case alert:
			if(i == 5)
			{
				i = 0;
				state = alertBluetooth;
			}
			break;			
		case alertBluetooth:
			if(alertDone)
			{
				alertDone = 0;
				state = readPW;
			}
			break;

		default:
			state = wait;
			break;

	}

	//actions PasswordFct
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
				LCD_WriteData('*');
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
				++fails;
			}
			break;

		case validPW:
			if(i==0)
				LCD_DisplayString(1,"Valid PW!");
			++i;
			break;

		case invalidPW:
			if(i==0)
				LCD_DisplayString(1,"Invalid PW!");
			++i;

			break;

		case alert:
			LCD_DisplayString(1,"    !ALERT!     Invalid 3 times!");
			strcpy(alertString, "ALERT: 3 invalid pw attempts\n");
			++i;
			break;

		case alertBluetooth:
			if(alertCnt > 28){
				memset(&alertString[0], 0,28);
				alertDone = 1;
				alertCnt = 0;
			}
			else{
				if(USART_IsSendReady(1)){
					USART_Send(alertString[alertCnt], 1);
					alertCnt++;
				}
			}
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

	//Transitions MenuFct
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
			state = waitMenu;
			break;

		default:
			state = waitMenu;
			break;
	}

	//actions MenuFct
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
				LCD_DisplayString(1,"Room   is now     locked!");
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
				sizeArr = 17; //subtract one to get rid of ? error
			}
			else
			{
				LCD_DisplayString(1,"Room   is now     unlocked!");
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
						
			if(b > sizeArr){
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
	initUSART(0);		//microcontroller 2 communication
	USART_Flush(0);
	initUSART(1);		//Bluetooth LE communication
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
	
    while(1) {}
}