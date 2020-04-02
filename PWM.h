/*
 * PWM.h
 *
 *  Created on: Mar 17, 2020
 *      Author: mjrx7
 */

#ifndef PWM_H_
#define PWM_H_

typedef enum {
	PWM1_0,PWM1_1,PWM1_2,PWM1_3,PWM1_4,PWM1_5
} PIN;

class PWM
{
protected:
	static bool runOnce;
	unsigned int _period_check;
	static unsigned int _period;
	static unsigned int _frequency;
	unsigned int _pulseWidth;
	float _dutyCycle;
	PIN _pin;
	void setupPWM(PIN pin);
public:
	PWM();
	PWM(PIN pin);
	PWM(const PWM& m);
	~PWM();
	PWM &operator= (float value){setDC(value); return *this;}
	operator float(){return getDC();}
	float getDC();
	unsigned int getPeriod();
	unsigned int getPulseWidth();
	unsigned int getFrequency();
	void setFrequency(unsigned int freq);
	void setPeriod(unsigned int period);
	void setPulseWidth(unsigned int pulseWidth);
	void setDC(float DC);
};



#endif /* PWM_H_ */
