/**************
 * LIBRAIRIES
 **************/

//MultiThread
#include <NilRTOS.h>

//Lib to access memory with SPI
#include <SPI.h>

// Library that allows to start the watch dog allowing automatic reboot in case of crash
// The lowest priority thread should take care of the watch dog
#include <avr/wdt.h>

// http://www.arduino.cc/playground/Code/Time
// We need to deal with EPOCH
#include <Time.h>

/******************
 * DEFINE CARD TYPE
 ******************/

#define TYPE_FTIR  1 
// THIS SHOULD BE THE ONLY PARAMETER THAT CHANGES !!!!
//#define TYPE_ZIGBEE_GENERAL     1   // card to control the basic functions: pH, motor, temperature
//#define TYPE_PRECISE_PID    1

//#define TYPE_OLD_PIN_CONFIG     1   //define this if you use the card with the old pin configuration
//(old=until integratedBertha v1.0)

/**************************************
 * ACTIVE THREAD DEPENDING CARD TYPE
 **************************************/

#ifdef TYPE_FTIR
  #define MODEL_ZIGBEE       //Serial Com on Zigbee antenna SZ05
  #define I2C_EEPROM 80      //I2C EEPROM memory 512k or 1M (m24512)
  
  #define TEMPERATURE_CTRL 1    //DS18B20+ inputs
    #define PELTIER_PID  1    //Peltier Cooling Thread
    #define TEMP_IR        1  //change the analog input here
    #define TEMP_SINK      2  //change the analog input here
    #define TEMP_SENSOR    3  //change the analog input here
    #define TEMP_ALU       4  //change the analog input here
    #define PELTIER_50     5  //change the digital output here
    #define PELTIER_75     6  //change the digital output here

  #define IR_SENSE      1
    #define I2C_IR_INTENSITY  9   //change the digital input here
    #define IR_INTENSITY      10   //change the analog input here
  
  #define PIEZO_DRV           1     //piezo driver on DAC1220 via SPI
    #define PIEZO_SELECT      11      //change digital pin

  #define THR_MONITORING     1  // starts the blinking led and the watch dog counter 
  #define MONITORING_LED     13
    
#endif

/***********************
 * SERIAL, LOGGER AND DEBUGGER
 ************************/
// #define EEPROM_DUMP   1   // Gives the menu allowing to dump the EEPROM

#define THR_SERIAL        1

#ifdef MODEL_ZIGBEE
#define THR_ZIGBEE      1 // communication process on Serial1
#endif

/*******************************
 * CARD DEFINITION (HARD CODED)
 *******************************/

#ifdef IR_SENSE
  #define PARAM_STEP_NUMBER  0
  #define PARAM_I2C_IR       1
  #define PARAM_AN_IR        2
#endif

#ifdef     TEMPERATURE_CTRL
  #define PARAM_TEMP_IR      3
  #define PARAM_TEMP_SINK    4
  #define PARAM_TEMP_SENSOR  5
  #define PARAM_TEMP_ALU     6
  #if defined (PELTIER_PID)
    #define PARAM_TARGET_SENSOR_TEMP          7
    #define PARAM_TEMP_IR_MAX                 8
    #define PARAM_TEMP_SINK_MAX               9
    #define PARAM_TEMP_SENSOR_MIN             10
    #define PARAM_PID_REGULATION_TIME_WINDOW  11 //in [ms]
  #endif
#endif

/******************
 * FLAG DEFINITION
 ******************/

//Status parameters giving the system state
#define PARAM_STATUS          12

#define STARTUP               0
#define COOLING               1
#define PUMPING               2
#define ACQUIRE               3
#define ERROR_TEMP_IR_HOT     4
#define ERROR_TEMP_SINK_HOT   5
#define ERROR_TEMP_IR_FAIL    6
#define ERROR_TEMP_SINK_FAIL  7
#define ERROR_TEMP_SENSE_FAIL 8
#define ERROR_TEMP_ALU_FAIL   9
#define ERROR_IR_SENSE_FAIL   10
#define IR_FAN_ON             11
#define COOLING_FAN_ON        12

/*********
 * Autoreboot parameters
 *********/
#define AUTOREBOOT 36000 // we will reboot automatically every 1h ... bad trick to prevent some crash problems of ethernet ...
uint16_t autoreboot=0;

/*********
 * SETUP
 *********/

void setup() {
  delay(2000);
  Serial.begin(9600);
  delay(1000);
  setupParameters();

  setSafeConditions(false);
  nilSysBegin();

}

void loop() {

}
