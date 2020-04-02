/*
 * HD44780.cpp
 *
 *  Created on: Apr 1, 2020
 *      Author: mjrx7
 */
#include "TimerLPC.h"

// Registers for port 0
#define FIO0DIR (*(volatile unsigned int *)0x2009c000)
#define FIO0PIN (*(volatile unsigned int *)0x2009c014)

void BusoutWrite(unsigned int);//uint8_t);
void commandLed(unsigned int);//uint8_t);
void charWrite(unsigned int);//uint8_t);
void setup(void);
void wordWrite(char*);

/*
 * RS p0.9
 * E p0.8
 * bits lsb - msb
 * p0.7,p0.6,p0.0,p0.1,p0.18,p0.17,p0.15,p0.16
 *
 */
const int BusoutBits[8] = {7,6,0,1,18,17,15,16};

void wordWrite(char* word){
	for(int i = 0; word[i] != '\0'; i++)
		charWrite(word[i]);
}

void setup(void){
	/*
	 * Setup Pins
	 * RS: p0.9
	 * En: p0.8
	 * data bits: lsb 7,6,0,1,18,17,15,16 msb
	 */
	FIO0DIR |= (0x783C3);
	commandLed(0x38);
	commandLed(0x0C);
	commandLed(0x06);
	commandLed(0x01);
	//commandLed(0x10);
	wait_ms(400);

	// Setup Keypad Pins
	/*for(int i = 0; i < 4; i++){
		FIO0DIR |= (1 << outputs[i]);
		FIO0DIR &= ~(1 << inputs[i]);
	}*/
}

void charWrite(unsigned int data){//uint8_t data){
	BusoutWrite(data & 0xff);
	FIO0PIN |= (1 << 9);
	FIO0PIN |= (1 << 8);
	FIO0PIN &= ~(1 << 8);
	wait_us(100);
}

void commandLed(unsigned int cmd){//uint8_t cmd){
	BusoutWrite(cmd);
	FIO0PIN &= ~(1 << 9);
	FIO0PIN |= (1 << 8);
	FIO0PIN &= ~(1 << 8);
	wait_us(100);
	if(cmd==1)
		wait_ms(3);
}

void BusoutWrite(unsigned int x){//uint8_t x){
	for(int i=0; i < 8; i++){
		if((x >> i) & 1)
			FIO0PIN |= (1 << BusoutBits[i]);
		else
			FIO0PIN &= ~(1 << BusoutBits[i]);
	}
}
