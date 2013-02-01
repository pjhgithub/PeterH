/*
===============================================================================
 Name        : Peter Hettwer
 Class		 : ECE 471
 Assignment	 : Lab 2
===============================================================================
*/
#include "LPC17xx.h"

//start code

#define LED0 (1<<19)
#define LED1 (1<<20)
#define LED2 (1<<21)
#define LED3 (1<<22)
#define LED4 (1<<23)
#define LED5 (1<<24)
#define LED6 (1<<25)
#define LED7 (1<<26)

#define Button1 (1<<2)
#define Button2 (1<<3)

//global variables

volatile unsigned long i;

//interrupt functions

void EINT0_IRQHandler(void)
{
	while(1)
	{
		//second blinking LED (hardware interrupt controlled)
		LPC_GPIO1 -> FIOPIN |= LED0 + LED2;
		LPC_GPIO1 -> FIOPIN &= ~LED1;
		for(i=0;i<900000;i++);
		LPC_GPIO1 -> FIOPIN |= LED1;
		for(i=0;i<900000;i++);
	}
}
void EINT3_IRQHandler(void)
{
	while(1)
	{
		//third blinking LED(software )
		LPC_GPIO1 -> FIOPIN |= LED0 + LED1;
		LPC_GPIO1 -> FIOPIN &= ~LED2;
		for(i=0;i<900000;i++);
		LPC_GPIO1 -> FIOPIN |= LED2;
		for(i=0;i<900000;i++);
	}
}

//main function

int main(void) {
	LPC_GPIO1 -> FIODIR |= LED0;
	LPC_GPIO1 -> FIODIR |= LED1;
	LPC_GPIO1 -> FIODIR |= LED2;
	LPC_GPIO1 -> FIODIR |= LED3;
	LPC_GPIO1 -> FIODIR |= LED4;
	LPC_GPIO1 -> FIODIR |= LED5;
	LPC_GPIO1 -> FIODIR |= LED6;
	LPC_GPIO1 -> FIODIR |= LED7;
	NVIC_EnableIRQ(EINT0_IRQn);
	NVIC_EnableIRQ(EINT3_IRQn);
	
	NVIC_SetPriority(EINT3_IRQn, 0);
	NVIC_SetPriority(EINT0_IRQn, 1);
	LPC_GPIOINT -> IO0IntEnR |= Button1;



	LPC_GPIO1 -> FIOPIN |= LED0 + LED1 + LED2 + LED3 + LED4 + LED5 + LED6 + LED7;
	while(1)
	{
		//set enable bit for second led flash
		if((~(LPC_GPIO0 -> FIOPIN) & Button2) == Button2)
		{
			NVIC_SetPendingIRQ(EINT0_IRQn);
		}

		//main blinking LED
		LPC_GPIO1 -> FIOPIN &= ~LED0;
		for(i=0;i<900000;i++);
		LPC_GPIO1 -> FIOPIN |= LED0;
		for(i=0;i<900000;i++);


	}

	return 1;
}
