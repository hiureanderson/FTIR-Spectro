#if defined (PIEZO_DRV) && defined(I2C_IR_INTENSITY)

#define EEPRO
//#define IR_DEBUG 1
#define MAX_STEP (unsigned int) 0xFFFF
#define STEP_JUMP     2
#define ACQUISITIONS  4

/*****************************************************
SPI thread to control the DAC1120
65535 acquisition steps after the IR flag is raised
Acquisition performed via ZTP-115M module (Amphenol) 
ADC conversion on I2C MCP3425 element (see ThreadWire)
Author : Quentin Cabrol @EPFL-Univalle 
Date   : 16.09.2015
******************************************************/

NIL_WORKING_AREA(waThreadIR,96);
NIL_THREAD(ThreadIR, arg) {      

  //wait for all threads to be ready
  nilThdSleepMilliseconds(5000); 
  //setup SPI interface
  SPI.begin();  
  nilThdSleepMilliseconds(200);   
  //Set DAC 1220  	
  SPI.setBitOrder(MSBFIRST);
  digitalWrite(PIEZO_SELECT, LOW);
  SPI.transfer(0b00100100); // Cmd byte=Write mode (1st bit= 0, 2 bytes to write 0bx01xXXXX on CMR (addr4))
  SPI.transfer(0b00100000); //config MSB 
  SPI.transfer(0b00110000); //config LSB
  //end communication
  digitalWrite(PIEZO_SELECT, HIGH);
  //lock acquisition awating for acquisition order
  eeprom_write_word((uint16_t*) LOCKER,0);
  
  while(TRUE){

     
     //thread variables
     unsigned int step_number=0; // step number beween 0 and MAX_STEP
     byte acq_number=0; // acq_number between 0 and ACQUISITION
     
    //here add a condition on the flag // flag has to be defined in bioparaams
    while(eeprom_read_word((uint16_t*) EEPROM_IR_FLAG)==1){
      
        //Set DAC 1220
        digitalWrite(PIEZO_SELECT, LOW);
        SPI.transfer(0b00100000);   // Cmd byte=Write mode (1st bit= 0, 1 byte to write 0bx01xXXXX addr0=DIR_MSB)
        SPI.transfer((byte)((step_number>>8)& 0xFF));  // DIR MSB
        SPI.transfer((byte)((step_number)& 0xFF));      // DIR LSB       
        digitalWrite(PIEZO_SELECT, HIGH);
        //Acquisition of the IR sensor on 16 bits I2C with auto-average
        setParameter(PARAM_I2C_IR,getSensor());
        
        
        #ifdef IR_DEBUG
          Serial.println(getParameter(PARAM_I2C_IR));
          delay(10);
        #endif
  
  
        //go to the next step
        step_number= (step_number +STEP_JUMP) % (MAX_STEP); 	
        nilThdSleepMilliseconds(100); 
    }
  }
  
}



/*****************************************************
    Dedicated functions
******************************************************/


/***********************
   Dedicated to MCP3424
************************/
#ifdef I2C_IR_INTENSITY
int getSensor() {
    if (wireDeviceExists(I2C_IR_INTENSITY)) {
      long int sum=0;
      byte i=0;
      //built-in average
      for(i=0; i<20;i++){
        wireWrite(I2C_IR_INTENSITY,0b10010000);
        nilThdSleepMilliseconds(6);
        sum += wireReadFourBytesToInt(I2C_IR_INTENSITY);
      //nilThdSleepMilliseconds(100);
      }
      sum = sum/i;
      return sum;
    }
    else{
      //could be anything above PH_INTERCEPT, 
      //just to produce a negative pH value->error
      return 0xFFFF;  
    }
  }
#endif




#endif

