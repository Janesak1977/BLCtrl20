/*############################################################################
Slaveadr = 0x52 = Front, 0x54 = Back, 0x56 = Right, 0x58 = Left
############################################################################*/
#include <avr/io.h>
#include <util/twi.h>
#include "main.h"

unsigned char I2CRxBuffer[8];

unsigned char Byte_Counter = 0;
unsigned char twi_state = 0;
unsigned char unk_100173 = 0;
unsigned int I2C_Setpoint = 0;
unsigned int unk_10016F = 0;
unsigned char pI2CRxBuf = 0;
unsigned int unk_10016C = 0;
unsigned char unk_10016B = 0;
volatile unsigned char unk_10016A = 0;


//############################################################################
//I2C (TWI) Interface Init
//############################################################################
void InitIC2_Slave(uint8_t adr)
{
	TWAR = adr + (2*MotorAddress); // Eigene Adresse setzen
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
}

void Reset_I2C(uint8_t adr)
{
	TWCR = 0x00;
	TWAR = adr + (2*MotorAddress); // Eigene Adresse setzen
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWIE) | (1<<TWEA);
}


ISR (TWI_vect)
{
	unsigned int tmp;
	unsigned char tmpTWDR;
	
	switch (TWSR & 0xF8)
	{
		case SR_SLA_ACK: 					//Own SLA+W has been received, ACK has been returned
			TWCR |= _BV(TWINT);
			Byte_Counter=0;
			twi_state = 0;
			return;
			
		// Receiving Data
		case SR_PREV_ACK:
			tmpTWDR = TWDR;					// Previously addressed with own SLA+W, data has been received, ACK has been returned
			TWCR |= _BV(TWINT);
			switch(twi_state++)
			{
				case 0:
					if ((tmpTWDR==0) && (unk_100173!=0) && (I2C_Timeout!=0))
						I2C_Setpoint = 1;
					else
						if ((I2C_Timeout!=0) && (unk_100173!=0) )
							I2C_Setpoint = tmpTWDR << 3;							
						else
							I2C_Setpoint = 0;
					//Upper bits of 11 bit setpoint
					unk_100173 = tmpTWDR;
					unk_10016C += Mittelstrom;
					unk_10016B++;
					I2C_Timeout = 500;
					return;
				
				case 1:
					if (I2C_Setpoint!=0)
						I2C_Setpoint += tmpTWDR & 0x07;
					Byte_Counter = tmpTWDR >> 3;
					I2C_Timeout = 500;
					return;
					
				case 2:
					if (pI2CRxBuf<8)
					{
						I2CRxBuffer[pI2CRxBuf++] = tmpTWDR;
						twi_state = 2;
					}
					I2C_Timeout = 500;
					return;	
			}
			twi_state = 254;
			I2C_Timeout = 500;
			return;	
			
		// Sending Data
		case SW_SLA_ACK:					// Own SLA+R has been received, ACK has been returned 
		case SW_DATA_ACK:			// Data byte in TWDR has been transmitted, ACK has been received
			if (Byte_Counter<31)
			{					
				switch(Byte_Counter++)
				{
					case 0:
						if ((unsigned int)Mittelstrom>254)
							TWDR = 254;
						else
							TWDR = Mittelstrom;
		
						TWCR |= _BV(TWINT);
						return;

					case 10:
						if ((unsigned int)Mittelstrom<=510)
							TWDR = (unsigned int)Mittelstrom / 2;	
						else
							TWDR = 254;

						TWCR |= _BV(TWINT);
						return;
					
					case 1:
					case 11:
						if (I2C_Setpoint==0)
							TWDR = 250;
						else if (unk_100134<50)
								TWDR = 40;
							else if (Limit_I > MaxSetpoint)		
									TWDR = MaxSetpoint;
								else
									TWDR = Limit_I;
				
						TWCR |= _BV(TWINT);
						return;
						
					case 2:
					case 12:
						TWDR = Temperatur;
					
						TWCR |= _BV(TWINT);
						return;
						
					case 3:
					case 13:
						TWDR = rpm_raw;
					
						TWCR |= _BV(TWINT);
						return;

					case 14:
						TWDR = (ConsumptedCurrent>>8) /2;
					
						TWCR |= _BV(TWINT);
						return;
					
					case 4:
						TWDR = ConsumptedCurrent>>8;
					
						TWCR |= _BV(TWINT);
						return;
						
					case 5:
						TWDR = (unsigned char)UBAT;
						
						TWCR |= _BV(TWINT);
						return;
						
					case 6:
						TWDR = (unsigned char)unk_10016F;
						
						TWCR |= _BV(TWINT);
						return;

					case 8:
						TWDR = 31;				//Minor Version?
						
						TWCR |= _BV(TWINT);
						return;

					case 9:
						if (unk_10016B<50)
							tmp = unk_10016C/unk_10016B;
						else
							tmp = Mittelstrom;
						if (tmp<254)
							TWDR = (unsigned char)tmp;				
						else
							TWDR = 254;	
						unk_10016C = 0;
						unk_10016B = 0;				
						
						TWCR |= _BV(TWINT);
						return;

					case 16:
						TWDR = BLConfig.Revision;
						
						TWCR |= _BV(TWINT);
						return;

					case 17:
						TWDR = BLConfig.SetMask;
						
						TWCR |= _BV(TWINT);
						return;
			
					case 18:
						TWDR = BLConfig.PwmScaling;
						
						TWCR |= _BV(TWINT);
						return;
					
					case 19:	
						TWDR = BLConfig.CurrentLimit;
						
						TWCR |= _BV(TWINT);
						return;
					
					case 20:
						TWDR = BLConfig.TempLimit;
						TWCR |= _BV(TWINT);
						return;
					
					case 21:
						TWDR = BLConfig.CurrentScaling;
						TWCR |= _BV(TWINT);
						return;
					
					case 22:
						TWDR = BLConfig.BitConfig;
						TWCR |= _BV(TWINT);
						return;

					case 23:
						TWDR = BLConfig.crc;
						TWCR |= _BV(TWINT);
						return;

					case 24:
						TWDR = NotStarted_anz;
						TWCR |= _BV(TWINT);
						return;

					case 7:
					case 15:
					case 27:
						TWDR = 0;				//Major Version?
						TWCR |= _BV(TWINT);
						return;

					case 30:
						TWDR = 0xA5;
						TWCR |= _BV(TWINT);
						return;						
					
					case 25:
					case 26:
					case 28:
					case 29:
						TWDR = 255;
						Byte_Counter = 254;
						TWCR |= _BV(TWINT);
						return;
						
				}

			}			
			else
			{
				TWDR = 255;
				Byte_Counter = 254;
				TWCR |= _BV(TWINT);
				return;	
			}
		
		
		case TWI_BUS_ERR_1:
		case TWI_BUS_ERR_2:
			TWCR = 0xD5;
			unk_10016F++;
			TWCR = 0xC5;
			return;
			
		default:
			TWCR = 0xC5;
			return;
	}
	
}	


void CheckI2CConfigBuf(void)
{
	if (pI2CRxBuf>=8)
	{
		SetupConfigBytes(I2CRxBuffer);	
		pI2CRxBuf = 0;
	}		
}
