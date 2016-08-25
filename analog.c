/*############################################################################

############################################################################*/

#include "main.h"

unsigned char ADC_Timeout;
unsigned char ADC_UBat_Timer = 0;



//############################################################################
// Timer 2 Overflow Interrupt Serivce Routine
//############################################################################
SIGNAL(TIMER2_OVF_vect)
{


}


//############################################################################
//Init ADC
//############################################################################
void ADC_Init(void)
{
	ADCSRA = 0xA6;
	ADMUX = 0xC7;		     // Kanal 7, IntREF
	ADCSRA |= _BV(ADSC);
	ADCSRB |= _BV(ACME);
}


//############################################################################
// Analogwerte lesen
//############################################################################
void AdConvert(void)
{
	unsigned char sense;
	unsigned tmp;
	
	ADC_Timeout--;
	if (ADC_Timeout==0xFF)
	{
		ADC_Timeout = 100;
		ADC_UBat_Timer--;
		if (ADC_UBat_Timer==0xFF)
		{
			sense = ADMUX;
			ADMUX  =  0xC7;		//Channel 7 (UBat input select)
			ACSR |= _BV(ACD);
			ADCSRA =  0xD3;
			ADCSRA |=  0x10;		
			ADMUX  =  0x0C7;
			ADCSRA |= _BV(ADSC);	// Start
			while (((ADCSRA & _BV(ADIF)) == 0));
			ADMUX = sense;
			tmp = (ADCW * 19) / 64;
			if (tmp>255) tmp = 255;
			UBAT = (UBAT + tmp) / 2;		//UBat calculations
			ADCSRA = 0x00; 
			ADCSRB |= _BV(ACME);
			ACSR &= ~(_BV(ACD));			// Analog Comperator ein
			ADC_UBat_Timer = 3;			
			return;
		}
		else
		{
			sense = ADMUX;
			ADMUX  =  0xC3;		//Channel 3 (Temperature input select)
			ACSR |= _BV(ACD);
			ADCSRA =  0xD3;
			ADCSRA |=  0x10;
			ADMUX  =  0x0C7;
			ADCSRA |= _BV(ADSC);	// Start
			while (((ADCSRA & _BV(ADIF)) == 0));
			ADMUX = sense;
			tmp = ADCW * 10;
			Temperature = (Temperature + tmp) / 2;
			ADCSRA = 0x00;
			ADCSRB |= _BV(ACME);
			ACSR &= ~(_BV(ACD));			// Analog Comperator ein
			return;	
		}
	}
	
	sense = ADMUX;
	ADMUX  =  0xC6;		//Channel 6 (Current input select)
	ACSR |= _BV(ACD);
	ADCSRA =  0xD3;
	ADCSRA |=  0x10;
	ADMUX  =  0x0C6;
	ADCSRA |= _BV(ADSC);	// Start
	while (((ADCSRA & _BV(ADIF)) == 0));
	ADMUX = sense;
	tmp = (ADCW * ProgConst1) / 4;
	StromADC = StromADC + tmp;
	MittelStromADcount++;
	Strom = (Strom + StromADC) / 2;
	if (Strom_max < Strom) Strom_max = Strom;
	ADCSRA = 0x00;
	ADCSRB |= _BV(ACME);
	ACSR &= ~(_BV(ACD));			// Analog Comperator ein	
}


//############################################################################
//Strom Analogwerte lesen
//############################################################################
unsigned int MessAD(unsigned char channel)
{
	unsigned char sense;
	
	sense = ADMUX;				// Sense-Kanal merken
	ADMUX  =  0x0C0 | channel;   // Kanal 6
	ACSR |= _BV(ACD);  
	ADCSRA =  0xD3;  
	ADCSRA |=  0x10;
	ADMUX  =  0x0C0 | channel;
	ADCSRA |= _BV(ADSC);				// Start
	while (((ADCSRA & _BV(ADIF)) == 0));
	ADMUX = sense;				// zurück auf den Sense-Kanal
	ADCSRA = 0x00;
	ACSR &= ~(_BV(ACD));			// Analog Comperator ein
	return(ADCW);
}	


//############################################################################
//Strom Analogwerte lesen
//############################################################################
void FastADConvert(void)
{
	Strom = (MessAD(6) * ProgConst1) / 16;
	if (Strom_max < Strom) Strom_max = Strom;
}
