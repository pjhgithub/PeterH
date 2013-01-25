////////////////////////////////////////////////////////////////////////
//Peter Hettwer
//ECE 471
//Lab 1
//Blinking LED w/ pushbutton enable/disable
////////////////////////////////////////////////////////////////////////


//start code
#include "LPC17xx.h"

#define LED0 (1<<19)
#define LED1 (1<<20)
#define LED2 (1<<21)
#define LED3 (1<<22)
#define LED4 (1<<23)
#define LED5 (1<<24)
#define LED6 (1<<25)
#define LED7 (1<<26)

#define Button1 (1<<2)

//global variables

int enable = 1;
volatile unsigned long m;

//interrupt functions


void EINT3_IRQHandler(void)
{
	NVIC_DisableIRQ(EINT3_IRQn);
	enable = !enable;
	for(m = 0;m<900000;m++)
	NVIC_EnableIRQ(EINT3_IRQn);
	LPC_GPIOINT -> IO0IntClr |= Button1;
}

//main function


int main(void)
{
	LPC_GPIO1 -> FIODIR |= LED0;
	LPC_GPIO1 -> FIODIR |= LED1;
	LPC_GPIO1 -> FIODIR |= LED2;
	LPC_GPIO1 -> FIODIR |= LED3;
	LPC_GPIO1 -> FIODIR |= LED4;
	LPC_GPIO1 -> FIODIR |= LED5;
	LPC_GPIO1 -> FIODIR |= LED6;
	LPC_GPIO1 -> FIODIR |= LED7;

	LPC_GPIOINT ->IO0IntEnR |= Button1;
	NVIC_EnableIRQ(EINT3_IRQn);


	int LED[8] = {LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7};
	int i;
	LPC_GPIO1 -> FIOPIN |= LED0 + LED1 + LED2 + LED3 + LED4 + LED5 + LED6 + LED7;
	while(1)
	{
			for(i = 0;i<=7;i++)
			{
				if(enable)
				{
				LPC_GPIO1 -> FIOPIN |= LED[i-1] + LED[i+1];
				LPC_GPIO1 -> FIOPIN &= ~LED[i];
				for(m=0;m<800000;m++);
				}
			}
			for(i = 7;i>=0;i--)
			{
				if(enable)
				{
				LPC_GPIO1 -> FIOPIN |= LED[i-1] + LED[i+1];
				LPC_GPIO1 -> FIOPIN &= ~LED[i];
				for(m=0;m<800000;m++);
				}
			}
		}
return 1;
}
