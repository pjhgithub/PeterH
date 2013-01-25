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

//main function

int main(void)
{
	LPC_GPIO -> FIODIR |= LED0;
	LPC_GPIO -> FIODIR |= LED1;
	LPC_GPIO -> FIODIR |= LED2;
	LPC_GPIO -> FIODIR |= LED3;
	LPC_GPIO -> FIODIR |= LED4;
	LPC_GPIO -> FIODIR |= LED5;
	LPC_GPIO -> FIODIR |= LED6;
	LPC_GPIO -> FIODIR |= LED7;

	int LED[] = {LED0, LED1, LED2, LED3, LED4, LED5, LED6, LED7};
	int k = 0;
	int enable = 1;
	
	while(1)
	{
		if(k==0)
		{
			for(i = 0;i<=7;i++)
			{
				if(Button1)
				{
					enable = !enable;
				}
				else if(i==0)
				{
					LPC_GPIO -> FIOPIN |= LED[i];
				}
				else if(i>0 & i<7)
				{
					LPC_GPIO -> FIOPIN |= !LED[(i-1)];
					LPC_GPIO -> FIOPIN |= LED[i];
				}
				else
				{
					LPC_GPIO -> FIOPIN |= !LED[(i-1)];
					LPC_GPIO -> FIOPIN |= LED[i];
					k = 1;
				}
				pause(100);
			}
		}

		else
		{
			for(i = 7;i>=0;i--)
				{
					if (Button1)
					{
						Button1 = !Button1;
					}
					else if(i==7)
					{
						LPC_GPIO -> FIOPIN |= LED[i];
					}
					else if(i>0 & i<7)
					{
						LPC_GPIO -> FIOPIN |= !LED[(i+1)];
						LPC_GPIO -> FIOPIN |= LED[i];
					}
					else
					{
						LPC_GPIO -> FIOPIN |= !LED[(i+1)];
						LPC_GPIO -> FIOPIN |= LED[i];
						k = 0;
					}
				}
			pause(100);
		}

	}

return 1;
}
