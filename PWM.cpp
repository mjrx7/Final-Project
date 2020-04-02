/*
 * PWM.cpp
 *
 *  Created on: Mar 17, 2020
 *      Author: mjrx7
 */

#include "PWM.h"

#define PCONP (*(volatile unsigned int *) 0x400FC0C4)
#define PCLKSEL0 (*(volatile unsigned int *) 0x400FC1A8)
#define PINSEL4 (*(volatile unsigned int *) 0x4002C010)
#define PWM1PR (*(volatile unsigned int *) 0x4001800C)
#define PWM1MR0 (*(volatile unsigned int *) 0x40018018)
#define PWM1MR1 (*(volatile unsigned int *) 0x4001801C)
#define PWM1MR2 (*(volatile unsigned int *) 0x40018020)
#define PWM1MCR (*(volatile unsigned int *) 0x40018014)
#define PWM1LER (*(volatile unsigned int *) 0x40018050)
#define PWM1PCR (*(volatile unsigned int *) 0x4001804C)
#define PWM1TCR (*(volatile unsigned int *) 0x40018004)
#define PWM1TC (*(volatile unsigned int *) 0x40018008)

unsigned int PWM::_period;
unsigned int PWM::_frequency;
bool PWM::runOnce = false;

PWM::PWM()
{
	_period = 2000;
	_period_check = _period;
	_pulseWidth = 0;
	_frequency = 1000000 / _period;
	_dutyCycle = 0;
	//setupPWM();
}

PWM::PWM(PIN pin)
{
	_pin = pin;
	if(!runOnce){
		_period = 20000;
		if(_period==0)
			_frequency = 0;
		else
			_frequency = 1000000 / _period;
	}
	_period_check = _period;
	_pulseWidth = 0;//_period/2;
	_dutyCycle = 0;//(float)_pulseWidth / _period;
	setupPWM(pin);
}

unsigned int PWM::getPulseWidth(void)
{
	return _pulseWidth;
}

unsigned int PWM::getPeriod(void)
{
	return _period;
}

float PWM::getDC(void)
{
	return _dutyCycle;
}

unsigned int PWM::getFrequency(void)
{
	return _frequency;
}

void PWM::setFrequency(unsigned int freq){
	_frequency = freq;
	if(_frequency==0)
		_period = 0;
	else
		_period = 1000000 / freq;
	_period_check = _period;
	_pulseWidth = _dutyCycle * _period;
	PWM1MR0 = _period;
	PWM1MR1 = _pulseWidth;
	PWM1MR2 = _pulseWidth;
	PWM1LER = (1 << 0) | (1 << _pin);//2) | (1<<1); //Load the MR1 new value at start of next cycle

}

void PWM::setPeriod(unsigned int period)
{
	_period = period;
	_period_check = _period;
	if(_period==0)
		_frequency = 0;
	else
		_frequency = 1000000 / _period;
	_pulseWidth = _dutyCycle * _period;
	PWM1MR0 = _period;
	PWM1MR1 = _pulseWidth;
	PWM1MR2 = _pulseWidth;
	PWM1LER = (1 << 0) | (1 << _pin);//2) | (1<<1); //Load the MR1 new value at start of next cycle

}

void PWM::setPulseWidth(unsigned int pulseWidth)
{
	_pulseWidth = pulseWidth;
	_dutyCycle = 1. * _pulseWidth / _period;
	PWM1MR1 = _pulseWidth;
	PWM1MR2 = _pulseWidth;
	PWM1LER = (1 << 2) | (1<<1); //Load the MR1 new value at start of next cycle
}

void PWM::setDC(float DC)
{
	if(DC != _dutyCycle  || _period_check != _period){
		if(DC > 1)
			DC = 1;
		else if(DC < 0)
			DC = 0;
		_dutyCycle = DC;
		_pulseWidth = _dutyCycle * _period;
		if(_pin == PWM1_1)
			PWM1MR1 = _pulseWidth;
		else if(_pin == PWM1_2)
			PWM1MR2 = _pulseWidth;
		PWM1LER = (1 << _pin);	//Load the MR1 new value at start of next cycle
		if(_period_check != _period){
			PWM1TCR = (1 << 1);	// Reset PWM TC & PR
			PWM1TCR = (1 << 0) | (1 << 3);	// Enable counters & PWM mode
			_period_check = _period;
		}
	}
}

void PWM::setupPWM(PIN pin)
{
	/*
	 * Need to correct PINSEL4, write now sets pin mode for PWM1.1 and PWM1.2
	 * auto to PWM mode. Need to make optional depending on object being initialized
	 *
	 */

	if(runOnce == false){
		PCONP |= (1 << 6);	// Power on PWM default ON
		PCLKSEL0 &= ~(1 << 12);	// Divide by 4 default divide/4
		PINSEL4 |= (0b01 << 0) | (0b01 << 2);	// Set pin 42 to PWM 1.1  pin 43 tp PWM 1.2

		PWM1PCR = 0;	// Sets single edge PWM
		PWM1PR = 0;		// Prescale Register Amount to increment TC counter by in addition
		runOnce = true;
	}
	PWM1MR0 = _period;
	if(pin==PWM1_1)
		PWM1MR1 = _pulseWidth;
	else if(pin==PWM1_2)
		PWM1MR2 = _pulseWidth;
	PWM1MCR = (1 << 1);							// Reset PWM TC on PWM1MR0 match
	PWM1LER = (1 << 0) | (1 << pin);			// Update values in MR0 & MR1

	PWM1PCR |= (1 << (pin+8));
	/*if(pin==PWM1_1)
		PWM1PCR |= (1 << 9);					// Enable PWM output
	else if(pin==PWM1_2)
		PWM1PCR |= (1 << 10);*/

	PWM1TCR = (1 << 1);							// Reset PWM TC & PR
	PWM1TCR = (1 << 0) | (1 << 3);				// Enable counters & PWM mode
}
