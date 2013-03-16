/*
    FreeRTOS V7.2.0 - Copyright (C) 2012 Real Time Engineers Ltd.


    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    >>>NOTE<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.  FreeRTOS is distributed in the hope that it will be useful, but
    WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
    or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public
    License and the FreeRTOS license exception along with FreeRTOS; if not it
    can be viewed here: http://www.freertos.org/a00114.html and also obtained
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?                                      *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, training, latest information,
    license and contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool.

    Real Time Engineers ltd license FreeRTOS to High Integrity Systems, who sell
    the code with commercial support, indemnification, and middleware, under
    the OpenRTOS brand: http://www.OpenRTOS.com.  High Integrity Systems also
    provide a safety engineered and independently SIL3 certified version under
    the SafeRTOS brand: http://www.SafeRTOS.com.
*/

/*
 * This is a very simple demo that demonstrates task and queue usages only.
 * Details of other FreeRTOS features (API functions, tracing features,
 * diagnostic hook functions, memory management, etc.) can be found on the
 * FreeRTOS web site (http://www.FreeRTOS.org) and in the FreeRTOS book.
 * Details of this demo (what it does, how it should behave, etc.) can be found
 * in the accompanying PDF application note.
 *
*/

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// Other Includes
#include "LCD Source Files/lcd.h"
#include <stdio.h>

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The bit of port 0 that the LPCXpresso LPC13xx LED is connected. */
#define mainLED_BIT 						( 22 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define mainQUEUE_SEND_FREQUENCY_MS			( 500 / portTICK_RATE_MS )

/* The number of items the queue can hold.  This is 1 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define timeQUEUE_LENGTH					( 4 )
#define buttonQUEUE_LENGTH					( 1 )

// Button Push #define's
#define P0_2_PUSHED	(((~(LPC_GPIO0 -> FIOPIN)) & P0_2) == P0_2)
#define P0_3_PUSHED	(((~(LPC_GPIO0 -> FIOPIN)) & P0_3) == P0_3)

typedef enum stopwatch_state
{
	RUNNING = 0,
	STOPPED = 1,
	RESET = 2,
}stopwatch_state;

/*
 * The tasks as described in the accompanying PDF application note.
 */
static void prvQueueReceiveTimeTask( void *pvParameters );
static void prvQueueSendTimeTask( void *pvParameters );
static void prvQueueReceiveButtonTask( void *pvParameters );
static void prvQueueSendButtonTask( void *pvParameters );
void updateLcdButtonData( stopwatch_state currentState );
void updateLcdTimeData( uint32_t tenMs, uint32_t seconds, uint32_t minutes, uint32_t hours );
void updateTime(uint32_t *p_tenMs, uint32_t *p_seconds, uint32_t *p_minutes, uint32_t *p_hours);
stopwatch_state getCurrentState(stopwatch_state previousState);

/*
 * Simple function to toggle the LED on the LPCXpresso LPC17xx board.
 */
static void prvToggleLED( void );

/* The queue used by both tasks. */
static xQueueHandle xQueueTime = NULL;
static xQueueHandle xQueueButton = NULL;
static stopwatch_state state = RUNNING;
static stopwatch_state prevState = RUNNING;
uint32_t tenMs = 0;
uint32_t seconds = 0;
uint32_t minutes = 0;
uint32_t hours = 0;

/*-----------------------------------------------------------*/

int main(void)
{
	/* Initialize P0_22 for the LED. */
	LPC_PINCON->PINSEL1	&= ( ~( 3 << 12 ) );
	LPC_GPIO0->FIODIR |= ( 1 << mainLED_BIT );

	/* Initialize LCD */
	lcd_init();
	fillScreen(ST7735_16_WHITE);
	setBackgroundColor16(ST7735_16_WHITE);
	drawString(1, 1, "Lab6: FreeRTOS");
	drawString(1, 16, "Time: ");

	/* Create the queue. */
	xQueueTime = xQueueCreate( timeQUEUE_LENGTH, sizeof( unsigned long ) );
	xQueueButton = xQueueCreate( buttonQUEUE_LENGTH, sizeof( unsigned long ) );

	if( (xQueueTime != NULL) && (xQueueButton != NULL) )
	{
		/* Start the two tasks as described in the accompanying application
		note. */
		xTaskCreate( prvQueueReceiveTimeTask, ( signed char * ) "Rx_Time", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
		xTaskCreate( prvQueueSendTimeTask, ( signed char * ) "Tx_Time", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );

		xTaskCreate( prvQueueReceiveButtonTask, ( signed char * ) "Rx_Button", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
		xTaskCreate( prvQueueSendButtonTask, ( signed char * ) "Tx_Button", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );

		/* Start the tasks running. */
		vTaskStartScheduler();
	}

	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvQueueSendTimeTask( void *pvParameters )
{
portTickType xNextWakeTime;

	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* Place this task in the blocked state until it is time to run again.
		The block state is specified in ticks, the constant used converts ticks
		to ms.  While in the blocked state this task will not consume any CPU
		time. */
		vTaskDelayUntil( &xNextWakeTime, 10 ); // 10ms

		// update current time if not stopped
		switch (state)
		{
			case RUNNING:
				updateTime(&tenMs,&seconds,&minutes,&hours);
				break;

			case RESET:
				tenMs = 0;
				seconds = 0;
				minutes = 0;
				hours = 0;
				break;

			case STOPPED:
				break;
		}
		/* Send to the queue - causing the queue receive task to flash its LED.
		0 is used as the block time so the sending operation will not block -
		it shouldn't need to block as the queue should always be empty at this
		point in the code. */
		xQueueSend( xQueueTime, &tenMs, 0 );
		xQueueSend( xQueueTime, &seconds, 0 );
		xQueueSend( xQueueTime, &minutes, 0 );
		xQueueSend( xQueueTime, &hours, 0 );
	}
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTimeTask( void *pvParameters )
{
	for( ;; )
	{
		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */
		xQueueReceive( xQueueTime, &tenMs, portMAX_DELAY );
		xQueueReceive( xQueueTime, &seconds, portMAX_DELAY );
		xQueueReceive( xQueueTime, &minutes, portMAX_DELAY );
		xQueueReceive( xQueueTime, &hours, portMAX_DELAY );

		updateLcdTimeData(tenMs, seconds, minutes, hours);
	}
}
/*-----------------------------------------------------------*/

static void prvQueueSendButtonTask( void *pvParameters )
{
portTickType xNextWakeTime;
const unsigned long ulValueToSend = 100UL;

	/* Initialize xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
		/* Place this task in the blocked state until it is time to run again.
		The block state is specified in ticks, the constant used converts ticks
		to ms.  While in the blocked state this task will not consume any CPU
		time. */
		vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

		/* Send to the queue - causing the queue receive task to flash its LED.
		0 is used as the block time so the sending operation will not block -
		it shouldn't need to block as the queue should always be empty at this
		point in the code. */
		xQueueSend( xQueueButton, &state, 0 );
	}
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveButtonTask( void *pvParameters )
{
unsigned long ulReceivedValue;
stopwatch_state currentState;
	for( ;; )
	{
		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */
		xQueueReceive( xQueueButton, &currentState, portMAX_DELAY );

		/*  To get here something must have been received from the queue, but
		is it the expected value?  If it is, toggle the LED. */
		if( ulReceivedValue == 100UL )
		{
			updateLcdButtonData(currentState);
		}
	}
}
/*-----------------------------------------------------------*/

static void prvLED( void )
{
//unsigned long ulLEDState;
//
//	/* Obtain the current P0 state. */
//	ulLEDState = LPC_GPIO0->FIOPIN;
//
//	/* Turn the LED off if it was on, and on if it was off. */
//	LPC_GPIO0->FIOCLR = ulLEDState & ( 1 << mainLED_BIT );
//	LPC_GPIO0->FIOSET = ( ( ~ulLEDState ) & ( 1 << mainLED_BIT ) );
}
/*-----------------------------------------------------------*/

void updateLcdButtonData( stopwatch_state currentState )
{
	char s[30] = { 0 };
	sprintf(s,"Time: 00:00:00.00");

}
/*-----------------------------------------------------------*/

void updateLcdTimeData( uint32_t tenMs, uint32_t seconds, uint32_t minutes, uint32_t hours )
{
	char s[30] = { 0 };
	//vTaskDelay(1250000); //wait 50ms
	sprintf(s,"Time: %02d:%02d:%02d.%02d",hours,minutes,seconds,tenMs);
	drawString(1,16,s);
}
/*-----------------------------------------------------------*/

void updateTime(uint32_t *p_tenMs, uint32_t *p_seconds, uint32_t *p_minutes, uint32_t *p_hours)
{
	*p_tenMs += 10;
	if(*p_tenMs >= 1000)
	{
		*p_tenMs = 0;
		*p_seconds += 1;
	}

	if(*p_seconds >= 60)
	{
		*p_seconds = 0;
		*p_minutes += 1;
	}

	if(*p_minutes >= 60)
	{
		*p_minutes = 0;
		*p_hours += 1;
	}

	if(*p_hours >= 12)
	{
		*p_hours = 0;
	}
}
/*-----------------------------------------------------------*/

stopwatch_state getCurrentState(stopwatch_state previousState)
{
	stopwatch_state currentState;
	if (P0_3_PUSHED) currentState = RESET;
	else if (P0_2_PUSHED && (previousState == RUNNING)) currentState = STOPPED;
	else if (P0_2_PUSHED && ((previousState == STOPPED) | (previousState == RESET))) currentState = RUNNING;
	else currentState = previousState;

	return (currentState);
}
/*-----------------------------------------------------------*/

static void buttonStateTask( void )
{
	while(1)
	{
		vTaskDelay(250000); //wait 10ms
		state = getCurrentState(prevState);
	}

}
/*-----------------------------------------------------------*/

static void LcdTask( void )
{
	while(1)
	{
		vTaskDelay(250000); //wait 10ms
		state = getCurrentState(prevState);
		switch (state)
		{
		case RUNNING:
			updateTime(&tenMs,&seconds,&minutes,&hours);
			break;

		case RESET:
			tenMs = 0;
			seconds = 0;
			minutes = 0;
			hours = 0;
			break;

		case STOPPED:
			break;
		}

	}
}
/*-----------------------------------------------------------*/




