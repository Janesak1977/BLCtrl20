/*############################################################################
############################################################################*/

 
#ifndef BLMC_H_
#define BLMC_H_

extern volatile unsigned char Phase;
extern unsigned int MinUpmPulse;

void Manually(void);

#define A_H	3		//PORTB.3
#define B_H 2   //PORTB.2
#define C_H 2   //PORTD.2
#define A_L 3		//PORTD.3
#define B_L 4		//PORTD.4
#define C_L 5		//PORTD.5


#define PWM_OFF   {PORTB &= ~(0x0C); DDRB |= 0x0C; PORTD &= ~(_BV(C_H)); DDRD |= _BV(C_H);}


#define  STEUER_A_H {DDRB &= ~(_BV(A_H)); DDRB |= _BV(B_H); DDRD |= _BV(C_H); }
#define  STEUER_B_H {DDRB &= ~(_BV(B_H)); DDRB |= _BV(A_H); DDRD |= _BV(C_H); }
#define  STEUER_C_H {DDRB |= 0x0C; DDRD &= ~(_BV(C_H)); }

#define  STEUER_A_L {PORTD &= ~0x30; PORTD |= _BV(A_L); }
#define  STEUER_B_L {PORTD &= ~0x28; PORTD |= _BV(B_L); }
#define  STEUER_C_L {PORTD &= ~0x18; PORTD |= _BV(C_L); }
#define  DRIVER_OFF {PORTD &= ~(0x38); }    // Turn off low side FETs
#define  HIGH_OFF {PORTB &= ~(0x0C); DDRB |= 0x0C; PORTD &= ~(_BV(C_H)); DDRD |= _BV(C_H); }  //Turn off high side FETs
#define  FETS_OFF {PORTD &= ~0x38; PORTB &= ~(0x0C); DDRB |= 0x0C; PORTD &= ~(_BV(C_H)); DDRD |= _BV(C_H); }		//Turn off all FETs

#define SENSE_A ADMUX = 0 | 0xC0;
#define SENSE_B ADMUX = 1 | 0xC0;
#define SENSE_C ADMUX = 2 | 0xC0;

#define ClrSENSE            ACSR |= _BV(ACI)
#define SENSE               ((ACSR & _BV(ACI)))
#define SENSE_L             (!(ACSR & _BV(ACO)))
#define SENSE_H             ((ACSR & _BV(ACO)))
#define ENABLE_SENSE_INT    {ACSR |= 0x0A; }
#define DISABLE_SENSE_INT   {ACSR &= ~(_BV(ACIE)); }

#define SENSE_FALLING_INT   ACSR &= ~(_BV(ACIS0))
#define SENSE_RISING_INT    ACSR |= 0x03
#define SENSE_TOGGLE_INT    ACSR &= ~0x03

#endif //BLMC_H_


