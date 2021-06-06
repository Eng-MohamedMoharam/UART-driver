/*
 * UART_Driver.c
 *
 * Created: 4/10/2021 8:43:52 AM
 *  Author: m
 */

#include "UART_Driver.h"

#define BAUD_PRESCALE(USART_BAUDRATE) (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
unsigned short int rxBufferSize, strSize, numSize, ackSize;
char *rxBuffer=NULL_VALUE, *rxNumBuffer=NULL_VALUE, *rxStrBuffer= NULL_VALUE, rxAckBuffer[4];
volatile char ch, arrIndex=RESET, strIndex=RESET, numIndex=RESET, ackIndex=RESET, *rxComplete=NULL_VALUE, rxStrComplete=RESET, rxNumComplete=RESET, rxStart=RESET, ackFlag=SET;

ISR(USART_RXC_vect)
{
	ch= UDR;

	if((ch=='S') && (rxStart==RESET))								//for string
	{
		rxStart= SET;
		arrIndex= strIndex;
		rxBuffer= rxStrBuffer;
		rxBufferSize= strSize;
		rxComplete= &rxStrComplete;

		for(unsigned char i=0; i<strSize; i++)
		rxBuffer[i]= NULL_VALUE;
	}
	
	else if((ch=='N') && (rxStart==RESET))						//for number
	{
		rxStart= SET;
		arrIndex= numIndex;
		rxBuffer= rxNumBuffer;
		rxBufferSize= numSize;
		rxComplete= &rxNumComplete;

		for(unsigned char i=0; i<numSize; i++)
		rxBuffer[i]= NULL_VALUE;
	}

	else if((ch=='A') && (rxStart==RESET))						//for ACK
	{
		rxStart= SET;
		arrIndex= ackIndex;
		rxBuffer= rxAckBuffer;
		rxBufferSize= ackSize;
	}

	else if(rxBuffer != NULL_VALUE)
	{
		if((arrIndex<(rxBufferSize-1)) && (ch!=NULL_VALUE))
		rxBuffer[arrIndex++]= ch;
		
		else if(ch == NULL_VALUE)
		{
			rxStart= RESET;
			rxBuffer= NULL_VALUE;
			*rxComplete= SET;
		}
	}
}

void uartInit(unsigned long baudRate, char* recStrBuffer, unsigned short int recStrBufferSize, char* recNumBuffer, unsigned short int recNumBufferSize)
{
	rxStrBuffer= recStrBuffer;
	rxNumBuffer= recNumBuffer;

	strSize= recStrBufferSize;
	numSize= recNumBufferSize;

	UART_DDR|= (1<<TX);
	UART_DDR&= ~(1<<RX);

	UCSRB= 0x98;
	UCSRC= 0xA6;
	UBRRL= BAUD_PRESCALE(baudRate);
	UBRRH= ((BAUD_PRESCALE(baudRate))>>8);
	
	sei();
}

void isOk(void)
{
	char ackArr[4]="Ack", counter=0;

	for(unsigned char i=RESET; i<3; i++)
	if(rxAckBuffer[i] == ackArr[i])
	counter++;
	
	if(counter == 3)
	{
		ackFlag= SET;
		for(unsigned char i=RESET; i<ackSize; i++)
		rxAckBuffer[i]= NULL_VALUE;
	}
}

void uartSendingCharacter (char txData)
{
	while((UCSRA&(1<<UDRE))==RESET);
	UDR= txData;
}

void uartSendingAck (void)
{
	char *data= "Ack";
	unsigned short int i= RESET;

	uartSendingCharacter('A');

	while(data[i] != NULL_VALUE)
	uartSendingCharacter(data[i++]);

	uartSendingCharacter(NULL_VALUE);
}

BOOL uartSendingString (char *data)
{
	if(ackFlag == RESET)
	isOk();

	if(ackFlag == SET)
	{
		unsigned short int i= RESET;

		uartSendingCharacter('S');

		while(data[i] != NULL_VALUE)
		uartSendingCharacter(data[i++]);

		uartSendingCharacter(NULL_VALUE);
		ackFlag= RESET;
		
		return TRUE;
	}
	
	return FALSE;
}

char uartReceivingCharacter(void)
{
	return rxStrBuffer[0];
}

volatile char* uartReceivingString(void)
{
	if(rxStrComplete == RESET)
	return NULL_VALUE;

	else
	{
		strIndex= RESET;
		uartSendingAck ();
		rxStrComplete= RESET;
		return rxStrBuffer;
	}
}

unsigned int uartSendingNumber(unsigned int num)
{
	unsigned int temp;
	
	if(ackFlag == RESET)
	isOk();

	if(ackFlag == SET)
	{
		uartSendingCharacter('N');

		while(num > 0)
		{
			temp= num % 10;
			num/= 10;
			uartSendingCharacter(temp + 48);
		}

		uartSendingCharacter(NULL_VALUE);
		ackFlag= RESET;
		
		return TRUE;
	}
	
	return FALSE;
}

static unsigned int uartValueMirrorNum(unsigned int* mirrorNum)
{
	unsigned int count= 0;
	unsigned long int temp = 0;
	while (*mirrorNum > 0)
	{
		if ((temp == 0) && ((*mirrorNum % 10) == 0))
		count++;

		temp *= 10;
		temp += (*mirrorNum % 10);
		*mirrorNum /= 10;
	}
	*mirrorNum = temp;

	return count;
}

unsigned int uartReceivingNumber(void)
{
	if(rxNumComplete == RESET)
	return FALSE;
	
	else
	{
		unsigned int num=RESET, i=RESET, counter=RESET;
		
		numIndex= RESET;
		uartSendingAck();
		rxNumComplete= RESET;

		while(rxNumBuffer[i] != NULL_VALUE)
		{
			num*= 10;
			num+= rxNumBuffer[i++] - 48;
		}
		
		counter= uartValueMirrorNum(&num);
		
		if (counter > RESET)
		for (unsigned short int j=RESET; j<counter; j++)
		num*= 10;
		
		return num;
	}
}