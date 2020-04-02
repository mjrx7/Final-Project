/*
 * TimerLPC.cpp
 *
 *  Created on: Feb 24, 2020
 *      Author: Matthew Johnson & Talia Nguyen
 */
#define T0TCR (*(volatile unsigned int *) 0x40004004)
#define T0TC (*(volatile unsigned int *) 0x40004008)

void timerStart(void){
	T0TCR |= (1 << 0);
}

void timerStop(void){
	T0TCR &= ~(1 << 0);
}

void timerReset(void){
	T0TCR |= (1 << 1);
	while(T0TC != 0){}
	T0TCR &= ~(1 << 1);
}

int timerRead_us(void){
	return T0TC;
}

float timerRead_ms(void){
	return (float)timerRead_us()/1000;
}

float timerRead(void){
	return timerRead_us()/1000000.0;
}

void wait_us(int us){
	int startTime;
	timerStart();
	startTime = timerRead_us();
	while((timerRead_us() - startTime) < us){}
}

void wait_ms(float s){
	wait_us(s * 985.0);//1000.0);
}

void wait(float s){
	wait_us(s * 985000.0);//1000000.0);
}
