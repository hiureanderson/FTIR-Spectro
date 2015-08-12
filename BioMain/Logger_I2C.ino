#ifdef I2C_EEPROM

/* 
 *  Use the I2C bus with EEPROM 24LC64 
 *  Sketch:    eeprom.pde
 *  Author: Internet
 *  Modified : David Lambelet
 *  Date: 13/08/2014
 */
#include <avr/interrupt.h>
#include <Wire.h> //I2C library
#include <wdt.h> //I2C library

#define WRITE_DELAY    0// 4ms wait before next write  
 

//Choice was made to only log the IR intensity and status 
#define ENTRY_SIZE_LOGS     8// Each log take 8 bytes
#define NB_PARAMETERS_LOGS  2
#define SIZE_MEMORY     65536 // M24512, 512 Kbits = 64Kbytes
#define NBMAX_LOGS     8192
#define EntryID_MAX  NBMAX_LOGS-1

//#define DEBUG_FLAG 1

//// Definition of the log sectors in the flash for the logs
#define DEVICE_ADDRESS         I2C_EEPROM // address of eeprom on the bus i2c
#define ADDRESS_BEG_EEPROM     0x0000
#define ADDRESS_MAX_EEPROM     0xFFFF // address 65535

// on 4 bytes, define the log number and log local time
  uint32_t entryID=0;       
  uint32_t timeLog=0;

//set additionnal busy flag so no log while print log active 
boolean  erase_flag= false ;
boolean  busy_flag= false ;


byte buffer[16];
char char_buffer[16] ;
byte readint[2];

/********************
 * Fonction for the Log
 * // David Lambelet 30.08.2014
 * // Modified Quentin Cabrol 2014.11
 **********************/

//default write log with event code 0 and parameter event 0
void writeLog(){
  writeLog(0x0000,0x0000);
}

// The fonction log the parameters in the eeprom by block of 64bytes
// With a memory of 64kbytes, 1024 logs are possible
// the variable entryID indicate the number of the log
void  writeLog (uint16_t event_number, uint16_t parameter_value)
{
  //ensure we will not go to sleep before writing
  #ifdef POWER_SAVE
    busy_thread_ON();
  #endif

  int i=0;
  // avoid writing when busy (should not occur)
  while(busy_flag && i<100){
    nilThdSleepMilliseconds(10);;
    i++;
  }

  if (busy_flag){
    #ifdef DEBUG_FLAG
    Serial.println("writing");
    #endif
    return;
  }
  
  busy_flag=true;
  if(!erase_flag){ //check first that we are not erasing the ram
    int param = 0;
    int addr_log=0;
    timeLog=now();
    
    /**************************/
    //    LOGGING THE ID      //
    //***********************//
    int ms_entryID =(int)(entryID >> 16);
    int ls_entryID =(int)(entryID & 0xFFFF);
    i2c_eeprom_write_check_int(DEVICE_ADDRESS,ENTRY_SIZE_LOGS*(entryID % NBMAX_LOGS), ms_entryID);
    i2c_eeprom_write_check_int(DEVICE_ADDRESS,ENTRY_SIZE_LOGS*(entryID % NBMAX_LOGS)+2, ls_entryID);

    
    
    /****************************/
    //    LOGGING THE EPOCH     //
    //************************//
    int ms_timeLog=(int)(timeLog >> 16);
    int ls_timeLog=(int)(timeLog & 0xFFFF);
    i2c_eeprom_write_check_int(DEVICE_ADDRESS,ENTRY_SIZE_LOGS*(entryID % NBMAX_LOGS)+4, ms_timeLog);
    i2c_eeprom_write_check_int(DEVICE_ADDRESS,ENTRY_SIZE_LOGS*(entryID % NBMAX_LOGS)+6, ls_timeLog);
    
    //****************************//
    //    LOGGING THE PARAMs      //
    //***************************//
    // write to EEPROM 2 bytes per parameter
    for(int i = 0; i < NB_PARAMETERS_LOGS; i++) {
      param = getParameter(i);
      addr_log=((ENTRY_SIZE_LOGS*(entryID % NBMAX_LOGS))+(2*i)+8);                    
      i2c_eeprom_write_check_int(DEVICE_ADDRESS, addr_log, param);
    }
    
    busy_flag=false;
    entryID++;  
    //allow sleep if the power save mode is active
  #ifdef POWER_SAVE
    busy_thread_OFF();
  #endif  
  }
}

// This fonction return the number of entryID done
// The entry of the last log = nb entryID-1;
// Have to be use to initialise the variable entryID at the beginning
uint32_t find_lastEntry()
{
  uint32_t current_=0;
  uint32_t next_=1;
  int i=0;  
  //check if the next identifier is different from the actual
  while(current_ != next_ && (i<NBMAX_LOGS))
  {
    current_= ((uint32_t)(i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*i))<<16) | ((uint32_t)(i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*i+2)));
    next_= ((uint32_t)(i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*(i+1)))<<16) | ((uint32_t)(i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*(i+1)+2)));

    if((current_+1)!= next_ ){
      // case of erased memory (0XFFFF FFFF)
      if (current_== 0xFFFFFFFF){
        current_=0;
      }
      return current_;       
    }    
    i=(i+1)%NBMAX_LOGS;
  }
  #ifdef DEBUG_FLAG
  Serial.print("Failed to retrieve last");
  #endif
  return 0;
}


uint32_t printLogN(Print* output, uint32_t entryN) {
  
  int i=0;
  // avoid writing when busy (should not occur)
  while(busy_flag && i<100){
    nilThdSleepMilliseconds(10);
    i++;
  }
  
  if (busy_flag){
    #ifdef DEBUG_FLAG
    Serial.println("ERROR eeprom");
    #endif
    return 0xFFFFFFFF;
  }

  busy_flag=true;
  // Are we asking for a log entry that is not on the card anymore ? Then we just start with the first that is on the card
  // And we skip a sector ...
  if ((entryID > EntryID_MAX) && (entryN < (entryID - EntryID_MAX))) {         
    entryN=entryID -  EntryID_MAX +1;
  }

  uint32_t addressOfEntryN = entryN % NBMAX_LOGS;
  byte checkDigit=0;
  for(byte i = 0; i < ENTRY_SIZE_LOGS; i++) {
    byte oneByte=i2c_eeprom_read_byte(DEVICE_ADDRESS,ENTRY_SIZE_LOGS*addressOfEntryN+i);        //change to read byte
    checkDigit^=toHex(output, oneByte);
  }
  checkDigit^=toHex(output, (int)getQualifier());
  toHex(output, checkDigit);
  output->println("");
  busy_flag=false;
  return entryN; 

}

void printLastLog(Print* output) {
  printLogN(output, entryID-1);
}


//************************
// DEBUG UTILITIES
//************************

#ifdef DEBUG_FLAG
void print_logs_debug()
{
  int nblog=0;

  if(entryID>EntryID_MAX)
    nblog=NBMAX_LOGS;  

  else
    nblog= entryID;   

  uint32_t temp_Time = 0;
  uint32_t temp_Entry =0 ;

  for(int i=(max(0,((entryID-2)%NBMAX_LOGS)));i<=(entryID%NBMAX_LOGS)-1;i++)    
  {
    temp_Entry = (((uint32_t)i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*i))<<16)| ((uint32_t)i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*i+2));
    Serial.print(ENTRY_SIZE_LOGS*i);
    Serial.print(" EntryID: ");
    Serial.println(temp_Entry);

    Serial.print(ENTRY_SIZE_LOGS*i);
    Serial.print(" EntryADDR: ");
    Serial.println(temp_Entry % NBMAX_LOGS);

    temp_Time = (((uint32_t)i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*i+4))<<16) |((uint32_t)i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*i+6));
    Serial.print(ENTRY_SIZE_LOGS*i+2);
    Serial.print(" Time: ");
    Serial.println(temp_Time);

    for(int j=0;j<4;j++)
    {
      Serial.print(ENTRY_SIZE_LOGS*i+8+2*j);
      Serial.print(" param ");
      Serial.print(j);
      Serial.print(" : ");
      Serial.println(i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*i+8+2*j));
    }
    Serial.print("EVENT ID :");
    Serial.println(i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*i+ENTRY_SIZE_LOGS-4));    
    Serial.print("EVENT Value :");
    Serial.println(i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*i+ENTRY_SIZE_LOGS-2));  
    Serial.println("******************************");
  }
  Serial.print("*NB LOG IN MEMORY: ");
  Serial.print(nblog);
  Serial.println("      *");
  Serial.println("******************************");
}

void  print_buffer (byte* buf,int sizebuf)
{
  Serial.println("buffer");
  for(int i=0; i<sizebuf ;i++)
  {
    Serial.print(i);
    Serial.print(" : ");
    Serial.println(*buf);
    buf++;
  }
}
#endif

/********************
 * Fonction for the external I2C EEPROM, taken from Arduino website 
 **********************/

void i2c_eeprom_write_byte( int deviceaddress, unsigned int eeaddress, byte data ) {
  int rdata = data;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(rdata);
  Wire.endTransmission();
}

// WARNING: address is a page address, 6-bit end will wrap around
// also, data can be maximum of about 30 bytes, because the Wire library has a buffer of 32 bytes 
void i2c_eeprom_write_page( int deviceaddress, unsigned int eeaddresspage, byte* data, byte length ) {
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddresspage >> 8)); // MSB
  Wire.write((int)(eeaddresspage & 0xFF)); // LSB
  byte c;
  for ( c = 0; c < length; c++)
    Wire.write(data[c]);
  Wire.endTransmission();
}

byte i2c_eeprom_read_byte( int deviceaddress, unsigned int eeaddress ) {    
  byte rdata = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,1);
  if (Wire.available()) rdata = Wire.read();
  return rdata;
}

// maybe let's not read more than 30 or 32 bytes at a time!
void i2c_eeprom_read_buffer( int deviceaddress, unsigned int eeaddress, byte *buffer, int length ) {
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,length);
  int c = 0;
  for ( c = 0; c < length; c++ )
    if (Wire.available()) buffer[c] = Wire.read();
}

/********************
 * Others utils functions for EEPROM
 * // David Lambelet 29.08.2014
 * // Quentin Cabrol 30.11.2014
 **********************/

// failsafe int 16 writing
void i2c_eeprom_write_check_int( int deviceaddress, unsigned int eeaddress, int data){
  int written = 0; 
  int i=0;
  do{
    i2c_eeprom_write_int(deviceaddress, eeaddress, data);
    //placed here to let time between write and read (3ms causes bug)
    nilThdSleepMilliseconds(5); 
    written = i2c_eeprom_read_int(deviceaddress, eeaddress); 
    i++;
  }
  while(written != data && i<100);
  if(i>=100){
    #ifdef DEBUG_FLAG
    Serial.println("error write");
    #endif
    delay(15);
  }
}

uint32_t i2c_eeprom_read_int32(int deviceaddress, unsigned int eeaddress){ 
  uint32_t written;
  written=(((i2c_eeprom_read_int(deviceaddress, eeaddress))<<16) | (i2c_eeprom_read_int(deviceaddress, eeaddress+2)));
  return written;  
}

void i2c_eeprom_write_int( int deviceaddress, unsigned int eeaddress, int data ) {
  
  byte data_msb = ((byte)(data>>8));
  byte data_lsb = ((byte)(data & 0xFF));
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.write(data_msb); // msb at eeaddress+1
  Wire.write(data_lsb); // lsb at eeaddress 
  Wire.endTransmission();
}

int i2c_eeprom_read_int( int deviceaddress, unsigned int eeaddress ) {
  
  int data = 0xFFFF;
  byte data_msb = 0xFF;
  byte data_lsb = 0xFF;
  Wire.beginTransmission(deviceaddress);
  Wire.write((int)(eeaddress >> 8)); // MSB
  Wire.write((int)(eeaddress & 0xFF)); // LSB
  Wire.endTransmission();
  Wire.requestFrom(deviceaddress,2);
  if (Wire.available()) data_msb = Wire.read(); // msb at eeaddress+1
  if (Wire.available()) data_lsb = Wire.read(); // lsb at eeaddress
  data=data_lsb+(data_msb << 8);
  return data;
}

void i2c_eeprom_init_erase(int deviceaddress){
  erase_flag=TRUE;
}

//not working well
void i2c_eeprom_erase(int deviceaddress){
  setTime(0);
  for(unsigned int i=0; i<=(ADDRESS_MAX_EEPROM)/2;i++){
    i2c_eeprom_write_check_int(DEVICE_ADDRESS,2*i,-1);
    if(!(i%200))
      Serial.println( (unsigned int)i );
      //nilThdSleepMilliseconds(20);
    //delay(10);
  }
  Serial.println("Done"); 
  erase_flag=false;
}


void i2c_eeprom_readall() {
  unsigned int i=0;
  int value=0;
  while(i<ADDRESS_MAX_EEPROM/2){
    value=i2c_eeprom_read_int(DEVICE_ADDRESS,2*i);
    nilThdSleepMilliseconds(10);
    #ifdef DEBUG_FLAG
    if(value != -1){
    Serial.print("int");
    Serial.print((unsigned int)i);
    Serial.print(" : ");
    Serial.println(value);
    }
    #endif
    i++;
  }
}

/*************************************
 * I2C EEPROM THREAD
 **************************************/
//NIL_WORKING_AREA(waThreadI2C_EEPROM, 124);
NIL_WORKING_AREA(waThreadI2C_EEPROM, 200);
NIL_THREAD(ThreadI2C_EEPROM, arg) {
  
  //setParameter(PARAM_STATUS,(getParameter(PARAM_STATUS)&~(FLAG_ERASE_EEPROM)));
  busy_flag=true;
  //Wait for the Wire thread to be ready before to start
  nilThdSleepMilliseconds(5000);
  entryID=find_lastEntry();
  //set time to last recorded
  timeLog = ((((uint32_t)i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*(entryID % NBMAX_LOGS)+4))*65536) + ((uint32_t)i2c_eeprom_read_int(DEVICE_ADDRESS, ENTRY_SIZE_LOGS*(entryID % NBMAX_LOGS)+6)));
  setTime(timeLog);
  busy_flag=false;
  writeLog(EVENT_ARDUINO_BOOT,0x0000);

  while (TRUE) {
    nilThdSleepMilliseconds(10000);
    
    writeLog();
    //print_logs_debug();   

    //erase ram condition
    if(erase_flag==TRUE){
      //setParameter(PARAM_STATUS,(getParameter(PARAM_STATUS)|FLAG_ERASE_EEPROM));
      wdt_disable();
      i2c_eeprom_erase(I2C_EEPROM);
      entryID=find_lastEntry();
      wdt_enable(WDTO_8S);
      //setParameter(PARAM_STATUS,(getParameter(PARAM_STATUS)&~(FLAG_ERASE_EEPROM)));
      busy_flag=false;
    }
  }
}

#endif



