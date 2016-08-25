

typedef struct
{
	uint8_t Revision;			// must be BL_REVISION
	uint8_t SetMask;			// settings mask
	uint8_t PwmScaling;			// maximum value of control pwm, acts like a thrust limit
	uint8_t CurrentLimit;		// current limit in A
	uint8_t TempLimit;			// in °C
	uint8_t CurrentScaling;		// scaling factor for current measurement
	uint8_t BitConfig;			// see defines above
	uint8_t crc;				// checksum
}  __attribute__((packed)) BLConfig_t;


#define MASK_SET_PWM_SCALING		0x01	//bit0
#define MASK_SET_CURRENT_LIMIT		0x02	//bit1
#define MASK_SET_TEMP_LIMIT			0x04	
#define MASK_SET_CURRENT_SCALING	0x08
#define MASK_SET_BITCONFIG			0x10
#define MASK_RESET_CAPCOUNTER		0x20
#define MASK_SET_DEFAULT_PARAMS		0x40
#define MASK_SET_SAVE_EEPROM	 	0x80


unsigned int RAM_Checksum(unsigned char* pBuffer, unsigned int len);
void SaveBLConfigToEEPROM(void);
void LoadBLConfigFromEEPROM(void);
void StoreToNVROM(unsigned char*,unsigned char*, unsigned int);
void ReadNVRAM(unsigned char*, unsigned char*, unsigned int);

