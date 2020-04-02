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

#define SEC (*(volatile unsigned int *) 0x40024020)
#define MIN (*(volatile unsigned int *) 0x40024024)
#define HOUR (*(volatile unsigned int *) 0x40024028)
#define CTIME0 (*(volatile unsigned int *) 0x40024014)
#define CCR (*(volatile unsigned int *) 0x40024008)

int main(void) {
	wait(2);
	setup();
	setupKeyPad();
	CCR = 0b10010;
	CCR = 0b10000;
	SEC = 0;
	MIN = 20;
	HOUR = 8;
	CCR = 0b10001;

    unsigned int time = 0;
	unsigned int timeInSeconds = 0;
    unsigned int timeInMinutes = 0;
    unsigned int timeInHours = 0;
    bool PM = false;
    char temp = 0;
    while(1) {
    	while(temp == 'A'){
    		while(keyPress()=='A'){};
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
			HOUR = (temp2 - 48)*10+(temp3-48);
			SEC = 0;
			CCR = 0b10001;
			temp = 0;
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
        if(timeInHours > 12){
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
