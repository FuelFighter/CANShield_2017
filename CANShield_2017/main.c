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

CanMessage_t txFrame;
CanMessage_t rxFrame;
char uartTxBuffer[50];
char uartRxBuffer[RX_BUFFER_SIZE];
int rxIndex = 0;
int parseIndex = 0;
int newUartMessages = 0;
uint8_t error = 0;

uint8_t ascii_to_dec(char c);
void handling_error();

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
		
		if (newUartMessages)
		{
			uint16_t canId = 0;
			uint8_t errorIncrement = 0;
			uint8_t canData = 0;
			
			//Looking for [ and skips it if we find it, error otherwise
			while(uartRxBuffer[parseIndex] != '[')
			{
				parseIndex = (parseIndex + 1) % RX_BUFFER_SIZE;
				if (errorIncrement == RX_BUFFER_SIZE)
				{
					printf("ERROR, Could not find start of CANmessage in uartRxBuffer\n");
					handling_error();
					break;
				} else if (errorIncrement > 1)
				{
					printf("ERROR, Looking for [, increment: %u\n", errorIncrement-1);
				}
				errorIncrement++;
			}
			
			if (error == 0)
			{
				parseIndex = (parseIndex + 1) % RX_BUFFER_SIZE;
				
				
				// Setting CAN-ID
				canId += (ascii_to_dec(uartRxBuffer[parseIndex]) << 8);
				parseIndex = (parseIndex + 1) % RX_BUFFER_SIZE;
				canId += (ascii_to_dec(uartRxBuffer[parseIndex]) << 4);
				parseIndex = (parseIndex + 1) % RX_BUFFER_SIZE;
				canId += ascii_to_dec(uartRxBuffer[parseIndex]);
				parseIndex = (parseIndex + 1) % RX_BUFFER_SIZE;
				printf("CAN id: %03X\n",canId);
				txFrame.id = canId;
				
				//Jump over :
				parseIndex = (parseIndex + 1) % RX_BUFFER_SIZE;
				
				//Translating from ASCII and setting length
				txFrame.length = ascii_to_dec(uartRxBuffer[parseIndex]);
				parseIndex = (parseIndex+1) % RX_BUFFER_SIZE;
				
				printf("CAN Length: %u\n",txFrame.length);
				
				//Jump over :
				parseIndex = (parseIndex+1) % RX_BUFFER_SIZE;
				
				//Setting data
				for (int i = 0; i < txFrame.length; i++)
				{
					canData = (ascii_to_dec(uartRxBuffer[parseIndex]) << 4);
					parseIndex = (parseIndex+1) % RX_BUFFER_SIZE;
					
					canData |= (ascii_to_dec(uartRxBuffer[parseIndex]));
					parseIndex = (parseIndex+1) % RX_BUFFER_SIZE;
					
					txFrame.data[i] = canData;
					printf("Can Data %d: %02X\n", i, txFrame.data[i]);
				}
				
				//Jump over ]
				parseIndex = (parseIndex+1) % RX_BUFFER_SIZE;
				//Jump over \n
				parseIndex = (parseIndex+1) % RX_BUFFER_SIZE;
				
				can_send_message(&txFrame);
				newUartMessages-- ;	
			}
			error--;
		}
	}
}

ISR(USART0_RX_vect){
	char rxChar = usbdbg_rx_char();
	uartRxBuffer[rxIndex] = rxChar;
	rxIndex = (rxIndex + 1) % RX_BUFFER_SIZE;
	
	if (rxChar == '\n')
	{	
		newUartMessages++;
	}
}

uint8_t ascii_to_dec(char c)
{
	uint8_t dec = 0;
	
	if ((c >= '0') && (c <= '9'))
	{
		dec = c - '0';
		
	} else if ((c >= 'A') && (c <= 'F'))
	{
		dec = c - 'A' + 10;
		
	} else if ((c >= 'a') && (c <= 'f'))
	{
		dec = c - 'a' + 10;
		
	} else {
		dec = 0;
	}
	
	return (uint8_t)dec;
}

void handling_error(){
	error++;
	newUartMessages--;
}