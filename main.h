#ifndef _MAIN_H
 #define _MAIN_H

#define VERSION_MAJOR 0
#define VERSION_MINOR 31
#define VERSION_PATCH 0
#define VERSION_SERIAL_MAJOR 11
#define VERSION_SERIAL_MINOR 0


#define MANUELL_PWM       0   // zur manuellen Vorgabe der PWM  Werte: 0-255
#define TEST_MANUELL      0   // zum Testen der Hardware ohne Kommutierungsdetektion  Werte: 0-255 (PWM)
#define TEST_SCHUB        0	  // Erzeugt ein Schubmuster        Werte: 0 = normal 1 = Test
#define POLANZAHL        12   // Anzahl der Pole (Magnete) Wird nur zur Drehzahlausgabe über Debug gebraucht
#define MAX_PWM         255
#define MIN_PWM           3
#define MIN_PPM          10   // ab hier (PPM-Signal) schaltet der Regler erst ein
#define FILTER_PPM        7   // wie stark soll das PPM-Signal gefiltert werden (Werte: 0-30)?

#define SIO_DEBUG         0   // Testwertausgaben auf der seriellen Schnittstelle


#define MAX_STROM        200  // ab ca. 20A PWM ausschalten
#define LIMIT_STROM      120  // ab ca. 12A PWM begrenzen

#define SYSCLK	8000000L	//Quarz Frequenz in Hz


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/boot.h>
#include <avr/wdt.h>

#include "uart.h"
#include "timer.h"
#include "BLMC.h"
#include "PPM_Decode.h"
#include "analog.h"
#include "TWISlave.h"
#include "eeprom.h"


#define BITCONF_REVERSE_ROTATION 0x01
#define BITCONF_RES1 0x02
#define BITCONF_RES2 0x04
#define BITCONF_RES3 0x08
#define BITCONF_RES4 0x10
#define BITCONF_RES5 0x20
#define BITCONF_RES6 0x40
#define BITCONF_RES7 0x80


extern signed int PWM;
extern unsigned int  I2C_Timeout;
extern unsigned int SIO_Timeout;
extern unsigned char unk_100134;
extern unsigned char MotorAddress;
extern unsigned char ProgConst1;
extern unsigned char CurrentLimit;
extern unsigned int  CntKommutierungen;
extern unsigned int  Strom;			//ca. in 0,1A
extern unsigned int StromADC;
extern unsigned int Strom_max;
extern int Mittelstrom;
extern int Temperature;
extern unsigned char Temperatur;
extern unsigned int I2C_Setpoint;
extern unsigned int ConsumptedCurrent;
extern unsigned char ZeitZumAdWandeln;
extern unsigned int UBAT;
extern unsigned int rpm_raw;
extern unsigned char MaxSetpoint;
extern unsigned char Limit_I;
extern unsigned char unk_10012F;
extern unsigned int unk_100130;
extern unsigned char MittelStromADcount;
extern unsigned char unk_10013E;
extern unsigned char NotStarted_anz;
extern BLConfig_t BLConfig;
extern int OCR1Atemp;
extern unsigned char ADC_Timeout;
extern unsigned char TmrOvfCnt;
extern unsigned int I2C_Setpoint;

unsigned int SetupConfigBytes(unsigned char *CfgDataPtr);
void PrepareDebugData(void);

#define LED		7

#define RED_ON  {DDRD &= ~(_BV(LED)); }
#define RED_OFF {DDRD |= _BV(LED); }

#define ADR1   0x40    // für Motoradresswahl
#define ADR2   0x80    //
#define ADR3   0x01



#endif //_MAIN_H
