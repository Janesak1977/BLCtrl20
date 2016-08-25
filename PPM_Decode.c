/*############################################################################
 + Regler für Brushless-Motoren
 + ATMEGA168 mit 8MHz
 + (c) 01.2007 Holger Buss
 + Nur für den privaten Gebrauch / NON-COMMERCIAL USE ONLY
 + Keine Garantie auf Fehlerfreiheit
 + Kommerzielle Nutzung nur mit meiner Zustimmung
 + Der Code ist für die Hardware BL_Ctrl V2.0 entwickelt worden
 + www.mikrocontroller.com
############################################################################*/
#include "main.h"

volatile unsigned char unk_100165 = 0;
volatile unsigned int PPMsignal = 0;
volatile unsigned char Timer1Overflow = 0;
volatile unsigned char PPM_Timeout = 0, anz_ppm_werte = 0;   // Ungültig, wenn Null

int OCR1Atemp;


//############################################################################
// Timer 1 Initializatin
//############################################################################
void InitPPM(void)
{
	TCCR1B |= (_BV(ICNC1) | _BV(ICES1));
	ICP_POS_FLANKE;
	ICP_INT_ENABLE;
	TIMER1_INT_ENABLE;
	TCCR2B = 0x04;			//Timer2 Clk/64
}



//############################################################################
// Timer 1 Overflow Interrupt Serivce Routine
//############################################################################
SIGNAL(TIMER1_OVF_vect)						// evry 64us  
{
	switch (unk_100165++) 
	{
		case 0:
			OCR1A = OCR1Atemp;
		break;
		
		case 1:		
			if (unk_10012F<2)
				OCR1A = OCR1Atemp;
			else
				OCR1A = OCR1Atemp + 1;			
		break;
		
		case 2:
			if (unk_10012F==3)
				OCR1A = OCR1Atemp + 1;
			else
				OCR1A = OCR1Atemp;
		break;
		
		case 3:
			if (unk_10012F==0)
				OCR1A = OCR1Atemp;
			else
				OCR1A = OCR1Atemp + 1;
			unk_100165 = 0;
		break;
	}		
	Timer1Overflow++;
}


//############################################################################
// Timer 1 Input Capture Interrupt Serivce Ruotine
//############################################################################
SIGNAL(TIMER1_CAPT_vect)				//one step is 125ns 
{
	static unsigned int tim_alt;
	static unsigned int ppm;
	unsigned int tmp, tmp1;
	
	if (TCCR1B & _BV(ICES1))		//if rising edge
	{
		unk_10016A = Timer1Overflow;
		tim_alt = ICR1;
		ICP_NEG_FLANKE;
		PPM_Timeout = 100;
		TCNT2 = 0;
	}
	else			// else falling edge
	{
		ICP_POS_FLANKE;
		tmp1 = (int)(Timer1Overflow-unk_10016A) * 512 + ICR1;
		ppm =  tmp1 - tim_alt;
		tmp = TCNT2 * 64;				// TCNT2Step(8us) / TCNT1Step(0.125us) = 64
		if ((tmp+300)<ppm)
		{
			ppm -= 512;
			if ((tmp+300)<ppm)
			{
				ppm -= 512;
				if ((tmp+300)<ppm) 
				ppm = tmp;
			}			
		}			
		else
		if ((tmp-300)>ppm)
		{
			ppm += 512;
			if ((tmp-300)>ppm)
			{
				ppm += 512;
				if ((tmp-300)>ppm) 
				ppm = tmp;
			}
		}

		ppm = ppm / 3;
		if (anz_ppm_werte!=255)
			if ((ppm-2701)<499)
				anz_ppm_werte++;
		if (ppm<3100)
			ppm = 3100;
			
		tmp = (ppm-3100);
		tmp1 = (3*PPMsignal + tmp) / 4;
		ppm = tmp1;
		PPMsignal = tmp1;							
	}
}	

