/*
===============================================================================
 Name        : Peter Hettwer
 Author      : 
 Version     : 0x01
 Copyright   : I like to count potatoes.
 Description : Lab4
===============================================================================
*/

#include "LPC17xx.h"
#include "..\LCD.h"

#include "stdio.h"

//define statements

#define Button1 (1<<2)
#define BUTTON1_PRESSED (!(((LPC_GPIO0 -> FIOPIN) & Button1) == Button1))
#define UART_RXD (1<<2)
#define UART_Receive (!(((LPC_GPIO2 -> FIOPIN) & UART_RXD) == UART_RXD))
#define UART_Bit (LPC_GPIO2 -> FIOPIN)
#define clear ('                         ')


//global variables

volatile unsigned long i;
int sample = 0;
int enable = 1;
int cx = 0;
int cy = 0;
int data = 0;
int bit = 0;
int count = 0;

//interrupt functions


void EINT3_IRQHandler(void)
{
	if(BUTTON1_PRESSED) //reset button
	{
		fillScreen(ST7735_16_WHITE);
		//drawString(10,10,clear); //clear LCD and reset carriage
		cx = 0;
		cy = 0;

	}
	if(UART_Receive && enable) //start data acquisition
	{
		enable = 0;
		NVIC_EnableIRQ(TIMER0_IRQn);
	}
	LPC_GPIOINT -> IO0IntClr |= Button1; //clear interrupt flag
	LPC_GPIOINT -> IO2IntClr |= UART_Receive; //clear interrupt flag
}

void TIMER0_IRQHandler(void)
{
	sample ^=1;
	if(sample)
	{
		if(count > 8)
		{
			bit = UART_Bit;
			data |= (bit<<count);
			count++;
		}
		if(count == 8)
		{
			count = 0;
			LPC_TIM0 -> TCR = 1;
		}
	}
	NVIC_DisableIRQ(TIMER0_IRQn);
	LPC_TIM0 -> IR = 1;
}

//main function


int main(void)
{
    LPC_GPIOINT -> IO0IntEnF |= Button1;
    LPC_GPIOINT -> IO2IntEnF |= UART_Receive;
    LPC_TIM0 -> MCR = 0x0003;
    LPC_TIM0 -> TCR = 0;
    LPC_TIM0 -> MR0 = 1302; //set timer to interrupt at 2x computer/arm baud rate

    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_SetPriority(EINT3_IRQn, 10);
    NVIC_SetPriority(TIMER0_IRQn,12);

    lcd_init();
    fillScreen(ST7735_16_WHITE);
    setColor16(ST7735_16_BLACK);
    setBackgroundColor16(ST7735_16_WHITE);



    while(1)
    {
        drawChar(10,10,data);
    }



	return 1 ;
}
