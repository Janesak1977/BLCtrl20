#define TIMER_TEILER     CK8
#define TIM0_START       TIMSK0 |= _BV(TOIE0)


extern volatile unsigned int CountMilliseconds;
extern unsigned char TmrOvfCnt;
extern volatile unsigned int I2C_Lost;


void Timer0_Init(void);
unsigned int SetDelay (unsigned int);
unsigned char CheckDelay (unsigned int t);
void Delay_ms(unsigned int);

