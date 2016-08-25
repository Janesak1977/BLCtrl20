/*****************************************************************************
 
*****************************************************************************/
#include "main.h"

volatile unsigned int CountMilliseconds = 0;
unsigned char TmrOvfCnt;
unsigned int ConsumptionTimer = 255;

enum {
	STOP             = 0,
	CK               = 1,
	CK8              = 2,
	CK64             = 3,
	CK256            = 4,
	CK1024           = 5,
	T0_FALLING_EDGE  = 6,
	T0_RISING_EDGE   = 7
};



SIGNAL(TIMER0_OVF_vect)
{
	static unsigned char cnt;
	
	if (TmrOvfCnt!=0) --TmrOvfCnt;
	if(!cnt--)
	{
		cnt = 3;
		CountMilliseconds += 1;
		if(I2C_Timeout) 
		{
			I2C_Timeout--;
			if ( (I2C_Timeout==440) || (I2C_Timeout==300) || (I2C_Timeout==100))
			{
				Reset_I2C(0x50);
				if (I2C_Timeout==440) DebugOut.Analog[15]++;		//Increment debug I2CLost
			}
		}			
		if(PPM_Timeout) PPM_Timeout--; else anz_ppm_werte = 0;
		if(SIO_Timeout) SIO_Timeout--;
		if(ADC_Timeout) ADC_Timeout--;
		
		ConsumptionTimer--;
		if (ConsumptionTimer == 0xFFFF)
		{
			if (ConsumptedCurrent<65000)
				ConsumptedCurrent += Mittelstrom;
			ConsumptionTimer = 1400;
			if (I2C_Timeout==0) Reset_I2C(0x50);
		}
	}
}


unsigned int SetDelay(unsigned int t)
{
  return((t + CountMilliseconds)-1);                                             
}

unsigned char CheckDelay (unsigned int t)
{
	return(((t - CountMilliseconds) & 0x8000) >> 8);
}

void Delay_ms(unsigned int w)
{
	unsigned int akt;
	akt = SetDelay(w);
	while (!CheckDelay(akt));
}


void Timer0_Init(void)
{
	TIMSK0 = 0x01;		//TOIE0=1;
	TCCR0B = 0x02;		// Timer0, CLK/8,Mode NORMAL
	TIFR0 = 0x07;		//Clear flags OCF0B,OCF0A,TOV0;
	TCCR0B  = TIMER_TEILER;
	TIM0_START;
	//TIMER2_INT_ENABLE;
}

