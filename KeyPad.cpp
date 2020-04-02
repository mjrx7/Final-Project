/*
 * KeyPad.cpp
 *
 *  Created on: Apr 1, 2020
 *      Author: mjrx7
 */
#include "KeyPad.h"
// Registers for port 0
#define FIO0DIR (*(volatile unsigned int *)0x2009c000)
#define FIO0PIN (*(volatile unsigned int *)0x2009c014)

/*
 *
 * p0.23    p0.27,p0.24,p0.25,p0.28    p0.26,	Inputs
 * p0.2,p0.3,p0.21,p0.22	Outputs
 *
 */
const int inputs[] = {27,24,25,28};
const int outputs[] = {2,3,21,22};

char keyPress(void){
	/*
	 * Checks column 1, I assume bit 3 does column 2,
	 * bit 21 column 3, bit 22 column 4
	 *
	 */

	// Column 1
	FIO0PIN &= ~(1 << outputs[0]);		// Set p0.2 = 0
	FIO0PIN |= (1 << outputs[1]);	// Set p0.21,p0.22,p0.3 = 1
	FIO0PIN |= (1 << outputs[2]);
	FIO0PIN |= (1 << outputs[3]);
	if(!((FIO0PIN >> inputs[0]) & 1)){
		return '1';
	}
	else if(!((FIO0PIN >> inputs[1]) & 1)){
		return '4';
	}
	else if(!((FIO0PIN >> inputs[2]) & 1)){
		return '7';
	}
	else if(!((FIO0PIN >> inputs[3]) & 1)){
		return '*';
	}

	// Column 2
	FIO0PIN &= ~(1 << outputs[1]);		// Set p0.3 = 0
	FIO0PIN |= (1 << outputs[2]);	// Set p0.21,p0.22,p0.2 = 1
	FIO0PIN |= (1 << outputs[3]);
	FIO0PIN |= (1 << outputs[0]);
	if(!((FIO0PIN >> inputs[0]) & 1)){
		return '2';
	}
	else if(!((FIO0PIN >> inputs[1]) & 1)){
		return '5';
	}
	else if(!((FIO0PIN >> inputs[2]) & 1)){
		return '8';
	}
	else if(!((FIO0PIN >> inputs[3]) & 1)){
		return '0';
	}

	// Column 3
	FIO0PIN &= ~(1 << outputs[2]);		// Set p0.21 = 0
	FIO0PIN |= (1 << outputs[3]);	// Set p0.2,p0.3,p0.22 = 1
	FIO0PIN |= (1 << outputs[0]);
	FIO0PIN |= (1 << outputs[1]);
	if(!((FIO0PIN >> inputs[0]) & 1)){
		return '3';
	}
	else if(!((FIO0PIN >> inputs[1]) & 1)){
		return '6';
	}
	else if(!((FIO0PIN >> inputs[2]) & 1)){
		return '9';
	}
	else if(!((FIO0PIN >> inputs[3]) & 1)){
		return '#';
	}

	// Column 4
	FIO0PIN &= ~(1 << outputs[3]);	// Set p0.22 = 0
	FIO0PIN |= (1 << outputs[0]);	// Set p0.21,p0.2,p0.3 = 1
	FIO0PIN |= (1 << outputs[1]);
	FIO0PIN |= (1 << outputs[2]);
	if(!((FIO0PIN >> inputs[0]) & 1)){
		return 'A';
	}
	else if(!((FIO0PIN >> inputs[1]) & 1)){
		return 'B';
	}
	else if(!((FIO0PIN >> inputs[2]) & 1)){
		return 'C';
	}
	else if(!((FIO0PIN >> inputs[3]) & 1)){
		return 'D';
	}

	return 0;
}

void setupKeyPad(void){
	// Setup Keypad Pins
	for(int i = 0; i < 4; i++){
		FIO0DIR |= (1 << outputs[i]);
		FIO0DIR &= ~(1 << inputs[i]);
	}
}
