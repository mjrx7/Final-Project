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

bool PM = false;
bool ALARM_ON = false;
bool ALARM_ACTIVE = false;
bool SNOOZE = false;

char temp = 0;

void setTime(void);
void setAlarm(void);
void alarm(void);
void clockSetup(void);
void changeTime(void);
void toggleAlarm(void);

int main(void) {
	wait(2);
	setupHD44780();
	setupKeyPad();
	clockSetup();

    while(1) {
    	temp = keyPress();

    	if(ALARM_INTERRUPT)
    		alarm();
    	if(temp == 'A')
    		setTime();
    	if(temp == 'B')
			setAlarm();
    	if(temp == 'C')
    		toggleAlarm();
    	changeTime();
		wait(.2);

    }
    return 0 ;
}

void toggleAlarm(void){
	while(keyPress()=='C'){};
	if(!ALARM_ON){
		AMR = 0;
		AMR |= (0x1f << 3);	// Mask Alarm Registers year, month, doy, dow, dom
		commandLed(1);
		commandLed(0xC0);
		charWrite(32);
		charWrite(32);
		charWrite(32);
		charWrite(32);
		wordWrite("Alarm On");
		wait(1);
	}
	else{
		AMR = 1;
		commandLed(1);
		commandLed(0xC0);
		charWrite(32);
		charWrite(32);
		charWrite(32);
		charWrite(32);
		wordWrite("Alarm Off");
		wait(1);
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
	unsigned int digit10 = timeInHours/10;
	unsigned int digit1 = timeInHours - 10*digit10;
	charWrite(digit10 + 48);
	charWrite(digit1 + 48);

	digit10 = timeInMinutes/10;
	digit1 = timeInMinutes - 10*digit10;
	charWrite(58);
	charWrite(digit10 + 48);
	charWrite(digit1 + 48);

	digit10 = timeInSeconds/10;
	digit1 = timeInSeconds - 10*digit10;
	charWrite(58);
	charWrite(digit10 + 48);
	charWrite(digit1 + 48);
	charWrite(32);
	if(PM){
		wordWrite("PM");
	}
	else{
		wordWrite("AM");
	}
	if(temp != 0){
		charWrite(32);
		charWrite(temp);
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
		wordWrite("Press ANY for snooze");
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
		if((off = keyPress()) != 0)
			break;

		_alarm.setFrequency(960);
		if((off = keyPress()) != 0)
			break;

		wait(0.7);
		if((off = keyPress()) != 0)
			break;

		changeTime();
		if((off = keyPress()) != 0)
			break;

		_alarm.setFrequency(770);
		if((off = keyPress()) != 0)
			break;

		wait(0.7);
		off = keyPress();
	}

	if(off != 'D'){
		ALMIN = ALMIN + 1;
		SNOOZE = true;
	}
	else
		SNOOZE = false;
	_alarm = 0;
	ILR |= (1 << 1);	// Clear Alarm
	ALARM_ACTIVE = false;
}

void clockSetup(void){
	AMR = 1;
	//AMR = 0;
	//AMR |= (0x1f << 3);	// Mask Alarm Registers year, month, doy, dow, dom
	CCR = 0b10010;
	CCR = 0b10000;
	SEC = 35;
	MIN = 20;
	HOUR = 16;
	ALSEC = 0;
	ALMIN = 0;
	ALHOUR = 0;
	CCR = 0b10001;
}

void setAlarm(void){
	while(temp=='B'){
		while(keyPress()=='B'){};
		char hrDig10temp = 0;
		char hrDig1temp = 0;
		char minDig10temp = 0;
		char minDig1temp = 0;
		char pauser;

		commandLed(1);
		wordWrite("Change alarm time");
		commandLed(0xC0);
		wordWrite("Current alarm: ");

		unsigned int digit10 = ALHOUR/10;
		unsigned int digit1 = ALHOUR - 10*digit10;
		unsigned int digit10min = ALMIN/10;
		unsigned int digit1min = ALMIN - 10*digit10min;
		charWrite(digit10 + 48);
		charWrite(digit1 + 48);
		charWrite(58);
		charWrite(digit10min + 48);
		charWrite(digit1min + 48);

		commandLed(0x94);
		wordWrite("Enter hour: XX");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		while(hrDig10temp == 0){
			hrDig10temp = keyPress();
			pauser = hrDig10temp;
		}
		if(hrDig10temp == '#')
			break;			// Exit time change w/o changes

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);

		wordWrite("Current alarm: ");
		charWrite(digit10 + 48);
		charWrite(digit1 + 48);
		charWrite(58);
		charWrite(digit10min + 48);
		charWrite(digit1min + 48);

		commandLed(0x94);
		wordWrite("Enter hour: ");
		charWrite(hrDig10temp);
		wordWrite("X");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		while(hrDig1temp == 0){
			while(keyPress()==pauser){};
			//pauser = 'x';
			hrDig1temp = keyPress();
			pauser = hrDig1temp;
		}
		if(hrDig1temp == '#')
			break;			// Exit time change w/o changes

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);

		wordWrite("Current alarm: ");
		charWrite(digit10 + 48);
		charWrite(digit1 + 48);
		charWrite(58);
		charWrite(digit10min + 48);
		charWrite(digit1min + 48);

		commandLed(0x94);
		wordWrite("Enter minute: XX");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		while(minDig10temp == 0){
			while(keyPress()==pauser){};
			minDig10temp = keyPress();
			pauser = minDig10temp;
		}
		if(minDig10temp == '#')
			break;			// Exit time change w/o changes

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);

		wordWrite("Current alarm: ");
		charWrite(digit10 + 48);
		charWrite(digit1 + 48);
		charWrite(58);
		charWrite(digit10min + 48);
		charWrite(digit1min + 48);

		commandLed(0x94);
		wordWrite("Enter minute: ");
		charWrite(minDig10temp);
		wordWrite("X");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		while(minDig1temp == 0){
			while(keyPress()==pauser){};
			pauser = 'x';
			minDig1temp = keyPress();
		}
		if(minDig1temp == '#')
			break;			// Exit time change w/o changes

		ALHOUR = (hrDig10temp - 48)*10 + (hrDig1temp-48);
		ALMIN = (minDig10temp - 48)*10 + (minDig1temp-48);

		temp = 0;
	}
}

void setTime(void){
	while(temp=='A'){
		while(keyPress()=='A'){};
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
		while(hrDig10temp == 0){
			hrDig10temp = keyPress();
			pauser = hrDig10temp;
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
		while(hrDig1temp == 0){
			while(keyPress()==pauser){};
			//pauser = 'x';
			hrDig1temp = keyPress();
			pauser = hrDig1temp;
		}
		if(hrDig1temp == '#')
			break;			// Exit time change w/o changes

		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);
		wordWrite("Enter minute: XX");
		commandLed(0x94);
		wordWrite("Press # to exit");
		while(minDig10temp == 0){
			while(keyPress()==pauser){};
			minDig10temp = keyPress();
			pauser = minDig10temp;
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
		while(minDig1temp == 0){
			while(keyPress()==pauser){};
			pauser = 'x';
			minDig1temp = keyPress();
		}
		if(minDig1temp == '#')
			break;			// Exit time change w/o changes

		CCR = 0b10010;
		CCR = 0b10000;
		HOUR = (hrDig10temp - 48)*10 + (hrDig1temp-48);
		MIN = (minDig10temp - 48)*10 + (minDig1temp-48);
		SEC = 0;
		CCR = 0b10001;
		temp = 0;
	}
}
