/*
===============================================================================
 Name        : Peter Hettwer
 Author      : 
 Version     : 0x01
 Copyright   : I like to count potatoes.
 Description : Lab3
===============================================================================
*/

#include "LPC17xx.h"
#include "..\LCD.h"

#include "stdio.h"

//define statements

#define Button1 (1<<2)
#define Button2 (1<<3)
#define BUTTON1_PRESSED (!(((LPC_GPIO0 -> FIOPIN) & Button1) == Button1))
#define BUTTON2_PRESSED (!(((LPC_GPIO0 -> FIOPIN) & Button2) == Button2))

//global variables

volatile unsigned long i;
int enable = 1;
int hours = 0;
int minutes = 0;
int seconds = 0;
int milsecs = 0;
char buffer[20];

//interrupt functions


void EINT3_IRQHandler(void)
{
	if(BUTTON1_PRESSED) //start_stop button
	{
		enable ^= 1;
		for(i=0;i<900000;i++);

	}
	if(BUTTON2_PRESSED) //reset button
	{
		hours = 0;
		minutes = 0;
		seconds = 0;
		milsecs = 0;
		sprintf(buffer,"            ");
	}
	LPC_GPIOINT -> IO0IntClr |= (Button1 + Button2); //clear interrupt flag
}

void TIMER0_IRQHandler(void)
{
	if(enable)
	{
		milsecs = milsecs + 1;
		if(milsecs>=1000)
		{
			milsecs = 0;
			seconds++;
		}
		if(seconds>=60)
		{
			seconds = 0;
			minutes++;
		}
		if(minutes>=60)
		{
			minutes = 0;
			hours++;
		}
	}
	LPC_TIM0 -> IR = 1;
}

//main function


int main(void)
{
    LPC_GPIOINT -> IO0IntEnF |= Button1 + Button2;
    LPC_TIM0 -> MCR = 0x0003;
    LPC_TIM0 -> TCR = 1;
    LPC_TIM0 -> MR0 = 25000;


    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_EnableIRQ(TIMER0_IRQn);

    NVIC_SetPriority(EINT3_IRQn, 10);
    NVIC_SetPriority(TIMER0_IRQn,12);

    lcd_init();
    fillScreen(ST7735_16_WHITE);
    setColor16(ST7735_16_BLACK);
    setBackgroundColor16(ST7735_16_WHITE);



    while(1)
    {
    	sprintf(buffer, "%02d:%02d:%02d:%03d",hours,minutes,seconds,milsecs);
    	//for(i=0;i<900000;i++);
        drawString(10,10,buffer);
    }



	return 1 ;
}
