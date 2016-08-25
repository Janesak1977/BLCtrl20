/*############################################################################
############################################################################*/

extern volatile unsigned char PPM_Timeout, anz_ppm_werte;
volatile unsigned int PPMsignal;

void InitPPM(void);

#define ICP_INT_ENABLE   TIMSK1 |= _BV(ICIE1)
#define ICP_INT_DISABLE  TIMSK1 &= ~(_BV(ICIE1))
#define TIMER1_INT_ENABLE   TIMSK1 |= _BV(TOIE1)
#define TIMER1_INT_DISABLE  TIMSK1 &= ~(_BV(TOIE1))
#define ICP_POS_FLANKE    TCCR1B |= (1<<ICES1)
#define ICP_NEG_FLANKE    TCCR1B &= ~(1<<ICES1)
