/*
 * HD44780.h
 *
 *  Created on: Apr 1, 2020
 *      Author: mjrx7
 */

#ifndef HD44780_H_
#define HD44780_H_

void BusoutWrite(unsigned int);//uint8_t);
void commandLed(unsigned int);//uint8_t);
void charWrite(unsigned int);//uint8_t);
void setupHD44780(void);
void wordWrite(char*);

#endif /* HD44780_H_ */
