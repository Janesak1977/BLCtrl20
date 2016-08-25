#include "main.h"


BLConfig_t BLConfig;


void WriteByteEEPROM(unsigned int EEAddr,unsigned char byte)
{
	while (((EECR & _BV(EEPE)) == 1));
	EECR = 0x00;
	EEARH = EEAddr >> 8;
	EEARL = EEARH & 0x00FF;

	
}


void StoreToNVROM(unsigned char* EEAddr,unsigned char* buf, unsigned int size)
{
	unsigned char* ptr;
	unsigned char memptr = 0;
	
	if (size==0)
		return;

	ptr = EEAddr;
	do
	{
		eeprom_write_byte(ptr,buf[memptr++]);
		ptr++;
		size--;
	} while(size!=0);

}

void ReadNVRAM(unsigned char* buf, unsigned char* EEAddr, unsigned int size)
{
	unsigned char* ptr;
	unsigned char memptr = 0;

	if (size==0)
		return;

	ptr = EEAddr;
	{
		buf[memptr++] = eeprom_read_byte(ptr);
		ptr++;
		size--;
	} while(size!=0);
}

unsigned int RAM_Checksum(unsigned char* pBuffer, unsigned int len)
{
	unsigned char crc = 0xAA;
	unsigned int i;

	for(i=0; i<len; i++)
	{
		crc += pBuffer[i];
	}
	return crc;
}




void SetDefaultBLConfig(void)
{
	BLConfig.Revision = 2;
	BLConfig.SetMask = 0;
	BLConfig.PwmScaling = 255;
	BLConfig.CurrentLimit = 40;
	BLConfig.TempLimit = 100;
	BLConfig.CurrentScaling = 64;
	BLConfig.BitConfig = 4;
	if (BLConfig.CurrentLimit > CurrentLimit) BLConfig.CurrentLimit = CurrentLimit;
	BLConfig.crc = RAM_Checksum((unsigned char*) &BLConfig,sizeof(BLConfig_t)-1);
}


void SaveBLConfigToEEPROM(void)
{
	if (BLConfig.Revision == 2)
	{
		BLConfig.crc = RAM_Checksum((unsigned char*) &BLConfig,sizeof(BLConfig_t)-1);
		StoreToNVROM((unsigned char*)0, (unsigned char*) &BLConfig, sizeof(BLConfig_t)-1);
	}
	
	
}


unsigned int SetupConfigBytes(unsigned char* CfgDataPtr)
{
	unsigned char CfgDat;

	if (RAM_Checksum(CfgDataPtr,sizeof(BLConfig_t)-1)!=CfgDataPtr[7]) return(5);
	if (CfgDataPtr[0] != 2) return(4);		// check BLConfig.Revision
	if (CfgDataPtr[1] & MASK_SET_DEFAULT_PARAMS)
	SetDefaultBLConfig();
	else
	{
		CfgDat=CfgDataPtr[1];
		BLConfig.SetMask = 0;
		if (CfgDat & MASK_SET_PWM_SCALING)
		BLConfig.PwmScaling = CfgDataPtr[2];
		if (CfgDat & MASK_SET_CURRENT_LIMIT)
		BLConfig.CurrentLimit = CfgDataPtr[3];
		if (CfgDat & MASK_SET_TEMP_LIMIT)
		BLConfig.TempLimit = CfgDataPtr[4];
		if (CfgDat & MASK_SET_CURRENT_SCALING)
		BLConfig.CurrentScaling = CfgDataPtr[5];
		if (CfgDat & MASK_SET_BITCONFIG)
		BLConfig.BitConfig = (BLConfig.BitConfig & 0x0E) | (CfgDataPtr[6] & 0xF1);
		if (CfgDat & MASK_RESET_CAPCOUNTER)
		BLConfig.BitConfig = (BLConfig.BitConfig & 0xF1) | (CfgDataPtr[6] & 0x0E);
		if (BLConfig.TempLimit > 114)	BLConfig.TempLimit = 115;
		if (BLConfig.CurrentLimit >= CurrentLimit) BLConfig.CurrentLimit = CurrentLimit;
		BLConfig.crc = RAM_Checksum((unsigned char*) &BLConfig,sizeof(BLConfig_t)-1);
	}
	if (CfgDataPtr[1] & MASK_SET_SAVE_EEPROM)
	SaveBLConfigToEEPROM();
	return(0);
}



void LoadBLConfigFromEEPROM(void)
{
	unsigned char crc;

	ReadNVRAM((unsigned char*) &BLConfig, (unsigned char*) 0, sizeof(BLConfig_t)-1);
	crc = RAM_Checksum((unsigned char*) &BLConfig,sizeof(BLConfig_t)-1);
	if (BLConfig.Revision==2)
	{
		if (BLConfig.crc!=crc)
		{
			SetDefaultBLConfig();
			SaveBLConfigToEEPROM();
		}	
	}
	else
	{
		SetDefaultBLConfig();
		SaveBLConfigToEEPROM();
	}
	
	if (BLConfig.CurrentLimit >= CurrentLimit) BLConfig.CurrentLimit = CurrentLimit;
	if (BLConfig.TempLimit > 114)	BLConfig.TempLimit = 115;
}



