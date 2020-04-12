/*
 * LPC1769_MEM_DEFINITIONS.h
 *
 *  Created on: Apr 3, 2020
 *      Author: mjrx7
 */

#ifndef LPC1769_RTC_DEFINITIONS_H_
#define LPC1769_RTC_DEFINITIONS_H_

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
#define ALARM_INTERRUPT ((ILR >> 1) & 1)

#define T0TCR (*(volatile unsigned int *) 0x40004004)
#define T0MR0 (*(volatile unsigned int *) 0x40004018)
#define T0MCR (*(volatile unsigned int *) 0x40004014)
#define T0TCR (*(volatile unsigned int *) 0x40004004)
#define ISER0 (*(volatile unsigned int *) 0xE000E100)
#define T0TC (*(volatile unsigned int *) 0x40004008)
#define T0IR (*(volatile unsigned int *) 0x40004000)

#define FIO0DIR (*(volatile unsigned int *)0x2009c000)
#define FIO0PIN (*(volatile unsigned int *)0x2009c014)

#define ISER0 (*(volatile unsigned int *) 0xE000E100)
#define IO0IntEnR (*(volatile unsigned int *) 0x40028090)
#define IO0IntEnF (*(volatile unsigned int *) 0x40028094)
#define IO0IntClr (*(volatile unsigned int *) 0x4002808C)
#define IO0IntStatR (*(volatile unsigned int *) 0x40028084)
#define IO0IntStatF (*(volatile unsigned int *) 0x40028088)
#define IOIntStatus (*(volatile unsigned int *) 0x40028080)

#define GPREG0 (*(volatile unsigned int *) 0x40024044)

#endif /* LPC1769_RTC_DEFINITIONS_H_ */
