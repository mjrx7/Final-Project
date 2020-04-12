/*
===============================================================================
 Name        : main.c
 Author      : Matthew Johnson
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

bool PM = false;				// Display AM/PM status
bool ALARM_ON;					// Set if Alarm is ON
bool ALARM_ACTIVE = false;		// Set of Alarm is ACTIVE
bool SNOOZE = false;			// Set if Snooze is ON
bool SNOOZE_ALREADY = false;	// Set if Snooze has ran ONCE
int SNOOZE_TIME = 1;			// Time to SNOOZE
unsigned int OLD_ALARM;			// Saves old alarm
unsigned int oldState = 0;			// Old QEI State
unsigned int newState = 0;			// New QEI State
unsigned volatile int change = 0;	// Change in State
volatile bool PAGE1 = true;			// Set if PAGE1 display
volatile int QEICOUNT = 0;			// Index for QEI
char KEYPUSHED = 0;				// Returns 4x4 keypushed
volatile bool SNOOZE_QEI = false;
void changeTime(void);			// Change time on display
void printTime(unsigned int);	// Print sub-routine
void setTime(void);				// Set time routine
void setAlarm(void);			// Set alarm routine
void setAlarmSub(unsigned int);	// Sub-routine for setAlarm
void toggleAlarm(void);			// Toggle alarm ON/OFF
void alarm(void);				// ALARM function
void setSnooze(void);			// Set Snooze function
void clockSetup(void);			// RTC Clock Setup function
void _IRQ_SETUP(void);			// IRQ Setup for 4x4 & QEI
void GREETING(void);

extern "C" void TIMER0_IRQHandler(void){
	// Setup timer interrupt every 500 us
	if((T0IR >> 0) & 1){
		T0MR0 = T0MR0 + 500;
		KEYPUSHED = keyPress();
		T0IR = (1 << 0);
	}
}

extern "C" void EINT3_IRQHandler(void){
	// If Interrupt is detected on GPIO0
	// New state of ports 23/26 are detected
	// & combined as XX, compared to oldstate
	if((IOIntStatus >> 0) & 1){
		newState = ((FIO0PIN >> 22) & 0x2) | ((FIO0PIN >> 26) & 0x1);
		change = (((oldState << 1) | (oldState >> 1)) & 0x3) ^ newState;
		switch (change) {
			case 0b01: 	QEICOUNT++;
						if((QEICOUNT%4) == 0){
							if(SNOOZE_QEI){
								if (SNOOZE_TIME == 9)
									SNOOZE_TIME = 1;
								else
									SNOOZE_TIME++;
							}
							else
								PAGE1 = !PAGE1;
						}
						break;
			case 0b10: 	QEICOUNT--;
						if((QEICOUNT%4) == 0){
							if(SNOOZE_QEI){
								if(SNOOZE_TIME == 1)
									SNOOZE_TIME = 9;
								else
									SNOOZE_TIME--;
							}
							else
								PAGE1 = !PAGE1;
						}
						break;
		}
		oldState = newState;
		IO0IntClr = (0b1001 << 23);
	}
}

int main(void) {
	setupHD44780();	// Setup HD44780 display
	setupKeyPad();	// Setup 4x4 Keypad
	_IRQ_SETUP();	// Setup IRQ for Keypad & QEI
	clockSetup();	// Setup RTC
	GREETING();

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

void GREETING(void){
	for(int i = 0; i < 3; i++){
		commandLed(1);
		commandLed(0x86);
		charWrite('/');
		charWrite(4);
		wordWrite("_/");
		charWrite(4);
		commandLed(0xC1);
		wordWrite("/");
		charWrite(4); // \\  / ");
		wordWrite("  / ");
		if((i%2) == 0)
			charWrite(3);
		else
			charWrite('-');
		charWrite(32);
		charWrite(3);
		charWrite(32);
		charWrite(4);
		commandLed(0x94);
		wordWrite("//");
		charWrite(4);
		charWrite(4);
		charWrite(32);
		charWrite(4);
		charWrite(5);
		wordWrite("(*)");
		charWrite(5);
		charWrite('/');
		commandLed(0xD4);
		wordWrite("`  ");
		charWrite(4);
		wordWrite("/   ^ /");
		commandLed(0xe1);
		wordWrite("MJ 2020");
		if(i == 0)
			wait(1.25);
		else if(i == 1)
			wait(0.5);
		else
			wait(3);
	}
}

void setSnooze(void){
	do{
		unsigned int backup = SNOOZE_TIME;
		while(KEYPUSHED == 'D'){};
		char snoozeDig1temp = 0;

		commandLed(1);
		wordWrite("Snooze time: ");
		charWrite(SNOOZE_TIME + 48);
		wordWrite(" min");

		commandLed(0xC0);
		wordWrite("Rotate QEI knob");
		commandLed(0x94);
		wordWrite("Press * to accept");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		commandLed(0xD);
		commandLed(0b10100);
		commandLed(0x8D);

		SNOOZE_QEI = true;
		unsigned int oldDigit = 0;
		while(snoozeDig1temp < '0' || snoozeDig1temp > '9'){
			snoozeDig1temp = KEYPUSHED;
			if(oldDigit != SNOOZE_TIME)
				charWrite((SNOOZE_TIME) + 48);
			oldDigit = SNOOZE_TIME;
			commandLed(0x8D);
			if(snoozeDig1temp == '#')
				break;
			if(snoozeDig1temp == '*'){
				break;
			}
		}
		SNOOZE_QEI = false;
		if(snoozeDig1temp == '*'){
			wait_ms(500);
			break;
		}
		if(snoozeDig1temp == '#'){
			SNOOZE_TIME = backup;
			break;
		}

		if(snoozeDig1temp == '0')
			snoozeDig1temp = '1';
		charWrite(snoozeDig1temp);
		wait_ms(500);
		while(KEYPUSHED == snoozeDig1temp){}
		SNOOZE_TIME = snoozeDig1temp - 48;
	}while(KEYPUSHED == 'D');
	commandLed(0x0c);
}

void _IRQ_SETUP(void){
	T0TCR |= 1;	// Start Timer
	T0MR0 = T0TC + 500; // Interrupt 1000 us into future
	T0MCR |= (1 << 0);	// enable interrupt on MR0 match
	ISER0 = (1 << 1);	// Enable interrupts for Timer 0

	IO0IntEnR |= (0b1001 << 23);	// Setup GPIO0 Interrupt on 23/26 Rising
	IO0IntEnF |= (0b1001 << 23);	// Setup GPIO0 Interrupt on 23/26 Falling
	IO0IntClr = (0b1001 << 23);		// Clear GPIO0 Interrupts
	oldState = ((FIO0PIN >> 22) & 0x2) | ((FIO0PIN >> 26) & 0x1);	// Save oldState
	ISER0 = (1 << 21);				// Enable GPIO0/EINT3 Interrupt
}

void toggleAlarm(void){
	while(KEYPUSHED == 'C'){};
	if(!ALARM_ON){
		AMR = 0;
		AMR |= (0x1f << 3);	// Mask Alarm Registers year, month, doy, dow, dom
		commandLed(1);
		commandLed(0xC4);
		wordWrite("Alarm ON");
		wait(1.5);
	}
	else{
		if(OLD_ALARM != ((ALHOUR << 6) | ALMIN)){
			ALHOUR = (OLD_ALARM >> 6);
			ALMIN = OLD_ALARM & 0x3f;
		}
		SNOOZE = false;
		AMR = 0x7f;
		commandLed(1);
		commandLed(0xC4);
		wordWrite("Alarm OFF");
		wait(1.5);
	}
	ALARM_ON = !ALARM_ON;
	GPREG0 = (AMR << 11) | (ALHOUR << 6) | ALMIN;
}

void changeTime(void){
	unsigned int time = CTIME0;
	#define timeInSeconds ((time) & 0x3F)
	#define timeInMinutes ((time >> 8) & 0x3F)
	#define timeInHours ((time >> 16) & 0x1F)

	if(timeInHours >= 12){
		if(timeInHours!=12)
			time = (time & 0xFFE0FFFF) | ((timeInHours - 12) << 16);
		PM = true;
	}
	else
		PM = false;
	unsigned int digit = (1 << 20) | (PM << 19) | (timeInHours/10 << 18) | (timeInHours%10 << 14) | (timeInMinutes/10 << 11) | (timeInMinutes%10 << 7) | (timeInSeconds/10 << 4) | (timeInSeconds%10);

	commandLed(1);
	commandLed(0x84);
	printTime(digit);
	if(!ALARM_ACTIVE){
		if(PAGE1){
			commandLed(0xC0);
			wordWrite("A: Set Time");
			commandLed(0x94);
			wordWrite("B: Set Alarm");
			commandLed(0xD4);
			wordWrite("C: On/Off (");
			if(ALARM_ON){
				wordWrite("ON)");
				charWrite(2);
			}
			else
				wordWrite("OFF)");
			if(SNOOZE){
				wordWrite("Zzz ");
				charWrite(0);
			}
			else{
				wordWrite("    ");
				charWrite(0);
			}
		}
		else{
			commandLed(0xC0);
			wordWrite("D: Set Snooze");
			if(ALARM_ON){
				commandLed(0xE2);
				charWrite(2);
			}
			commandLed(0xE7);
			charWrite(1);
		}
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
		if(!SNOOZE_ALREADY){
			OLD_ALARM = (ALHOUR << 6) | ALMIN;
			SNOOZE_ALREADY = true;
		}
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
	else{
		SNOOZE = false;
		SNOOZE_ALREADY = false;
		ALMIN = OLD_ALARM & 0x3f;
		ALHOUR = (OLD_ALARM >> 6);
	}
	while(off == KEYPUSHED){}
	_alarm = 0;
	ILR |= (1 << 1);	// Clear Alarm
	ALARM_ACTIVE = false;
}

void clockSetup(void){
	AMR = (GPREG0 >> 11);
	ALHOUR = (GPREG0 >> 6) & 0x1f;
	ALMIN = (GPREG0 >> 0) & 0x3f;
	if(AMR == 0xf8)
		ALARM_ON = true;
	else{
		AMR = 0x7f;
		ALARM_ON = false;
	}
	OLD_ALARM = (ALHOUR << 6) | ALMIN;
	CCR = 0b10010;	// Disable clock[0], reset CTC[1], cal counter disabled[4]
	CCR = 0b10000;	// Reset CTC[1] is removed
	ALSEC = 0;
	CCR = 0b10001;	// Enable clock[0], cal counter disabled[4]
}

void printTime(unsigned int digit){
	if(((digit >> 18)&1) == 0 && ((digit >> 14)&0xf) == 0){
		digit &= 0x83FFF;
		digit|= (1 << 18) | (2 << 14);
	}
	charWrite(((digit >> 18) & 1) + 48);
	charWrite(((digit >> 14) & 0xf) + 48);
	charWrite(58);
	charWrite(((digit >> 11) & 0x7) + 48);
	charWrite(((digit >> 7) & 0xf) + 48);
	if(digit >> 20){
		charWrite(58);
		charWrite(((digit >> 4) & 0x7) + 48);
		charWrite(((digit >> 0) & 0xf) + 48);
	}
	if((digit >> 19) & 1)
		wordWrite(" PM");
	else
		wordWrite(" AM");
}

void setAlarmSub(unsigned int digit){
	commandLed(1);
	wordWrite("Change alarm time");
	commandLed(0xC0);
	wordWrite("Current: ");
	printTime(digit);
	commandLed(0x94);
}

void setAlarm(void){
	while(KEYPUSHED=='B'){
		while(KEYPUSHED == 'B'){};
		commandLed(0xD);	// Set cursor blinking
		char hrDig10temp = 0;
		char hrDig1temp = 0;
		char minDig10temp = 0;
		char minDig1temp = 0;
		char AMorPM;
		char pauser;
		bool STATUS_OF_TIME_OF_DAY = false;
		unsigned int tempTime = ALHOUR;
		if(tempTime >= 12){
			STATUS_OF_TIME_OF_DAY = true;
			if(tempTime > 12)
				tempTime -= 12;
		}
		unsigned int digit = (STATUS_OF_TIME_OF_DAY << 19) | ((tempTime/10) << 18) | ((tempTime%10) << 14) | (ALMIN/10 << 11) | ((ALMIN%10) << 7);
		setAlarmSub(digit);
		wordWrite("XX:XX AM/PM");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		commandLed(0x94);	// Set cursor at first X

		while(hrDig10temp < '0' || hrDig10temp > '1'){
			hrDig10temp = KEYPUSHED;
			pauser = hrDig10temp;
			if(hrDig10temp == '#')
				break;			// Exit time change w/o changes
		}
		if(hrDig10temp == '#')
			break;			// Exit time change w/o changes

		setAlarmSub(digit);
		charWrite(hrDig10temp);
		wordWrite("X:XX AM/PM");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		commandLed(0x95);

		char LimitDigitOne = 0;
		if(hrDig10temp == '1')
			LimitDigitOne = '2';
		else
			LimitDigitOne = '9';
		while(hrDig1temp < '0' || hrDig1temp > LimitDigitOne){
			while(KEYPUSHED == pauser){}
			hrDig1temp = KEYPUSHED;
			pauser = hrDig1temp;
			if(hrDig1temp == '#')
				break;			// Exit time change w/o changes
		}
		if(hrDig1temp == '#')
			break;			// Exit time change w/o changes

		setAlarmSub(digit);
		charWrite(hrDig10temp);
		charWrite(hrDig1temp);
		wordWrite(":XX AM/PM");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		commandLed(0x97);

		while(minDig10temp < '0' || minDig10temp > '5'){
			while(KEYPUSHED == pauser){}
			minDig10temp = KEYPUSHED;
			pauser = minDig10temp;
			if(minDig10temp == '#')
				break;			// Exit time change w/o changes
		}
		if(minDig10temp == '#')
			break;			// Exit time change w/o changes

		setAlarmSub(digit);
		charWrite(hrDig10temp);
		charWrite(hrDig1temp);
		charWrite(58);
		charWrite(minDig10temp);
		wordWrite("X AM/PM");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		commandLed(0x98);


		while(minDig1temp < '0' || minDig1temp > '9'){
			while(KEYPUSHED == pauser){}
			minDig1temp = KEYPUSHED;
			pauser = minDig1temp;
			if(minDig1temp == '#')
				break;			// Exit time change w/o changes
		}
		if(minDig1temp == '#')
			break;			// Exit time change w/o changes

		setAlarmSub(digit);
		charWrite(hrDig10temp);
		charWrite(hrDig1temp);
		charWrite(58);
		charWrite(minDig10temp);
		charWrite(minDig1temp);
		wordWrite(" 1:AM 2:PM _");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		commandLed(0xA4);
		//commandLed(0x0c);
		while(AMorPM < '1' || AMorPM > '2'){
			while(KEYPUSHED == pauser){}
			pauser = AMorPM;
			AMorPM = KEYPUSHED;
			if(AMorPM == '#')
				break;
		}
		if(AMorPM == '#')
			break;
		if(AMorPM == '2')
			ALHOUR = (hrDig10temp - 48)*10 + (hrDig1temp-48) + 12;
		else{
			ALHOUR = (hrDig10temp - 48)*10 + (hrDig1temp-48);
			if(ALHOUR == 12)
				ALHOUR = 0;
		}
		ALMIN = (minDig10temp - 48)*10 + (minDig1temp-48);
		OLD_ALARM = (ALHOUR << 6) | ALMIN;
		unsigned int amrtemp = AMR;
		amrtemp &= 0x7;
		GPREG0 = (AMR << 11) | (ALHOUR << 6) | ALMIN;
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
		wordWrite("XX:XX AM/PM");
		commandLed(0x94);
		wordWrite("Press # to exit");
		commandLed(0xD);	// Set cursor blinking
		commandLed(0xC0);	// Move cursor to X
		while(hrDig10temp < '0' || hrDig10temp > '1'){
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
		charWrite(hrDig10temp);
		wordWrite("X:XX AM/PM");
		commandLed(0x94);
		wordWrite("Press # to exit");
		commandLed(0xC1);	// Move cursor to second X
		char LimitDigitOne = 0;
		if(hrDig10temp == '1')
			LimitDigitOne = '2';
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
		charWrite(hrDig10temp);
		charWrite(hrDig1temp);
		wordWrite(":XX AM/PM");
		commandLed(0x94);
		wordWrite("Press # to exit");
		commandLed(0xC3);	// Move cursor to X

		while(minDig10temp < '0' || minDig10temp > '5'){
			while(KEYPUSHED ==pauser){}
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
		charWrite(hrDig10temp);
		charWrite(hrDig1temp);
		charWrite(58);
		charWrite(minDig10temp);
		wordWrite("X AM/PM");
		commandLed(0x94);
		wordWrite("Press # to exit");
		commandLed(0xC4);	// Move cursor to second X

		while(minDig1temp < '0' || minDig1temp > '9'){
			while(KEYPUSHED == pauser){}
			pauser = 'x';
			minDig1temp = KEYPUSHED ;
			if(minDig10temp == '#')
				break;			// Exit time change w/o changes
		}
		if(minDig1temp == '#')
			break;			// Exit time change w/o changes
		pauser = minDig1temp;
		commandLed(1);
		wordWrite("Change current time");
		commandLed(0xC0);
		charWrite(hrDig10temp);
		charWrite(hrDig1temp);
		charWrite(58);
		charWrite(minDig10temp);
		charWrite(minDig1temp);
		wordWrite(" AM/PM");
		commandLed(0x94);
		wordWrite("1: AM      2: PM");
		commandLed(0xD4);
		wordWrite("Press # to exit");
		commandLed(0x0C);
		char TimeOfDay = 0;	// AM or PM

		while(TimeOfDay < '1' || TimeOfDay > '2'){
			while(KEYPUSHED == pauser){}
			pauser = 'x';
			TimeOfDay = KEYPUSHED;
			if(TimeOfDay == '#')
				break;
		}
		if(TimeOfDay == '#')
			break;

		CCR = 0b10010;
		CCR = 0b10000;

		if(TimeOfDay == '1'){
			if(hrDig10temp == '1' && hrDig1temp == '2'){
				hrDig10temp = '0';
				hrDig1temp = '0';
			}
			HOUR = (hrDig10temp - 48)*10 + (hrDig1temp-48);
		}
		else if(TimeOfDay == '2'){
			if(hrDig10temp == '1' && hrDig1temp == '2'){
				HOUR = (hrDig10temp - 48)*10 + (hrDig1temp-48);
			}
			else
				HOUR = (hrDig10temp - 48)*10 + (hrDig1temp-48) + 12;
		}


		MIN = (minDig10temp - 48)*10 + (minDig1temp-48);
		SEC = 0;
		CCR = 0b10001;
	}
	commandLed(0x0c);
}
