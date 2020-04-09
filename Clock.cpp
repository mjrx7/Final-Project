/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "TimerLPC.h"
#include "HD44780.h"
#include "KeyPad.h"
#include "LPC1769_RTC_DEFINITIONS.h"
#include "PWM.h"

#define START_SECONDS 0
#define START_MINUTES 0
#define START_HOUR 12

bool PM = false;
bool ALARM_ON = false;
bool ALARM_ACTIVE = false;
bool SNOOZE = false;

char KEYPUSHED = 0;
int SNOOZE_TIME = 1;

void printTime(unsigned int[]);
void setTime(void);
void setAlarm(void);
void alarm(void);
void clockSetup(void);
void changeTime(void);
void toggleAlarm(void);
void setSnooze(void);
void TIMER_IRQ_SETUP(void);

extern "C" void TIMER0_IRQHandler(void){
	if((T0IR >> 0) & 1){
		T0MR0 = T0MR0 + 500;
		KEYPUSHED = keyPress();
		T0IR = (1 << 0);
	}
}

int main(void) {
	wait(2);
	setupHD44780();
	setupKeyPad();
	TIMER_IRQ_SETUP();
	clockSetup();

    while(1) {
    	if(ALARM_INTERRUPT)
    		alarm();
    	if(KEYPUSHED == 'A')
    		setTime();
    	if(KEYPUSHED == 'B')
			setAlarm();
    	if(KEYPUSHED == 'C')
    		toggleAlarm();
    	if(KEYPUSHED == 'D')
    		setSnooze();
    	changeTime();
		wait(.3);
    }
    return 0 ;
}

void setSnooze(void){
	while(KEYPUSHED == 'D'){};
	char snoozeDig1temp = 0;

	commandLed(1);
	wordWrite("Change snooze time");
	commandLed(0xC0);
	wordWrite("Current: ");
	charWrite(SNOOZE_TIME + 48);
	if(SNOOZE_TIME == 1)
		wordWrite(" min");
	else
		wordWrite(" mins");

	commandLed(0x94);
	wordWrite("Input: X (1-9 max)");
	commandLed(0xD4);
	wordWrite("Press # to exit");

	commandLed(0xD);	// Set cursor blinking
	commandLed(0x9b);

	while(snoozeDig1temp == 0){
		snoozeDig1temp = KEYPUSHED;
	}
	if(snoozeDig1temp != '#'){
		if((snoozeDig1temp <= '0') || (snoozeDig1temp > '9'))
			SNOOZE_TIME = 1;
		else
			SNOOZE_TIME = snoozeDig1temp - 48;
	}
	commandLed(0x0c);
}

void TIMER_IRQ_SETUP(void){
	T0TCR |= 1;	// Start Timer
	T0MR0 = T0TC + 500; // Interrupt 1000 us into future
	T0MCR |= (1 << 0);	// enable interrupt on MR0 match
	ISER0 = (1 << 1);	// Enable interrupts for Timer 0
}

void toggleAlarm(void){
	while(KEYPUSHED == 'C'){};
	if(!ALARM_ON){
		AMR = 0;
		AMR |= (0x1f << 3);	// Mask Alarm Registers year, month, doy, dow, dom
		commandLed(1);
		commandLed(0xC0);
		charWrite(32);
		charWrite(32);
		charWrite(32);
		charWrite(32);
		wordWrite("Alarm ON");
		wait(1.5);
	}
	else{
		SNOOZE = false;
		AMR = 1;
		commandLed(1);
		commandLed(0xC0);
		charWrite(32);
		charWrite(32);
		charWrite(32);
		charWrite(32);
		wordWrite("Alarm OFF");
		wait(1.5);
	}
	ALARM_ON = !ALARM_ON;
}

void changeTime(void){
	unsigned int time = CTIME0;
	unsigned int timeInSeconds = (time) & 0x3F;
	unsigned int timeInMinutes = (time >> 8) & 0x3F;
	unsigned int timeInHours = (time >> 16) & 0x1F;
	if(timeInHours >= 12){
		if(timeInHours!=12)
			timeInHours = timeInHours - 12;
		PM = true;
	}
	else
		PM = false;

	commandLed(1);
	charWrite(32);
	charWrite(32);
	charWrite(32);
	charWrite(32);
	unsigned int digit[6] = {timeInHours/10,timeInHours - 10*digit[0],timeInMinutes/10,digit[3] = timeInMinutes - 10*digit[2],
			digit[4] = timeInSeconds/10,digit[5] = timeInSeconds - 10*digit[4]};

	printTime(digit);

	charWrite(58);
	charWrite(digit[4] + 48);
	charWrite(digit[5] + 48);
	charWrite(32);
	if(PM){
		wordWrite("PM");
	}
	else{
		wordWrite("AM");
	}
	if(KEYPUSHED != 0){
		charWrite(32);
		charWrite(KEYPUSHED);
	}
	if(!ALARM_ACTIVE){
		commandLed(0xC0);
		wordWrite("A: Set Time");
		commandLed(0x94);
		wordWrite("B: Set Alarm");
		commandLed(0xD4);
		wordWrite("C: On/Off (");
		if(ALARM_ON)
			wordWrite("ON)");
		else
			wordWrite("OFF)");
		if(SNOOZE)
			wordWrite("  Zzzz");
	}
	else{
		commandLed(0x94);
		wordWrite("Press D for OFF");
		commandLed(0xD4);
		wordWrite("Press OTHER 4 SNOOZE");
	}
}

void alarm(void){
	ALARM_ACTIVE = true;
	PWM _alarm(PWM1_1);
	_alarm.setFrequency(770);
	_alarm = 0.5;
	char off = 0;

	while(off == 0){
		commandLed(1);
		_alarm.setFrequency(960);
		wait(0.2);
		if((off = KEYPUSHED) != 0)
			break;
		changeTime();
		_alarm.setFrequency(770);
		wait(0.2);
		off = KEYPUSHED;
	}

	if(off != 'D'){
		if(ALMIN + SNOOZE_TIME >= 60){
			if(ALHOUR + 1 == 24)
				ALHOUR = 0;
			else
				ALHOUR++;
			ALMIN = ALMIN + SNOOZE_TIME - 60;
		}
		else
			ALMIN = ALMIN + SNOOZE_TIME;
		SNOOZE = true;
		off = KEYPUSHED;
	}
	else
		SNOOZE = false;
	while(off == KEYPUSHED){}
	_alarm = 0;
	ILR |= (1 << 1);	// Clear Alarm
	ALARM_ACTIVE = false;
}

void clockSetup(void){
	AMR = 1;
	CCR = 0b10010;	// Disable clock[0], reset CTC[1], cal counter disabled[4]
	CCR = 0b10000;	// Reset CTC[1] is removed
	ALSEC = 0;
	CCR = 0b10001;	// Enable clock[0], cal counter disabled[4]
}

void printTime(unsigned int digit[]){
	charWrite(digit[0] + 48);
	charWrite(digit[1] + 48);
	charWrite(58);
	charWrite(digit[2] + 48);
	charWrite(digit[3] + 48);
}

void setAlarm(void){
	while(KEYPUSHED=='B'){
		while(KEYPUSHED == 'B'){};
		char hrDig10temp = 0;
		char hrDig1temp = 0;
		char minDig10temp = 0;
		char minDig1temp = 0;
		char pauser;

		commandLed(1);
		wordWrite("Change alarm time");
		commandLed(0xC0);
		wordWrite("Current alarm: ");

		unsigned int digit[4] = {ALHOUR/10,ALHOUR - 10*digit[0],ALMIN/10,ALMIN - 10*digit[2]};
		printTime(digit);

		commandLed(0x94);
		wordWrite("Enter hour: XX");
		commandLed(0xD4);
		wordWrite("Press # to exit");

		commandLed(0xD);	// Set cursor blinking
		commandLed(0xA0);

		while(hrDig10temp < '0' || hrDig10temp > '2'){
			hrDig10temp = KEYPUSHED;
			pauser = hrDig10temp;
			if(hrDig10temp == '#')
				break;			// Exit time change w/o changes
		}
		if(hrDig10temp == '#')
			break;			// Exit time change w/o changes

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);

		wordWrite("Current alarm: ");
		printTime(digit);

		commandLed(0x94);
		wordWrite("Enter hour: ");
		charWrite(hrDig10temp);
		wordWrite("X");
		commandLed(0xD4);
		wordWrite("Press # to exit");

		commandLed(0xA1);

		char LimitDigitOne = 0;
		if(hrDig10temp == '2')
			LimitDigitOne = '3';
		else
			LimitDigitOne = '9';

		while(hrDig1temp < '0' || hrDig1temp > LimitDigitOne){
			while(KEYPUSHED == pauser){};
			//pauser = 'x';
			hrDig1temp = KEYPUSHED;
			pauser = hrDig1temp;
			if(hrDig1temp == '#')
				break;			// Exit time change w/o changes
		}
		if(hrDig1temp == '#')
			break;			// Exit time change w/o changes

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);

		wordWrite("Current alarm: ");
		printTime(digit);

		commandLed(0x94);
		wordWrite("Enter minute: XX");
		commandLed(0xD4);
		wordWrite("Press # to exit");

		commandLed(0xA2);

		while(minDig10temp < '0' || minDig10temp > '5'){
			while(KEYPUSHED == pauser){};
			minDig10temp = KEYPUSHED;
			pauser = minDig10temp;
			if(minDig10temp == '#')
				break;			// Exit time change w/o changes
		}
		if(minDig10temp == '#')
			break;			// Exit time change w/o changes

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);

		wordWrite("Current alarm: ");
		printTime(digit);

		commandLed(0x94);
		wordWrite("Enter minute: ");
		charWrite(minDig10temp);
		wordWrite("X");
		commandLed(0xD4);
		wordWrite("Press # to exit");

		commandLed(0xA3);


		while(minDig1temp < '0' || minDig1temp > '9'){
			while(KEYPUSHED == pauser){};
			pauser = 'x';
			minDig1temp = KEYPUSHED;
			if(minDig1temp == '#')
				break;			// Exit time change w/o changes
		}
		if(minDig1temp == '#')
			break;			// Exit time change w/o changes

		ALHOUR = (hrDig10temp - 48)*10 + (hrDig1temp-48);
		ALMIN = (minDig10temp - 48)*10 + (minDig1temp-48);
	}
	commandLed(0x0c);
}

void setTime(void){
	while(KEYPUSHED == 'A'){
		while(KEYPUSHED == 'A'){};
		char hrDig10temp = 0;
		char hrDig1temp = 0;
		char minDig10temp = 0;
		char minDig1temp = 0;
		char pauser;

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);
		wordWrite("Enter hour: XX");
		commandLed(0x94);
		wordWrite("Press # to exit");

		commandLed(0xD);	// Set cursor blinking
		commandLed(0xCC);	// Move cursor to X

		while(hrDig10temp < '0' || hrDig10temp > '2'){
			hrDig10temp = KEYPUSHED ;
			pauser = hrDig10temp;
			if(hrDig10temp == '#')
				break;			// Exit time change w/o changes
		}
		if(hrDig10temp == '#')
			break;			// Exit time change w/o changes

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);
		wordWrite("Enter hour: ");
		charWrite(hrDig10temp);
		wordWrite("X");
		commandLed(0x94);
		wordWrite("Press # to exit");

		commandLed(0xCD);	// Move cursor to second X

		char LimitDigitOne = 0;
		if(hrDig10temp == '2')
			LimitDigitOne = '3';
		else
			LimitDigitOne = '9';

		while(hrDig1temp < '0' || hrDig1temp > LimitDigitOne){
			while(KEYPUSHED == pauser){};
			hrDig1temp = KEYPUSHED ;
			pauser = hrDig1temp;
			if(hrDig10temp == '#')
				break;			// Exit time change w/o changes
		}
		if(hrDig1temp == '#')
			break;			// Exit time change w/o changes

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);
		wordWrite("Enter minute: XX");
		commandLed(0x94);
		wordWrite("Press # to exit");
		commandLed(0xCE);	// Move cursor to X

		while(minDig10temp < '0' || minDig10temp > '5'){
			while(KEYPUSHED ==pauser){};
			minDig10temp = KEYPUSHED ;
			pauser = minDig10temp;
			if(minDig10temp == '#')
				break;			// Exit time change w/o changes
		}
		if(minDig10temp == '#')
			break;			// Exit time change w/o changes

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);
		wordWrite("Enter minute: ");
		charWrite(minDig10temp);
		wordWrite("X");
		commandLed(0x94);
		wordWrite("Press # to exit");
		commandLed(0xCF);	// Move cursor to second X

		while(minDig1temp < '0' || minDig1temp > '9'){
			while(KEYPUSHED == pauser){};
			pauser = 'x';
			minDig1temp = KEYPUSHED ;
			if(minDig10temp == '#')
				break;			// Exit time change w/o changes
		}
		if(minDig1temp == '#')
			break;			// Exit time change w/o changes

		CCR = 0b10010;
		CCR = 0b10000;
		HOUR = (hrDig10temp - 48)*10 + (hrDig1temp-48);
		MIN = (minDig10temp - 48)*10 + (minDig1temp-48);
		SEC = 0;
		CCR = 0b10001;
	}
	commandLed(0x0c);
}
