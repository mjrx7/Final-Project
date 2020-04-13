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
void setupHD44780(void);
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

void setupHD44780(void){
	/*
	 * Setup Pins
	 * RS: p0.9
	 * En: p0.8
	 * data bits: lsb 7,6,0,1,18,17,15,16 msb
	 */
	wait_ms(20);		// Display needs 10 ms to come online
	FIO0DIR |= (0x783C3);
	commandLed(0x38);	// Function Set
	commandLed(0x0C);	// Display ON/OFF control
	commandLed(0x06);	// Entry mode set
	commandLed(0x01);	// Clears display
	wait_ms(400);


	// Custome character setup
	commandLed(0x40);
	//Up Arrow(0)
	charWrite(0x4);
	charWrite(0x4);
	charWrite(0x4);
	charWrite(0x4);
	charWrite(0x1f);
	charWrite(0xE);
	charWrite(0x4);
	charWrite(0x0);
	// Down Arrow(1)
	charWrite(0x0);
	charWrite(0x4);
	charWrite(0xE);
	charWrite(0x1f);
	charWrite(0x4);
	charWrite(0x4);
	charWrite(0x4);
	charWrite(0x4);
	// Alarm Bell(2)
	charWrite(0x4);
	charWrite(0xe);
	charWrite(0xe);
	charWrite(0xe);
	charWrite(0xe);
	charWrite(0x1f);
	charWrite(0x4);
	charWrite(0x0);
	// Cat eye(3)
	charWrite(0b00000);
	charWrite(0b00000);
	charWrite(0b01110);
	charWrite(0b01010);
	charWrite(0b01110);
	charWrite(0b00000);
	charWrite(0b00000);
	charWrite(0b00000);
	// \ character(4)
	charWrite(0b00000);
	charWrite(0b10000);
	charWrite(0b01000);
	charWrite(0b00100);
	charWrite(0b00010);
	charWrite(0b00001);
	charWrite(0b00000);
	charWrite(0b00000);
	// ~ Character(5)
	charWrite(0b00000);
	charWrite(0b00000);
	charWrite(0b01000);
	charWrite(0b10101);
	charWrite(0b00010);
	charWrite(0b00000);
	charWrite(0b00000);
	charWrite(0b00000);
	// Stick man 1 (6)
	charWrite(0b01110);
	charWrite(0b01010);
	charWrite(0b01110);
	charWrite(0b00101);
	charWrite(0b01110);
	charWrite(0b10100);
	charWrite(0b01010);
	charWrite(0b01001);
	// Stick man 1 (7)
	charWrite(0b01110);
	charWrite(0b01010);
	charWrite(0b01110);
	charWrite(0b10100);
	charWrite(0b01111);
	charWrite(0b00100);
	charWrite(0b00111);
	charWrite(0b11001);
	// Stick man 1 (8)
	/*charWrite(0b01110);
	charWrite(0b01010);
	charWrite(0b01110);
	charWrite(0b00100);
	charWrite(0b11111);
	charWrite(0b00101);
	charWrite(0b01110);
	charWrite(0b10001);*/

	commandLed(0);
	// Save custom character
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
		wait_ms(2);
}

void BusoutWrite(unsigned int x){//uint8_t x){
	for(int i=0; i < 8; i++){
		if((x >> i) & 1)
			FIO0PIN |= (1 << BusoutBits[i]);
		else
			FIO0PIN &= ~(1 << BusoutBits[i]);
	}
}
