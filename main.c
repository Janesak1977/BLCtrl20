#include "main.h"

unsigned int RuheStrom = 5;
unsigned int UBAT = 128;		//[0.1V] ??
         int Temperature = 0x201E;		//AD converted value
unsigned char Temperatur = 30;		//[degC]
unsigned char ProgConst1 = 25;		//[A] ???
unsigned int Limitstrom = 25;		//[A]
unsigned char CurrentLimit = 40;		//[A]
unsigned char ZeitFuerBerechnungen = 1;
unsigned char ZeitZumAdWandeln = 1;
unsigned char MotorStopped = 1;
unsigned int unk_100104 = 2043;
unsigned char MotorAddress = 1;
unsigned char PPM_Betrieb = 1;
unsigned char MaxSetpoint = 255;
unsigned char Limit_I = 255;

         int  PWM = 0;
unsigned int  Strom = 0;
unsigned int Strom_max = 0;
	     int Mittelstrom = 0;
unsigned int ConsumptedCurrent = 0;
unsigned int rpm_raw = 0;
unsigned char TmrOvfCnt = 0;
unsigned char unk_10013E = 0;
unsigned char MittelStromADcount = 0;
unsigned int StromADC = 0;
unsigned int I2C_Timeout = 0;
unsigned int SIO_Timeout = 0;
unsigned char ADC_Timeout = 0;
unsigned char MotorStartingUp = 0;
unsigned char unk_100134 = 0;
unsigned int  CntKommutierungen = 0;
unsigned int unk_100130 = 0;
unsigned char unk_10012F = 0;
         int OCR1Atemp = 0;
unsigned char Revision = 0;

unsigned char NotStarted_anz;
unsigned char FLASH_Config[16];


void UART_SendMotorAddr(void)
{
	unsigned char tmp,tmp1;

	uart_putchar('\n');
	uart_putchar('A');
	uart_putchar('D');
	uart_putchar('R');
	uart_putchar(':');
	tmp = MotorAddress;
	tmp1 = MotorAddress + '0';
	if (tmp1 > '9')		//MotorAddress > 9
	{
		uart_putchar('1');
		tmp1 = tmp + '&';
	}
	uart_putchar(tmp1);
	
	uart_putchar('\n');
	uart_putchar('\r');
}

void SetPWM(void)
{
	int tmp_pwm;
	
	tmp_pwm = PWM;
	if ((Strom>(BLConfig.CurrentLimit*10)))
	{
		RED_ON
		if (Strom > ((CurrentLimit*10)+50))
		{
			OCR1A = 0;
			DRIVER_OFF;
			tmp_pwm = 0;
		}
		else
			if (tmp_pwm<0) tmp_pwm = 0;
		
	}
	else
		if (tmp_pwm<0) tmp_pwm = 0;
	
	
	if (unk_100134!=0xFF)
	{
		if (tmp_pwm > ((unk_100134*8)+80))
			tmp_pwm = (unk_100134*8)+80;
	}
	
	if (tmp_pwm >= 2044)
		tmp_pwm = 2043;

	OCR1Atemp = tmp_pwm / 4; 		//Max value for OCR1A is 512, so we DIV 4
	unk_10012F = tmp_pwm % 4;       //and MODULO 4 for remaider
}


void sub211(void)
{
	int tmpTemp;
	int tmp,tmp1;
	
	if (Temperature<=8450)
	{
		if (Temperature>6640) 
		{
			tmpTemp = Temperature / 6;
			tmp = 1400;
		}								      //temp -0.8 to 29 deg
		else
		if (Temperature>4290) 
		{
			tmpTemp = Temperature / 8;      //temp 30 to 59 deg
			tmp = 1130;
		}
		else
		if (Temperature>2440)       //temp  59.5 to 90.4
		{
			tmpTemp = Temperature / 6;
			tmp = 1310;
		}
		else
		if (Temperature>1320)       //temp 90.7 to 118.7
		{
			tmpTemp = Temperature / 4;
			tmp = 1517;
		}
		else
		{
			tmpTemp = Temperature / 2;      //temp 90.7 to 118.7
			tmp = 1850;
		}
	}

	tmpTemp = tmp - tmpTemp;

	Temperatur = tmpTemp / 10;
	DebugOut.Analog[3] = tmpTemp;
	
	if (Temperatur<120)
	{
		tmp = (tmpTemp * (int)Limitstrom) / 256;
		tmp = tmp - ( ((int)Limitstrom * 10) / 7);
		tmp = (int)((unsigned int)Mittelstrom/8 + tmp) / 10;
		tmp1 = (int)Limitstrom / 3;  
		if(tmp>tmp1)
			ProgConst1 = Limitstrom - tmp1;
		else
			ProgConst1 = Limitstrom - tmp;
	}
	else
	ProgConst1 = Limitstrom;
	
	tmp = 250 - Mittelstrom;
	if (tmp > 200) tmp = 200;
	if (tmp<-50) tmp = -50;
	tmp = tmpTemp - ((BLConfig.TempLimit * 10) + tmp);
	if (tmp>0)
	{
		tmp1 = 255 - (tmp * 2);
		if (tmp1<2)
			tmp1 = 1;
		MaxSetpoint = tmp1;

		RED_ON;
	}
	else
	MaxSetpoint = 255;
	
	DebugOut.Analog[15] = Limitstrom;
}


void PrepareDebugData(void)
{
	DebugOut.Analog[0] = Mittelstrom;
	DebugOut.Analog[1] = Strom;
	DebugOut.Analog[2] = ConsumptedCurrent;
	DebugOut.Analog[4] = UBAT;
	DebugOut.Analog[5] = rpm_raw;
	DebugOut.Analog[6] = PWM;				//setpoint
	DebugOut.Analog[7] = MaxSetpoint;
	DebugOut.Analog[8] = PPMsignal;
	DebugOut.Analog[9] = BLConfig.PwmScaling;
	DebugOut.Analog[10] = BLConfig.CurrentLimit;
	DebugOut.Analog[11] = BLConfig.TempLimit;
	DebugOut.Analog[12] = BLConfig.CurrentScaling;
	DebugOut.Analog[13] = BLConfig.BitConfig;
	DebugOut.Analog[14] = (BLConfig.BitConfig & 0x0E) >> 1;
	DebugOut.Analog[31] = Limit_I;
}

unsigned int DetermineSetpoint(void)
{
	unsigned int setpoint;
	unsigned int tmp_setpoint;
	
	cli();
	tmp_setpoint = I2C_Setpoint;
	sei();
	if(!I2C_Timeout)   // if 0 then go
	{
		if(SIO_Timeout)  // there are valid SIO-Daten
		{
			setpoint =  ((unsigned int)SIO_setpoint * 8);
			PPM_Betrieb = 0;
			ICP_INT_DISABLE;
			RED_OFF;
		}
		else
		if(anz_ppm_werte > 20)  // there are valid PPM-Data
		{
			PPM_Betrieb = 1;
			tmp_setpoint = PPMsignal;
			if (tmp_setpoint>3000)
				tmp_setpoint = 0;
			
			if (tmp_setpoint>2043)
				tmp_setpoint = 2043;
			
			if	(tmp_setpoint>80)
				setpoint = tmp_setpoint;
			else
				if (unk_100134==0)
					setpoint = 0;
				else
				setpoint = tmp_setpoint;
			RED_OFF;
		}
		else
		{
			RED_ON;
			setpoint = 0;
		}
		
		I2C_Setpoint = 0;
	}
	else
	{
		setpoint = tmp_setpoint;
		
		if ((unk_100134<10) && (tmp_setpoint>=2040))
			setpoint = 0;
		
		RED_OFF;
		SIO_setpoint = 0;
		PPMsignal = 0;
		PPM_Betrieb = 0;
		ICP_INT_DISABLE;
	}
	
	
	if (BLConfig.PwmScaling!=255)
	{
		setpoint = ((long)setpoint * (long)BLConfig.PwmScaling) / 256;
	}
	
	if (MaxSetpoint!=255)
	{
		setpoint = ((long)setpoint * (long)MaxSetpoint) / 256;
		RED_ON;
	}
	
	if (Limit_I!=255)
	{
		setpoint = ((long)setpoint * (long)Limit_I) / 256;
		RED_ON;
	}
	
	if (setpoint!=0)
	setpoint += 56;
	
	if (setpoint>=2044)
	setpoint = 2043;
	
	return(setpoint);
}


void PWM_Init(void)
{
	PWM_OFF;
	TCCR1A = 0x82;		//COM1A1=1, COM1A0=0, COM1B1=0,COM1B0=0, WGM11=1,WGM10=0
						//Clear OC1A/OC1B on compare match, set OC1A/OC1B at BOTTOM (non-inverting mode)
	TCCR1B = 0x09; 		//ICNC1=0,ICES1=0,NoPrescaler,Mode=6(FastPWM,9-bit)
	TIMSK1 |= _BV(TOIE1);

}


void Wait(unsigned char dauer)
{
	dauer = (unsigned char)TCNT0 + dauer;
	while((TCNT0 - dauer) & 0x80);
}


void RotBlink(unsigned char anz)
{
	sei(); // Interrupts ein
	Limit_I = anz;
	while(anz--)
	{
		RED_ON;
		Delay_ms(300);
		RED_OFF;
		Delay_ms(300);
	}
	Delay_ms(1000);
}


char StartingUp(unsigned int pwm)
{
	unsigned long timer = 300,i;
	
	DISABLE_SENSE_INT;
	PWM = pwm * 4;
	Strom = 0;
	SetPWM();
	Phase = 0;
	Manually();
	MinUpmPulse = SetDelay(300);
	while(!CheckDelay(MinUpmPulse))
	{
		FastADConvert();
		if(Strom > 120)
		{
			FETS_OFF; 						// Shutdown due to short circuit
			RotBlink(10);
			NotStarted_anz = 10;
			return(0);
		}
	} 
	PWM = pwm * 8;
	SetPWM();
	while(1)
	{		
		for(i=0;i<timer; i++)
		{
			Wait(100);	// wait
			USART0_TransmitTxData();
		}		
		FastADConvert();
		if(Strom > 90)
		{
			FETS_OFF; 						// Shutdown due to short circuit
			RotBlink(10);
			return(0);
		}
		
		timer-= (timer/15) + 1;
		if (timer<15)
			return(1);
		
		Manually();
		Phase++;
		Phase %= 6;
		AdConvert();
	}		
}


#define TEST_STROMGRENZE 120
unsigned char DelayM(unsigned int timer)
{
	while(timer--)
	{
		FastADConvert();
		if(Strom > (TEST_STROMGRENZE + RuheStrom))
		{
			FETS_OFF;
			return(1);
		}
	}
	return(0);
}


unsigned char Delay(unsigned int timer)
{
	while(timer--)
	{
	//   if(SENSE_H) { PORTC |= ROT; } else { PORTC &= ~ROT;}
}
return(0);
}



#define HIGH_A_EIN DDRB &= ~(_BV(3))
#define HIGH_B_EIN DDRB &= ~(_BV(2))
#define HIGH_C_EIN DDRD &= ~(_BV(2))
#define HIGH_A_OFF DDRB |= _BV(3)
#define HIGH_B_OFF DDRB |= _BV(2)
#define HIGH_C_OFF DDRD |= _BV(2)
#define LOW_A_EIN  PORTD |= _BV(3)
#define LOW_B_EIN  PORTD |= _BV(4)
#define LOW_C_EIN  PORTD |= _BV(5)

//############################################################################
void MotorTon(void)
{
	unsigned char ADR_TAB[19] = {0,0,2,1,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17};
	unsigned int BattVoltage;
	unsigned int i;
	unsigned char anz = 0;
	unsigned char MosfetOkay = 0xFF;


	RED_OFF;
	HIGH_OFF;
	uart_putchar('\n');
	uart_putchar('\r');
	DelayM(2);
	Delay_ms((300 * ADR_TAB[MotorAddress])+100);
	if (I2C_Setpoint!=0)
		return;

	DISABLE_SENSE_INT;
	cli();
	FETS_OFF;
	Strom_max = 0;
	DelayM(500);
	RuheStrom = Strom;
	BattVoltage = MessAD(7);			//Read UBat
	OCR1A = 500;
	Strom = 0;
	Delay(10000);
	Strom = 0;
	Delay(10000);
	NotStarted_anz = 0;
	SENSE_A;

	FETS_OFF;
	LOW_B_EIN;
	LOW_C_EIN;

	Strom = 0;
	
	#define TONDAUER  40000
	#define SOUND_E 1  // 1
	#define SOUND1_A 300
	#define SOUND2_A 330
	#define SOUND3_A 360
	
	for(i=0; i< (TONDAUER / SOUND2_A) ; i++)
	{
		HIGH_A_EIN; // Test A
		HIGH_B_OFF;
		HIGH_C_OFF;
		Delay(SOUND_E);
		if(MessAD(0) > 100)
			MosfetOkay |= 0x01;
		else 
			MosfetOkay &= ~0x01;
	
		HIGH_OFF;
		Delay(SOUND1_A);
	}

	FETS_OFF;
	LOW_A_EIN; // Low A ein
	LOW_C_EIN; // Low C ein
	for(i=0; i<(TONDAUER / SOUND1_A); i++)
	{
		HIGH_B_EIN; // Test B
		HIGH_A_OFF;
		HIGH_C_OFF;
		Delay(SOUND_E);
		if(MessAD(1) > 100)
			MosfetOkay |= 0x02;
		else 
			MosfetOkay &= ~0x02;
		HIGH_OFF;
		Delay(SOUND1_A);
	}
		
	FETS_OFF;
	LOW_A_EIN; // Low A ein
	LOW_B_EIN; // Low B ein
	for(i=0; i<(TONDAUER / SOUND3_A); i++)
	{
		DDRB |= 0x0C; // Test C		
		HIGH_C_EIN;		
		Delay(SOUND_E);
		if(MessAD(2) > 100)
			MosfetOkay |= 0x04;
		else
			MosfetOkay &= ~0x04;
		HIGH_OFF;
		Delay(SOUND2_A);
	}

	FETS_OFF;
	LOW_A_EIN; // Low A ein
	for(i=0; i< (TONDAUER / SOUND2_A) ; i++)
	{
		HIGH_B_EIN;
		HIGH_A_OFF;
		HIGH_C_OFF;
		Delay(SOUND_E);
		if(MessAD(0) > BattVoltage)
			MosfetOkay &= ~0x08;			
		else
			MosfetOkay |= 0x08;
		HIGH_OFF;
		Delay(SOUND2_A);
	}

	FETS_OFF;
	LOW_C_EIN;	// Low C ein
	for(i=0; i< (TONDAUER / SOUND1_A) ; i++)
	{
		HIGH_B_EIN;
		HIGH_A_OFF;
		HIGH_C_OFF;
		Delay(SOUND_E);
		if(MessAD(2) > BattVoltage)
			MosfetOkay &= ~0x20;			
		else
			MosfetOkay |= 0x20;
		HIGH_OFF;
		Delay(SOUND3_A);
	}

	FETS_OFF;
	LOW_B_EIN; // Low B ein
	for(i=0; i<(TONDAUER / SOUND3_A); i++)
	{
		DDRB |= 0x0C;	
		HIGH_C_EIN;
		Delay(SOUND_E);
		if(MessAD(1) > BattVoltage)
			MosfetOkay &= ~0x10;			
		else
			MosfetOkay |= 0x10;
		HIGH_OFF;
		Delay(SOUND3_A);
	}
			
	FETS_OFF;
	UCSR0B &= ~(_BV(RXCIE0));
	UCSR0B &= ~(_BV(TXCIE0));
	sei();
	if (BattVoltage<=235)
		return;
	Delay_ms(250*MotorAddress);

	if(!(MosfetOkay & 0x01))  { anz = 1; uart_putchar('A');uart_putchar('+'); } else
 	if(!(MosfetOkay & 0x02))  { anz = 2; uart_putchar('B');uart_putchar('+'); } else
 	if(!(MosfetOkay & 0x04))  { anz = 3; uart_putchar('C');uart_putchar('+'); } else
 	if(!(MosfetOkay & 0x08))  { anz = 4; uart_putchar('a');uart_putchar('-'); } else
 	if(!(MosfetOkay & 0x10))  { anz = 5; uart_putchar('b');uart_putchar('-'); } else
 	if(!(MosfetOkay & 0x20))  { anz = 6; uart_putchar('c');uart_putchar('-'); }
	NotStarted_anz = anz;
	if(anz) while(1) RotBlink(anz);
	RotBlink(anz);
 	uart_putchar('.');
	uart_putchar('\n');
	uart_putchar('\r');
}



int main(void)
{
	char altPhase = 0;
	int test = 0;
	int tmp, tmp1;
	unsigned int Blink2;
	unsigned int MittelstromTimer,DrehzahlMessTimer,MotorGestopptTimer;
	int tmpCurr;
	
	DDRC  = 0x00;					// PORTC all inputs
	PORTC = 0x10;					// PORTC.4(SDA) = 1
	DDRD  = 0xBE;					// PORTD.0(RxD),PORTD.6(MITTEL) - inputs
	PORTD = 0x00;
	DDRB  = 0x0E;					// PORTB.1,2,3 - outputs
	PORTB = 0xF1;					// PORTB.1,2,3 = 0
	DDRD &= ~(_BV(LED));	// PORTD.7(LED) - output

	Delay(1);
	DelayM(1);

	#define ADRESSOFFSET 0

	#if (MOTORADDRESS == 0)
		PORTB |= (ADR1 + ADR2);   // Pullups fRr Adresswahl
		for(test=0;test<500;test++);
		 

		if(PINB & ADR1)                                                                                      
		{                                                                                                    
			if (PINB & ADR2)                                                                                   
				MotorAddress = 1 + ADRESSOFFSET;                                                                 
			else                                                                                               
				MotorAddress = 2 + ADRESSOFFSET;                                                                 
		}                                                                                                    
		else                                                                                                 
		{                                                                                                    
			if (PINB & ADR2)                                                                                     
				MotorAddress = 3 + ADRESSOFFSET;                                                                   
			else                                                                                                 
				MotorAddress = 4 + ADRESSOFFSET;                                                                   
		}                                                                                                                                                                                         
	#else
		MotorAddress  = MOTORADDRESS;
	#endif

	if (!(PINB & ADR3))
		MotorAddress += 4;

	Limitstrom = ProgConst1;
	if (Limitstrom == 39)
		Limitstrom = 46;

	sei();
	UART_Init();
	Timer0_Init();
	PWM_Init();
	InitPPM();
	InitIC2_Slave(0x50);
	ADC_Init();
	FastADConvert();
	MinUpmPulse = SetDelay(500);
	while(CheckDelay(MinUpmPulse)!=0)
		if(I2C_Setpoint) break;

	PWM = 0;
	SetPWM();
	ADMUX = 1;
	PPMsignal = 0;
	PORTB = 0x31;
	uart_putchar('\n');
	uart_putchar('\r');
	if (I2C_Setpoint==0)
		MotorTon();

	MotorGestopptTimer = SetDelay(101);
	Blink2 = SetDelay(102);
	MinUpmPulse = SetDelay(103);
	MittelstromTimer = SetDelay(254);
	DrehzahlMessTimer = SetDelay(105);
	UART_SendMotorAddr();
	LoadBLConfigFromEEPROM();
	BLConfig.SetMask = 0;
	UCSR0B |= _BV(RXCIE0);			//Enable UART Rx Interrupt
	altPhase = Phase;
	RuheStrom = 0;
	while(1)
	{
		PWM = DetermineSetpoint();
		if (PWM==0)
			PWM = DetermineSetpoint();

		if(Phase != altPhase)					// there was a commutation in interrupt
		{
			MotorStopped = 0;
			ZeitFuerBerechnungen = 0;			// immediately after a commutation time
			MinUpmPulse = SetDelay(150);		// Timeout, in case a Motor is stopped
			altPhase = Phase;
		}

		if(!PWM)				// if setpoint == 0
		{
			MotorStartingUp = 0;      		// No Start Attempt
			unk_100134 = 0;
			ZeitFuerBerechnungen = 0;
			if(CheckDelay(MotorGestopptTimer))
			{
				DISABLE_SENSE_INT;
				MotorStopped = 1;
				FETS_OFF;
				if (RuheStrom>Strom)
					RuheStrom--;
				else
					if (RuheStrom<Strom)
						RuheStrom++;
				MotorGestopptTimer = SetDelay(50);
			}
		}			
		else
		{
			if(MotorStopped)
				MotorStartingUp = 1;				// Start attempt
			MotorGestopptTimer = SetDelay(1500);
		}

		if(MotorStopped)
			PWM = 0;
		SetPWM();

	
		if(--ZeitFuerBerechnungen == 0xFF)			//if ZeitFuerBerechnungen==0xFF then go
		{
			ZeitFuerBerechnungen = 50;
			CheckI2CConfigBuf();
			if (CrcOkay!=0)
			{
				USART0_TransmitTxData();
				USART0_ProcessRxData();
			}

			if(MotorStopped && ZeitZumAdWandeln)
			{
				FastADConvert();
				ZeitZumAdWandeln=0;
			}

			if(CheckDelay(MittelstromTimer))
			{
				MittelstromTimer = SetDelay(50); // 50ms
				tmpCurr = StromADC / MittelStromADcount;
				StromADC = Strom;
				MittelStromADcount = 1;
				if (RuheStrom<=tmpCurr)
				{
					tmpCurr -= RuheStrom;
					tmpCurr = (unsigned int)(tmpCurr * BLConfig.CurrentScaling) / 64;
				}
				else
					 tmpCurr=0;
				

				if ((unsigned int)tmpCurr > Mittelstrom)
					Mittelstrom++;
				else
				{
					if ((unsigned int)tmpCurr < Mittelstrom)
						Mittelstrom--;
				}

				if ((unsigned int)Mittelstrom >= 11)
				{
					//tmpCurr -= Mittelstrom;
					tmpCurr = (tmpCurr-Mittelstrom) / 8;
					Mittelstrom += tmpCurr;
				}

				if (((unsigned int)Mittelstrom/10)>BLConfig.CurrentLimit)
				{
					tmp1 = 255 - ((Mittelstrom - (BLConfig.CurrentLimit * 10)) *3);

					if (tmp1<25)
						tmp1 = 25;

					if (tmp1>255)
						tmp1 = 255;

					Limit_I = tmp1;	
				}
				else
					Limit_I = 255;


				if (CntKommutierungen >= 500)
				{
					CntKommutierungen = 500;
					if ((rpm_raw>=201) && ((unsigned int)PWM>0))
						MotorStartingUp = 1;
				}

				rpm_raw = CntKommutierungen / 2;

				if (unk_100134>100)
				{
					if (CntKommutierungen < (unk_100130/2))
						unk_10013E = 3;
					else
						unk_10013E = 0;
				}
				else
					unk_10013E = 0;

				if ( (PWM!=0) && (unk_100134>100) && (CntKommutierungen<20) && (unk_100130>40))
					MotorStartingUp = 1;
				
				unk_100130 = 0;
				CntKommutierungen = 0;

				sub211();
				
				if (CrcOkay!=0)
					CrcOkay--;
			}

			if(CheckDelay(DrehzahlMessTimer))   // Ist-Drehzahl bestimmen
			{					
				DrehzahlMessTimer = SetDelay(5);
				if (unk_100134!=0xFF)
					unk_100134++;
				ZeitZumAdWandeln = 1;
			}

			if(  (CheckDelay(MinUpmPulse)!=0) && (rpm_raw<10) || (MotorStartingUp!=0) )
			{
				MinUpmPulse = SetDelay(100);
				unk_100134 = 0;
				MotorStopped = 1;
				DISABLE_SENSE_INT;
				if(MotorStartingUp && (unsigned int)Temperature>2440)			//Temperature < 90degC
				{
					RED_OFF;
					Strom_max = 0;
					MotorStartingUp = 0;
					tmp = (BLConfig.BitConfig & 0x0E) * 2;
					tmp = (tmp - (UBAT/24)) + 11;
					if (tmp<4) tmp = 4;
					if (StartingUp(tmp))
					{
						unk_10013E = 2;
						MotorStopped = 1;
						PWM = 8;
						SetPWM();
						CntKommutierungen = 0;
						unk_100130 = 0;
						SENSE_TOGGLE_INT;
						ENABLE_SENSE_INT;
						MinUpmPulse = SetDelay(50);
						while(!CheckDelay(MinUpmPulse))	{};	// kurz Durchstarten
						tmp = 18 + ( (BLConfig.BitConfig & 0x0E) - (UBAT/16) );
						if (tmp<8) tmp = 8;
						PWM = (tmp * 8) + 56;
						SetPWM();
						if(DetermineSetpoint())
							MinUpmPulse = SetDelay(300);
						unk_10013E = 1;
						while (CheckDelay(MinUpmPulse)==0)
						{
							if (Strom>100)			//101 - LIMITSTROM/2
							{
								FETS_OFF;
								RotBlink(10);
								if (DetermineSetpoint()) MotorStartingUp = 1;
							}
							if (CntKommutierungen==0)
								break;
						}
						unk_10013E = 0;
						DrehzahlMessTimer = SetDelay(50);
						altPhase = 7;
					}
					else
						if(DetermineSetpoint())
							MotorStartingUp = 1;
				}
			}
		}
	}
}
