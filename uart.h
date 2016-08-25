#ifndef _UART_H
 #define _UART_H
 
#include "main.h"

void uart_putchar (uint8_t);
void USART0_TransmitTxData(void);
void USART0_ProcessRxData(void);
void UART_Init (void);
void SendString(unsigned char*);

extern int Debug_Timer;
extern unsigned char Revision;
extern unsigned int CrcOkay;
extern unsigned char SIO_setpoint;


typedef struct
{
	unsigned char Digital[2];
	unsigned int Analog[32];    // Debugvalues
} __attribute__((packed)) DebugOut_t;

extern DebugOut_t DebugOut;


typedef struct
{
	unsigned char SWMajor;
	unsigned char SWMinor;
	unsigned char ProtoMajor;
	unsigned char ProtoMinor;
	unsigned char SWPatch;
	unsigned char Reserved[5];
}  __attribute__((packed)) UART_VersionInfo_t;

#define RxD		PD0
#define TxD		PD1

#endif //_UART_H
