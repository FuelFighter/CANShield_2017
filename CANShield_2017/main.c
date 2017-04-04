/*
 * main.c
 *
 * Created: 04.04.2017 15:03:31
 *  Author: Jorgejac
 */ 

#include "../UniversalModuleDrivers/usbdb.h"
#include "../UniversalModuleDrivers/can.h"
#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/interrupt.h>

#define RX_BUFFER_SIZE 200
#define CAN_MESSAGE_ID_LENGTH 3
#define CAN_MESSAGE_LENGTH_LENGTH 1
#define CAN_MESSAGE_DATA_LENGTH 2

CanMessage_t txFrame;
CanMessage_t rxFrame;
char uartTxBuffer[50];
char uartRxBuffer[RX_BUFFER_SIZE];
int rxIndex = 0;
int parseIndex = 0;
int newUartMessage = 0;

uint8_t ascii_to_dec(char c);

int main(void){
	cli();
	usbdbg_init();
	can_init();
	sei();
	
	while (1)
	{
		if (can_read_message(&rxFrame))
		{
			int index = 0;
			index += sprintf(uartTxBuffer, "[%03X:%d:", rxFrame.id, rxFrame.length);
			
			 for(int i = 0; i < rxFrame.length; i++)
			 {
				 index += sprintf(&uartTxBuffer[index], "%02X", rxFrame.data[i]);
			 }
			 
			 sprintf(&uartTxBuffer[index], "]\n");
			 
			 printf("%s", uartTxBuffer);
		}
		
		if (newUartMessage)
		{
			uint16_t canId = 0;
			uint8_t errorIncrement = 0;
			uint8_t canData = 0;
			
			//Looking for [ and skips it if we find it, error otherwise
			while(uartRxBuffer[parseIndex] != '[')
			{
				parseIndex = (parseIndex+1)%RX_BUFFER_SIZE;
				if (errorIncrement == RX_BUFFER_SIZE)
				{
					printf("ERROR, Could not find CANmessage in uartRxBuffer");
					break;
				} else if (errorIncrement > 1)
				{
					printf("ERROR, Looking for [, increment: %u", errorIncrement-1);
				}
				errorIncrement++;
			}
			errorIncrement = 0;
			
			// Setting CAN-ID
			for (int i = CAN_MESSAGE_ID_LENGTH-1; i >= 0; i--)
			{
				canId += (ascii_to_dec(uartRxBuffer[parseIndex]) << 4*i);
				parseIndex = (parseIndex+1)%RX_BUFFER_SIZE;
			}
			txFrame.id = canId;
			
			//Looking for : and skips it if we find it, error otherwise
			while(uartRxBuffer[parseIndex] != ':')
			{
				parseIndex = (parseIndex+1)%RX_BUFFER_SIZE;
				if (errorIncrement == RX_BUFFER_SIZE)
				{
					printf("ERROR, Could not find CAN length in uartRxBuffer");
					break;
				} else if (errorIncrement > 1)
				{
					printf("ERROR, Looking for :, increment: %u", errorIncrement-1);
				}
				errorIncrement++;
			}
			errorIncrement = 0;
			
			//Translating from ASCII and setting length
			txFrame.length = ascii_to_dec(uartRxBuffer[parseIndex]);
			
			//Looking for : and skips it if we find it, error otherwise
			while(uartRxBuffer[parseIndex] != ':')
			{
				parseIndex = (parseIndex+1)%RX_BUFFER_SIZE;
				if (errorIncrement == RX_BUFFER_SIZE)
				{
					printf("ERROR, Could not find CAN Data in uartRxBuffer");
					break;
				} else if (errorIncrement > 1)
				{
					printf("ERROR, Looking for :, increment: %u", errorIncrement-1);
				}
				errorIncrement++;
			}
			errorIncrement = 0;
			
			//Setting data
			for (int i = 0; i < txFrame.length; i++)
			{
				for (int j = (CAN_MESSAGE_DATA_LENGTH-1); j >= 0; j--)
				{
					canData += (ascii_to_dec(uartRxBuffer[parseIndex]) << 4*i);
					parseIndex = (parseIndex+1)%RX_BUFFER_SIZE;
				}
				txFrame.data[i] = canData;
				canData = 0;
			}
			
			//Looking for ] and skips it if we find it, error otherwise
			while(uartRxBuffer[parseIndex] != ']')
			{
				parseIndex = (parseIndex+1)%RX_BUFFER_SIZE;
				if (errorIncrement == RX_BUFFER_SIZE)
				{
					printf("ERROR, Could not find ] in uartRxBuffer");
					break;
				} else if (errorIncrement > 1)
				{
					printf("ERROR, Looking for ], increment: %u", errorIncrement-1);
				}
				errorIncrement++;
			}
			errorIncrement = 0;
			
			//Looking for \n and skips it if we find it, error otherwise
			while(uartRxBuffer[parseIndex] != '\n')
			{
				parseIndex = (parseIndex+1)%RX_BUFFER_SIZE;
				if (errorIncrement == RX_BUFFER_SIZE)
				{
					printf("ERROR, Could not find end of CAN Data in uartRxBuffer");
					break;
				} else if (errorIncrement > 1)
				{
					printf("ERROR, Looking for \n, increment: %u", errorIncrement-1);
				}
				errorIncrement++;
			}
			errorIncrement = 0;
			can_send_message(&txFrame);
			newUartMessage = 0;
		}
	}
}

ISR(USART0_RX_vect){
	char rxChar = usbdbg_rx_char();
	uartRxBuffer[rxIndex] = rxChar;
	
	rxIndex = (rxIndex+1)%RX_BUFFER_SIZE;
	
	if (rxChar == '\n')
	{	
		newUartMessage = 1;
	}
}

uint8_t ascii_to_dec(char c)
{
	uint8_t dec = 0;
	
	if ((c >= '0') && (c <= '9'))
	{
		dec = (uint8_t)(c - '0');
		
	} else if ((c >= 'A') && (c <= 'F'))
	{
		dec = (uint8_t)(c - 'A');
		
	} else if ((c >= 'a') && (c <= 'f'))
	{
		dec = (uint8_t)(c - 'a');
		
		} else {
		dec = 0;
	}
	
	return dec;
}