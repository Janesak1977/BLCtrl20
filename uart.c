// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// + Regler für Brushless-Motoren
// + ATMEGA168 mit 8MHz
// + (c) 01.2007 Holger Buss
// + Nur für den privaten Gebrauch / NON-COMMERCIAL USE ONLY
// + Keine Garantie auf Fehlerfreiheit
// + Kommerzielle Nutzung nur mit meiner Zustimmung
// + Der Code ist für die Hardware BL_Ctrl V2.0 entwickelt worden
// + www.mikrocontroller.com
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#include <stdarg.h>
#include "main.h"

#define MAX_SENDE_BUFF   110
#define RXD_BUFFER_LEN  50

#define FALSE	0
#define TRUE	1

unsigned volatile char txd_complete = TRUE;
unsigned int DebugData_Interval = 5000;

unsigned volatile char rxd_buffer_locked = FALSE;
unsigned volatile char ReceivedBytes = 0;
unsigned volatile char *pRxData = 0;
unsigned volatile char RxDataLen = 0;
unsigned char Dbg_Request= 0;
unsigned char Request_DebugLabel = 0;
unsigned char unk_10014F = 0;
unsigned char SIO_setpoint = 0;
unsigned int CrcOkay = 0;

unsigned int DebugData_Timer;
unsigned volatile char txd_buffer[MAX_SENDE_BUFF];
DebugOut_t DebugOut;
UART_VersionInfo_t	UART_VersionInfo;
unsigned volatile char rxd_buffer[RXD_BUFFER_LEN];




unsigned int SIO_Timeout;



DebugOut_t DebugOut;

// keep lables in flash to save 512 bytes of sram space
const unsigned char ANALOG_LABEL[32][16] PROGMEM =
{
   //1234567890123456
	"Current   [0.1A]", //0
	"CurrentRaw      ",
	"mAh      [10mAh]",
	"Temp.    [0.1-C]",
	"Voltage   [0.1V]",
	"rpm (raw)       ", //5
	"Setpoint        ",
	"MaxSetpoint     ",
	"PPMSetpoint     ",
	"Param.PwmScaling",
	"Param.MaxCurrent", //10
	"Param.MaxTemp.  ",
	"Param.Scale I   ",
	"Param.Bits      ",
	"Param.StartPWM  ",
	"I2C Lost        ", //15
	"Debug16         ",
	"Debug17         ",
	"Debug18         ",
	"Debug19         ",
	"Debug20         ", //20
	"Debug21         ",
	"Debug22         ",
	"Debug23         ",
	"Debug24         ",
	"Debug25         ", //25
	"Debug26         ",
	"Debug27         ",
	"Debug28         ",
	"Debug29         ",
	"Hardware        ", //30
	"Limit I         "
};


//############################################################################
//Routine für die Serielle Ausgabe
//############################################################################
void uart_putchar (uint8_t c)
{

	if (UCSR0B & _BV(TXEN0))
	{
		//Warten solange bis Zeichen gesendet wurde
		loop_until_bit_is_set(UCSR0A, UDRE0);
		//Ausgabe des Zeichens
		UDR0 = c;
	}
}


//############################################################################
//UART Initialization
//############################################################################
void UART_Init (void)
{

	UCSR0B &= ~(_BV(RXCIE0));	// Disable RX complete INT
	UCSR0B &= ~(_BV(TXCIE0));	// Disable TX complete INT
	DDRD &= ~(_BV(RxD));			// RxD pin Input
	PORTD &= ~(_BV(RxD));
	DDRD |= _BV(TxD);				// TxD pin Output
	PORTD &= ~(_BV(TxD));			// PORTD.TxD <- 0
	UCSR0A |= _BV(U2X0);		// UART double speed
	UCSR0C &= ~(_BV(UMSEL01));
	UCSR0C &= ~(_BV(UMSEL00));	// Mode Asynchronous USART
	UCSR0C &= ~(_BV(UPM01));
	UCSR0C &= ~(_BV(UPM00));	// Parity disabled
	UCSR0C &= ~(_BV(USBS0));	// 1 STOP bit
	UCSR0B &= ~(_BV(UCSZ02));
	UCSR0C |= _BV(UCSZ01);
	UCSR0C |= _BV(UCSZ00);
	UCSR0B |= _BV(RXEN0);		//
	UCSR0B |= _BV(TXEN0);
	while(UCSR0A & _BV(RXC0)) 
	{
		unsigned char i = UDR0;
	}
	UCSR0B |= _BV(RXCIE0);
	UCSR0B &= ~(_BV(TXCIE0));
	DebugData_Timer = SetDelay(DebugData_Interval);
	
	rxd_buffer_locked = 0;
	pRxData = 0;
	RxDataLen = 0;
	txd_complete = 1;
	
	UART_VersionInfo.SWMajor = VERSION_MAJOR;
	UART_VersionInfo.SWMinor = VERSION_MINOR;
	UART_VersionInfo.SWPatch = VERSION_PATCH;
	UART_VersionInfo.ProtoMajor = VERSION_SERIAL_MAJOR;
	UART_VersionInfo.ProtoMinor = VERSION_SERIAL_MINOR;
	
	uart_putchar ('\r');
	uart_putchar ('\n');
	uart_putchar ('B');
	uart_putchar ('L');
	uart_putchar (':');
	uart_putchar ('V');
	uart_putchar (0x30 + VERSION_MAJOR);
	uart_putchar ('.');
	uart_putchar (0x30 + VERSION_MINOR/10);
	uart_putchar (0x30 + VERSION_MINOR%10);
	uart_putchar ('a');
	uart_putchar ('\r');
	uart_putchar ('\n');
	uart_putchar('H');
	uart_putchar('W');
	uart_putchar (':');
	uart_putchar('2');
	uart_putchar('.');
	uart_putchar('0');
	uart_putchar(' ');
	uart_putchar('R');
	uart_putchar('e');
	uart_putchar('v');
	uart_putchar (0x30 + Revision/10);
	uart_putchar (0x30 + Revision%10);
}


void SendUart(void)
{
	static unsigned int ptr = 0;
	unsigned char tmp_tx;
	
	if(!(UCSR0A & _BV(TXC0))) return;
	if(!txd_complete)
	{
		ptr++;                    // die [0] wurde schon gesendet
		tmp_tx = txd_buffer[ptr];
		if((tmp_tx == '\r') || (ptr == MAX_SENDE_BUFF))
		{
			ptr = 0;
			txd_complete = 1;
		}
		UCSR0A |= _BV(TXC0);
		UDR0 = tmp_tx;
	}
	else ptr = 0;
}



//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//++ Empfangs-Part der Datenübertragung
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
SIGNAL(USART_RX_vect)
{
	static uint16_t crc;
	static uint8_t ptr_rxd_buffer = 0;
	uint8_t crc1, crc2;
	uint8_t c;
	
	c = UDR0;  // catch the received byte

	if(rxd_buffer_locked) return;			// if rxd buffer is locked immediately return
	
	// the rxd buffer is unlocked
	if((ptr_rxd_buffer == 0) && (c == '#')) // if rxd buffer is empty and syncronisation character is received
	{
		rxd_buffer[ptr_rxd_buffer++] = c;	// copy 1st byte to buffer
		crc = c;							// init crc
	}
	
	#if 0
	else if (ptr_rxd_buffer == 1) // handle address
	{
		rxd_buffer[ptr_rxd_buffer++] = c; // copy byte to rxd buffer
		crc += c; // update crc
	}
	#endif
	
	else if (ptr_rxd_buffer < RXD_BUFFER_LEN) // collect incomming bytes
	{
		if(c != '\r') // no termination character
		{
			rxd_buffer[ptr_rxd_buffer++] = c; // copy byte to rxd buffer
			crc += c; // update crc
		}
		else // termination character was received
		{
			// the last 2 bytes are no subject for checksum calculation
			// they are the checksum itself
			crc -= rxd_buffer[ptr_rxd_buffer-2];
			crc -= rxd_buffer[ptr_rxd_buffer-1];
			// calculate checksum from transmitted data
			crc %= 4096;
			crc1 = '=' + crc / 64;
			crc2 = '=' + crc % 64;
			// compare checksum to transmitted checksum bytes
			if((crc1 == rxd_buffer[ptr_rxd_buffer-2]) && (crc2 == rxd_buffer[ptr_rxd_buffer-1]))
			{   // checksum valid
					CrcOkay = 2000;
					rxd_buffer[ptr_rxd_buffer] = '\r'; // set termination character
					ReceivedBytes = ptr_rxd_buffer + 1;// store number of received bytes
					rxd_buffer_locked = TRUE;          // lock the rxd buffer
					// if 2nd byte is an 'R' enable watchdog that will result in an reset
					if(rxd_buffer[2] == 'R') {wdt_enable(WDTO_250MS);} // Reset-Commando
			}
			else
			{	// checksum invalid
				rxd_buffer_locked = FALSE; // unlock rxd buffer
			}
			ptr_rxd_buffer = 0; // reset rxd buffer pointer
		}
	}
	else // rxd buffer overrun
	{
		ptr_rxd_buffer = 0; // reset rxd buffer
		rxd_buffer_locked = FALSE; // unlock rxd buffer
	}

}


// --------------------------------------------------------------------------
void AddCRC(uint16_t datalen)
{
	uint16_t tmpCRC = 0, i;
	for(i = 0; i < datalen; i++)
	{
		tmpCRC += txd_buffer[i];
	}
	tmpCRC %= 4096;
	txd_buffer[i++] = '=' + tmpCRC / 64;
	txd_buffer[i++] = '=' + tmpCRC % 64;
	txd_buffer[i++] = '\r';
	txd_complete = FALSE;
	UDR0 = txd_buffer[0]; // initiates the transmittion (continued in the TXD ISR)
}


// --------------------------------------------------------------------------
void SendOutData(uint8_t cmd, uint8_t addr, uint8_t numofbuffers, ...) // uint8_t *pdata, uint8_t len, ...
{
	va_list ap;
	uint16_t pt = 0;
	uint8_t a,b,c;
	uint8_t ptr = 0;

	uint8_t *pdata = 0;
	int len = 0;

	txd_buffer[pt++] = '#';			// Start character
	txd_buffer[pt++] = 'a' + addr;	// Address (a=0; b=1,...)
	txd_buffer[pt++] = cmd;			// Command

	va_start(ap, numofbuffers);
	if(numofbuffers)
	{
		pdata = va_arg(ap, uint8_t*);
		len = va_arg(ap, int);
		ptr = 0;
		numofbuffers--;
	}

	while(len)
	{
		if(len)
		{
			a = pdata[ptr++];
			len--;
			if((!len) && numofbuffers)
			{
				pdata = va_arg(ap, uint8_t*);
				len = va_arg(ap, int);
				ptr = 0;
				numofbuffers--;
			}
		}
		else a = 0;
		if(len)
		{
			b = pdata[ptr++];
			len--;
			if((!len) && numofbuffers)
			{
				pdata = va_arg(ap, uint8_t*);
				len = va_arg(ap, int);
				ptr = 0;
				numofbuffers--;
			}
		}
		else b = 0;
		if(len)
		{
			c = pdata[ptr++];
			len--;
			if((!len) && numofbuffers)
			{
				pdata = va_arg(ap, uint8_t*);
				len = va_arg(ap, int);
				ptr = 0;
				numofbuffers--;
			}
		}
		else c = 0;
		txd_buffer[pt++] = '=' + (a >> 2);
		txd_buffer[pt++] = '=' + (((a & 0x03) << 4) | ((b & 0xf0) >> 4));
		txd_buffer[pt++] = '=' + (((b & 0x0f) << 2) | ((c & 0xc0) >> 6));
		txd_buffer[pt++] = '=' + ( c & 0x3f);
	}
	va_end(ap);
	AddCRC(pt); // add checksum after data block and initates the transmission
}


// --------------------------------------------------------------------------
void Decode64(void)
{
	uint8_t a,b,c,d;
	uint8_t x,y,z;
	uint8_t ptrIn = 3;
	uint8_t ptrOut = 3;
	uint8_t len = ReceivedBytes - 6;

	while(len)
	{
		a = rxd_buffer[ptrIn++] - '=';
		b = rxd_buffer[ptrIn++] - '=';
		c = rxd_buffer[ptrIn++] - '=';
		d = rxd_buffer[ptrIn++] - '=';
		//if(ptrIn > ReceivedBytes - 3) break;

		x = (a << 2) | (b >> 4);
		y = ((b & 0x0f) << 4) | (c >> 2);
		z = ((c & 0x03) << 6) | d;

		if(len--) rxd_buffer[ptrOut++] = x; else break;
		if(len--) rxd_buffer[ptrOut++] = y; else break;
		if(len--) rxd_buffer[ptrOut++] = z; else break;
	}
	pRxData = &rxd_buffer[3];
	RxDataLen = ptrOut - 3;
}




void USART0_ProcessRxData(void)
{
	// if data in the rxd buffer are not locked immediately return
	if(!rxd_buffer_locked) return;
	
	Decode64(); // decode data block in rxd_buffer
	switch(rxd_buffer[2])
	{
		case 'u':// 
		if (pRxData[0]!=MotorAddress)
			if (pRxData[0]==0)
				unk_10014F = 0;
			else
				unk_10014F = 2;
		else
				unk_10014F = 0;

		Dbg_Request |= 0x08;		//bit 3
		break;

		case 'w':// Write Controller Configuration
			if (pRxData[0]==0)
				unk_10014F = SetupConfigBytes((unsigned char*)(pRxData+1));
			else
			{
				if (pRxData[0]==MotorAddress)
					unk_10014F = SetupConfigBytes((unsigned char*)(pRxData+1));
				else
					unk_10014F = 2;
			}
			Dbg_Request |= 0x10;	// bit 4					
		break;

		case 't'://Test Controllers
			SIO_setpoint = pRxData[MotorAddress-1];
			SIO_Timeout = 500;
		break;

		case 'd': // request for the debug data
			DebugData_Interval = (uint16_t) pRxData[0] * 10;
			if(DebugData_Interval > 0) Dbg_Request |= 2;
		break;

		case 'v': // request for version and board release
			Dbg_Request |= 1;		// bit 0
		break;

		case 'a':// request for labels of the analog debug outputs
			Request_DebugLabel = pRxData[0];
			Dbg_Request |= 4;		//bit 2
		break;

		default:
		//unsupported command received
		break;
	}
	pRxData = 0;
	RxDataLen = 0;
	rxd_buffer_locked = FALSE;
}


//---------------------------------------------------------------------------------------------
void USART0_TransmitTxData(void)
{
	unsigned char tmp;

	if(!txd_complete) 
	{
		SendUart();
		return;
	}		
	
	if (Dbg_Request & _BV(2))
	{
		unsigned char label[16];
		memcpy_P(label, ANALOG_LABEL[Request_DebugLabel], 16);		// read lable from flash to sram buffer
		SendOutData('A', 5, 2, (uint8_t *) &Request_DebugLabel, sizeof(Request_DebugLabel), label, 16);
		Request_DebugLabel = 0xFF;
		Dbg_Request &= ~(_BV(2));
	}
	else if( ((DebugData_Interval > 0) && CheckDelay(DebugData_Timer)) || (Dbg_Request & _BV(1)))
	{
		PrepareDebugData();
		SendOutData('D', 5, 1,(uint8_t *) &DebugOut, sizeof(DebugOut));
		DebugData_Timer = SetDelay(DebugData_Interval);
		Dbg_Request &= ~(_BV(1));
	}
	else if (Dbg_Request & _BV(0))
	{
		SendOutData('V', 5, 1,(uint8_t *) &UART_VersionInfo, sizeof(UART_VersionInfo));
		Dbg_Request &= ~(_BV(0));
	}
	else if (Dbg_Request & _BV(3))
	{	
		if (unk_10014F==0) tmp=1;
			else tmp=0;
		SendOutData('U', 5, 4, (uint8_t *) &tmp , sizeof(tmp), (uint8_t *) &unk_10014F ,sizeof(unk_10014F) ,(uint8_t *) &MotorAddress ,sizeof(MotorAddress) ,(uint8_t *) &BLConfig , sizeof(BLConfig));
		Dbg_Request &= ~(_BV(3));
	}
	else if (Dbg_Request & _BV(4))
	{
		if (unk_10014F==0) tmp=1;
		else tmp=0;
		SendOutData('W', 5, 2, (uint8_t *) &tmp , sizeof(tmp), (uint8_t *) &unk_10014F ,sizeof(unk_10014F));
		Dbg_Request &= ~(_BV(4));
	}			
}


void uart_sendbuffer(unsigned char* buff)
{
	unsigned char ptr;

	ptr = 0;
	if (buff[0]!=0)
		do
			uart_putchar(buff[ptr++]);
		while (buff[ptr]!=0);
}


