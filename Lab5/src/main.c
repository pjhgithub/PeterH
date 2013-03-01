/*
===============================================================================
 Name        : main.c
 Author      : Gerardo Zamora & Peter Hettwer
 Version     : I like to count potatoes
 Copyright   : Aww Yiss
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <NXP/crp.h>

// Variable to store CRP value in. Will be placed automatically
// by the linker when "Enable Code Read Protect" selected.
// See crp.h header for more information
__CRP const unsigned int CRP_WORD = CRP_NO_CRP ;

// Includes
#include "lcd.h"
#include <stdio.h>

// function prototypes
void I2C_SendData(uint8_t data);
void I2C_ReceiveData(uint8_t *data);
void I2C_Read(uint8_t deviceId, uint8_t address, uint8_t *dataRead);
void I2C_Write(uint8_t deviceId, uint8_t address, uint8_t dataWrite);
void I2C_StartBit( void );
void I2C_StopBit( void );

// Button Push #define's
#define P0_2_PUSHED	(((~(LPC_GPIO0 -> FIOPIN)) & P0_2) == P0_2)
#define P0_3_PUSHED	(((~(LPC_GPIO0 -> FIOPIN)) & P0_3) == P0_3)

// I2C #define's
#define SCL_START 	((~(LPC_GPIO1 -> FIOPIN) & P1_20) == P1_20)
#define SDA_START	((~(LPC_GPIO1 -> FIOPIN) & P1_22) == P1_22)
#define DEVICE_ID	(0b10100000)
#define START_BIT	(0x01)
#define STOP_BIT	(0x00)

typedef enum RW_Type
{
	IDLE = 0x00,
	READ = 0x01,
	WRITE = 0x02
}RW_Type;

RW_Type RW = IDLE;

uint32_t i = 0;
char s[20] = {0};

int main(void)
{
	LPC_PINCON -> PINMODE_OD1 |= P1_20 + P1_22;
	LPC_GPIO1 -> FIODIR |= P1_20 + P1_22;

	// Lines High
	LPC_GPIO1 -> FIOSET = P1_22; 	// SDA high
	delay_us(100);					// wait
	LPC_GPIO1 -> FIOSET = P1_20; 	// SCL high
	delay_us(100);					// wait

	uint8_t readByte = 0;
	uint32_t readAddress = 0x00;

	uint8_t writeByte = 0x01;
	uint32_t writeAddress = 0x00;

	// Initialize LCD
	lcd_init();
	fillScreen(ST7735_16_WHITE);
	setBackgroundColor16(ST7735_16_WHITE);
	drawString(1, 1, "I2C bit-banging");

	while(1)
	{
		if(P0_2_PUSHED && (!P0_3_PUSHED)) // Das write
		{
			RW = WRITE;
			for(i = 0; i < 900000; i++);
		}
		else if((!P0_2_PUSHED) && P0_3_PUSHED) // Das read
		{
			RW = READ;
			for(i = 0; i < 900000; i++);
		}
		else
		{
			RW = IDLE;
		}

		switch(RW)
		{

			case READ:
				I2C_Read(DEVICE_ID, readAddress, &readByte);
				sprintf(s, "read: %X", readByte);
				drawString(1, 20, s);
				break;

			case WRITE:
				I2C_Write(DEVICE_ID, writeAddress, writeByte);
				drawString(1, 10, "It-a works");
				break;

			case IDLE:
				// Do nothing
			default:
				break;
		}

	}
	return 0;
}

void I2C_SendData(uint8_t data)
{
	uint32_t x = 0;
	uint32_t ACK = 0;

	LPC_GPIO1 -> FIODIR |= P1_22;

	// Send byte
//		do
//		{
			// MSB first
			for(x = 0; x < 8; x++)
			{
				if(data & 0x80) LPC_GPIO1 -> FIOSET = (P1_22);
				else 			LPC_GPIO1 -> FIOCLR = (P1_22);

				delay_us(100);
				LPC_GPIO1 -> FIOSET = (P1_20);
				delay_us(100);
				LPC_GPIO1 -> FIOCLR = (P1_20);
				delay_us(100);
				data <<= 1;
			}

			// Make sure SDA is high
			LPC_GPIO1 -> FIOSET = (P1_22);

			// Set P1_22 to input: Set data line to input
			LPC_GPIO1 -> FIODIR &= ~(P1_22);

			// SCL Low
			LPC_GPIO1 -> FIOCLR = (P1_20);
			delay_us(100);

			// Receive Acknowledge
			LPC_GPIO1 -> FIOSET = (P1_20);
			delay_us(300);
			ACK = !(((LPC_GPIO1 -> FIOPIN) & (P1_22)) == P1_22);
			LPC_GPIO1 -> FIOCLR = (P1_20);
			delay_us(300);

			// Set P1_22 to output: Set data line to output
			LPC_GPIO1 -> FIODIR |= (P1_22);
//		}
//		while(!ACK);
}

void I2C_ReceiveData(uint8_t *data)
{
	uint32_t x = 0;
	uint32_t currentBit = 0;

	// Set data buffer to 0.
	*data = 0x00;

	// Make sure SDA is high
	LPC_GPIO1 -> FIOSET = (P1_22);

	// Set P1_22 to input: Set data line to input
	LPC_GPIO1 -> FIODIR &= ~(P1_22);

	// SCL Low
	LPC_GPIO1 -> FIOCLR = (P1_20);
	delay_us(100);

	for(x = 8; x > 0; x--)
	{
		LPC_GPIO1 -> FIOSET = (P1_20);
		delay_us(200);

		currentBit = (((LPC_GPIO1 -> FIOPIN) & (P1_22)) == P1_22)? 1:0;

		*data += (currentBit << (x-1));

		LPC_GPIO1 -> FIOCLR = (P1_20);
		delay_us(200);
	}

	LPC_GPIO1 -> FIOSET = (P1_20);
	delay_us(100);
	LPC_GPIO1 -> FIOCLR = (P1_20);
	delay_us(100);

	// Set P1_22 to output: Set data line to output
	LPC_GPIO1 -> FIODIR |= (P1_22);
}


void I2C_Read(uint8_t deviceId, uint8_t address, uint8_t *pDataRead)
{
	I2C_StartBit();

	// Send Device ID
	I2C_SendData(deviceId);

	// Send Address
	I2C_SendData(address);

	delay_us(100);

	I2C_StopBit();

	delay_us(100);

	I2C_StartBit();

	// Send Device ID
	I2C_SendData(deviceId+1);

	// Receive data
	I2C_ReceiveData(pDataRead);

	delay_us(100);

	I2C_StopBit();

	delay_us(100);


	RW = IDLE;
}

void I2C_Write(uint8_t deviceId, uint8_t address, uint8_t dataWrite)
{
	I2C_StartBit();

	// Send Device ID
	I2C_SendData(deviceId);

	// Send Address
	I2C_SendData(address);

	// Send Data
	I2C_SendData(dataWrite);

	I2C_StopBit();

	RW = IDLE;
}

void I2C_StartBit(void)
{
	// Lines high
	LPC_GPIO1 -> FIOSET = P1_22; 	// SDA high
	delay_us(100);					// wait
	LPC_GPIO1 -> FIOSET = P1_20; 	// SCL high
	delay_us(100);					// wait

	// Pull lines low
	LPC_GPIO1 -> FIOCLR = P1_22; 	// SDA low
	delay_us(100);					// wait
	LPC_GPIO1 -> FIOCLR = P1_20; 	// SCL low
	delay_us(100);					// wait
}

void I2C_StopBit(void)
{
	// SDA high
	LPC_GPIO1 -> FIOCLR = P1_22; 	// SDA high
	delay_us(100);					// wait

	// Stop condition
	LPC_GPIO1 -> FIOSET = P1_20; 	// SCL high
	delay_us(100);					// wait
	LPC_GPIO1 -> FIOSET = P1_22; 	// SDA high
	delay_us(100);					// wait
}
