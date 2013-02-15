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

#define LED0 (1<<19)
#define Button1 (1<<2)
#define BUTTON1_PRESSED (~(((LPC_GPIO0 -> FIOPIN) & Button1) == Button1))
#define UART_RXD (1<<1)
#define UART_Receive (~(((LPC_GPIO2 -> FIOPIN) & UART_RXD) == UART_RXD))
#define UART_Bit (((LPC_GPIO2 -> FIOPIN) & UART_RXD) >> 1)


//global variables

volatile unsigned long i;
int sample = 0;
int enable = 1;
int data = 0x00;
int bit = 0;
int count = 0;
int flag = 0;

//interrupt functions


void EINT3_IRQHandler(void)
{
	if(UART_Receive) //start data acquisition
	{
		enable = 0;
		LPC_TIM0 -> TCR &= 0xFFFFFFFC;
		LPC_TIM0 -> TCR |= 0x00000001;
	}
	LPC_GPIOINT -> IO0IntClr |= Button1; //clear interrupt flag write 0xffff if issues
	LPC_GPIOINT -> IO2IntClr |= UART_RXD; //clear interrupt flag
}

void TIMER0_IRQHandler(void)
{
	sample = !sample;
	if(sample)
	{
		if(count < 9)
		{
			bit = UART_Bit;
			if(count != 0)
			{
				data += (bit<<(count-1));
			}
			count++;
		}
		else
		{
			count = 0;
			LPC_TIM0 -> TCR &= 0xFFFFFFFE;
			flag = 1;
			sample = 0;
		}
	}
	LPC_TIM0 -> IR = 1;
}

//main function


int main(void)
{
    //LPC_GPIOINT -> IO0IntEnF |= Button1;
    LPC_GPIOINT -> IO2IntEnF |= UART_Receive;
    LPC_TIM0 -> MCR = 0x0003;
    LPC_TIM0 -> TCR &= 0xFFFFFFFC;
    LPC_TIM0 -> MR0 = 1302; //set timer to interrupt at 2x computer/arm baud rate

    NVIC_EnableIRQ(EINT3_IRQn);
    NVIC_EnableIRQ(TIMER0_IRQn);
//    NVIC_SetPriority(EINT3_IRQn, 12);
//    NVIC_SetPriority(TIMER0_IRQn,10);

    lcd_init();
    fillScreen(ST7735_16_WHITE);
    setColor16(ST7735_16_BLACK);
    setBackgroundColor16(ST7735_16_WHITE);

    while(1)
    {
    	if(flag)
    	{
    		drawChar(10,10,data);
    		data = 0x00;
    		flag = 0;
    		enable = 1;
    	}
        //for(i=0;i<900000;i++);
    }



	return 1 ;
}
