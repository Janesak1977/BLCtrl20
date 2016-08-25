// Part of BL_Ctrl 2.0

#include "main.h"

unsigned int MinUpmPulse = 0;
volatile unsigned char Phase = 0;


 //############################################################################
// + Interruptroutine
// + Dispatched by Analog Comparator
// + Commutation generates this
SIGNAL(ANALOG_COMP_vect)
//############################################################################
{
	unsigned char sense = 0;
	unsigned char Cntr = 30;
	if (TmrOvfCnt==0)
	{
		do                                                                            
		{	                                                                            
			if(SENSE_H) sense = 1;                                                      
				else sense = 0;                                                           
			if (!(BLConfig.BitConfig & 0x01))                                              
			{                                                                           
				switch(Phase)                                                             
				{                                                                         
					case 0:                                                                 
						STEUER_A_H;                                                           
						if(sense)                                                             
						{                                                                     
							STEUER_C_L;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_FALLING_INT;                                                  
							SENSE_B;                                                            
							Phase++;                                                            
							CntKommutierungen++;
							TmrOvfCnt = unk_10013E;                                                
						}									// Current flow A->C windings, Read B falling       
						else                                                                  
						{                                                                     
							STEUER_B_L;                                                         
						}									// Current flow A->B windings                       
					break;                                                                  
 	                                                                                
					case 1:                                                                 
						STEUER_C_L;                                                           
						if(!sense)                                                            
						{                                                                     
							STEUER_B_H;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_A;                                                            
							SENSE_RISING_INT;                                                   
							Phase++;                                                            
							CntKommutierungen++;
							TmrOvfCnt = unk_10013E;                                                
						}								// Current flow B->C windings, Read A rising          
						else                                                                  
						{                                                                     
							STEUER_A_H;		// Current flow A->C windings	                        
						}                                                                     
					break;                                                                  
 	                                                                                
					case 2:                                                                 
						STEUER_B_H;                                                           
						if(sense)                                                             
						{                                                                     
							STEUER_A_L;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_C;                                                            
							SENSE_FALLING_INT;                                                  
							Phase++;                                                            
							CntKommutierungen++;
							TmrOvfCnt = unk_10013E;                                                
						}							// Current flow B->A windings, Read C falling           
						else                                                                  
						{                                                                     
							STEUER_C_L;                                                         
						}                                                                     
					break;                                                                  
 	                                                                                
					case 3:                                                                 
						STEUER_A_L;                                                           
						if(!sense)                                                            
						{                                                                     
							STEUER_C_H;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_B;                                                            
							SENSE_RISING_INT;                                                   
							Phase++;                                                            
							CntKommutierungen++;
							TmrOvfCnt = unk_10013E;                                                
						}            	// Current flow C->A windings, Read B rising            
						else                                                                  
						{                                                                     
							STEUER_B_H;                                                         
						}                                                                     
					break;                                                                  
 	                                                                                
					case 4:                                                                 
						STEUER_C_H;                                                           
						if(sense)                                                             
						{                                                                     
							STEUER_B_L;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_A; 		                                                        
							SENSE_FALLING_INT;                                                  
							Phase++;                                                            
							CntKommutierungen++;
							TmrOvfCnt = unk_10013E;                                                
						}							// Current flow C->B windings, Read A falling           
						else                                                                  
						{                                                                     
							STEUER_A_L;                                                         
						}                                                                     
					break;                                                                  
 	                                                                                
					case 5:                                                                 
						STEUER_B_L;                                                           
						if(!sense)                                                            
						{                                                                     
							STEUER_A_H;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_C;                                                            
							SENSE_RISING_INT;                                                   
							Phase = 0;                                                          
							CntKommutierungen++;
							TmrOvfCnt = unk_10013E;                                                
						}         		// Current flow A->B windings, Read C rising            
						else                                                                  
						{                                                                     
							STEUER_C_H;                                                         
						}                                                                     
					break;                                                                  
				}			//end switch(Pha                                              
			}                                                                           
			else		//else if (BLCong.BitConfig & 0x01)                             
			{                                                                           
				switch(Phase)                                                             
				{                                                                         
					case 0:
						STEUER_B_H;                                                           
						if(sense)                                                             
						{                                                                     
							STEUER_C_L;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_FALLING_INT;                                                  
							SENSE_A;                                                            
							Phase++;                                                            
							CntKommutierungen++;
							TmrOvfCnt = unk_10013E;                                                
						}									// Current flow B->C windings, Read A falling       
						else                                                                  
						{                                                                     
							STEUER_A_L;                                                         
						}									// Current flow B->A windings                       
					break;
					
					case 1:
						STEUER_C_L;                                                           
						if(!sense)                                                             
						{                                                                     
							STEUER_A_H;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_B;                                                  
							SENSE_RISING_INT;                                                            
							Phase++;                                                            
							CntKommutierungen++;
							TmrOvfCnt = unk_10013E;                                                
						}									// Current flow A->C windings, Read B rising       
						else                                                                  
						{                                                                     
							STEUER_B_H;                                                         
						}									// Current flow B->C windings                       
					break;
					
					case 2:
						STEUER_A_H;                                                           
						if(sense)                                                             
						{                                                                     
							STEUER_B_L;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_C;                                                  
							SENSE_FALLING_INT;                                                            
							Phase++;                                                            
							CntKommutierungen++;
							TmrOvfCnt = unk_10013E;                                                
						}									// Current flow A->B windings, Read C falling       
						else                                                                  
						{                                                                     
							STEUER_C_L;                                                         
						}									// Current flow A->C windings                       
					break;
					
					case 3:
						STEUER_B_L;                                                           
						if(!sense)                                                             
						{                                                                     
							STEUER_C_H;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_A;                                                  
							SENSE_RISING_INT;                                                            
							Phase++;                                                            
							CntKommutierungen++;
							TmrOvfCnt = unk_10013E;                                                
						}									// Current flow C->B windings, Read A rising       
						else                                                                  
						{                                                                     
							STEUER_A_H;                                                         
						}									// Current flow A->B windings                       
					break;
					
					case 4:
						STEUER_C_H;                                                           
						if(sense)                                                             
						{                                                                     
							STEUER_A_L;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_B;                                                  
							SENSE_FALLING_INT;                                                            
							Phase++;                                                            
							CntKommutierungen++;                                                
							TmrOvfCnt = unk_10013E;
						}									// Current flow C->A windings, Read B falling       
						else                                                                  
						{                                                                     
							STEUER_B_L;                                                         
						}									// Current flow C->B windings                       
					break;
					
					case 5:
						STEUER_A_L;                                                           
						if(!sense)                                                             
						{                                                                     
							STEUER_B_H;                                                         
							if(ZeitZumAdWandeln) AdConvert();                                   
							SENSE_C;                                                  
							SENSE_RISING_INT;                                                            
							Phase++;                                                            
							CntKommutierungen++;                                            
							TmrOvfCnt = unk_10013E;
						}									// Current flow B->A windings, Read C rising       
						else                                                                  
						{                                                                     
							STEUER_C_H;                                                         
						}									// Current flow C->A windings                       
					break;		
				}
			}
			unk_100130++;			                                                                           
			if (--Cntr==255) 
				break;
		} while( (SENSE_L && sense) || (SENSE_H && !sense) );
	}
	ZeitZumAdWandeln = 0;		
}



//############################################################################
// Manually
//############################################################################
void Manually(void)           // Manually
{
	if ((BLConfig.BitConfig & _BV(0))==0)
	{
		switch(Phase)
		{
			case 0:
				STEUER_A_H;
				STEUER_B_L;
				SENSE_C;
				SENSE_RISING_INT;
			break;
			
			case 1:
				STEUER_A_H;   	// PhaseA TOP ON
				STEUER_C_L;		// PhaseC BOT ON
				SENSE_B;				// Detect ZeroCross on PhaseB
				SENSE_FALLING_INT;
			break;
			
			case 2:
				STEUER_B_H;		// PhaseB TOP ON
				STEUER_C_L;		// PhaseC BOT ON
				SENSE_A;
				SENSE_RISING_INT;
			break;
			
			case 3:
				STEUER_B_H;
				STEUER_A_L;
				SENSE_C;
				SENSE_FALLING_INT;
			break;
			
			case 4:
				STEUER_C_H;
				STEUER_A_L;
				SENSE_B;
				SENSE_RISING_INT;
			break;
			
			case 5:
				STEUER_C_H;
				STEUER_B_L;
				SENSE_A;
				SENSE_FALLING_INT;
			break;			
		}
	}
	else
	{
		switch(Phase)
		{
			case 0:
				STEUER_B_H;
				STEUER_A_L
				SENSE_C;
				SENSE_RISING_INT;
			break;
			
			case 1:
				STEUER_B_H;
				STEUER_C_L;
				SENSE_A;
				SENSE_FALLING_INT;			
			break;
			
			case 2:
				STEUER_A_H;
				STEUER_C_L;
				SENSE_B;
				SENSE_RISING_INT;
			break;
			
			case 3:
				STEUER_A_H;
				STEUER_B_L;
				SENSE_C;
				SENSE_FALLING_INT;
			break;
			
			case 4:
				STEUER_C_H;
				STEUER_B_L;
				SENSE_A;
				SENSE_RISING_INT;
			break;
			
			case 5:
				STEUER_C_H;
				STEUER_A_L;
				SENSE_B;
				SENSE_FALLING_INT;
			break;
		}	
	}	
}	
