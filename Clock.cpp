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
#include "PWM.h"

#define SEC (*(volatile unsigned int *) 0x40024020)
#define MIN (*(volatile unsigned int *) 0x40024024)
#define HOUR (*(volatile unsigned int *) 0x40024028)
#define CTIME0 (*(volatile unsigned int *) 0x40024014)
#define CCR (*(volatile unsigned int *) 0x40024008)

#define ALSEC (*(volatile unsigned int *) 0x40024060)
#define ALMIN (*(volatile unsigned int *) 0x40024064)
#define ALHOUR (*(volatile unsigned int *) 0x40024068)
#define AMR (*(volatile unsigned int *) 0x40024010)
// Mask year,mon,doy,dow,dom,hour,min,sec
#define ILR (*(volatile unsigned int *) 0x40024000)

bool PM = false;
char temp = 0;

void changeTime(void);
void setAlarm(void);

int main(void) {
	AMR = 0;
	AMR |= (0x1f << 3);	// Mask Alarm Registers year, month, doy, dow, dom
	wait(2);
	setup();
	setupKeyPad();
	CCR = 0b10010;
	CCR = 0b10000;
	SEC = 35;
	MIN = 20;
	HOUR = 8;
	ALSEC = 0;
	ALMIN = 21;
	ALHOUR = 15;
	CCR = 0b10001;

    unsigned int time = 0;
	unsigned int timeInSeconds = 0;
    unsigned int timeInMinutes = 0;
    unsigned int timeInHours = 0;

    PWM alarm(PWM1_1);
    alarm.setFrequency(770);
    while(1) {
    	if(((ILR >> 1) & 1) == 1){
    		ILR |= (1 << 1);
    		alarm = 0.5;
    		for(int i=0; i < 5; i++){
    			commandLed(1);
    			alarm.setFrequency(960);
    			wait(0.7);
    			wordWrite("Alarm!!!");
    			alarm.setFrequency(770);
    			wait(0.7);
    		}
    		alarm = 0;
    	}
    	if(temp == 'A'){
    		changeTime();
    	}
    	while(temp == 'B'){
			while(keyPress()=='B'){};
			char temp2 = 0;
			char temp3 = 0;
			char pauser;
			while(temp2 == 0){
				temp2 = keyPress();
				pauser = temp2;
			}
			while(temp3 == 0){
				while(keyPress()==pauser){};
				pauser = 'x';
				temp3 = keyPress();
			}
			CCR = 0b10010;
			CCR = 0b10000;
			MIN = (temp2 - 48)*10+(temp3-48);
			SEC = 0;
			CCR = 0b10001;
			temp = 0;
		}
    	temp = keyPress();
    	time = CTIME0;
        timeInSeconds = (time) & 0x3F;
        timeInMinutes = (time >> 8) & 0x3F;
        timeInHours = (time >> 16) & 0x1F;
        if(timeInHours >= 12){
        	if(timeInHours!=12)
        		timeInHours = timeInHours - 12;
        	PM = true;
        }
        else
        	PM = false;
        unsigned int digit10 = timeInHours/10;
        unsigned int digit1 = timeInHours - 10*digit10;

        commandLed(1);
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
		wait(.2);

    }
    return 0 ;
}

void changeTime(void){
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
