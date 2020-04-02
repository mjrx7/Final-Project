/*
 * TimerLPC.h
 *
 *  Created on: Feb 24, 2020
 *      Author: Matthew Johnson & Talia Nguyen
 */

#ifndef TIMERLPC_H_
#define TIMERLPC_H_

void timerStart(void);
void timerStop(void);
void timerReset(void);
int timerRead_us(void);
float timerRead_ms(void);
float timerRead(void);
void wait_us(int);
void wait_ms(float);
void wait(float);

#endif /* TIMERLPC_H_ */
