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

#define DIGITAL_5       5//D5 OC4A  
#define DIGITAL_6       6//D6 OC4D
#define DIGITAL_7       7//D7
#define DIGITAL_8       8//D8 A8
#define DIGITAL_9       9//D9 OC4B, OC1A, PCINT5
#define DIGITAL_10      10//D10 A10
#define DIGITAL_13      13//D13

#define ANALOG_0     18//0
#define ANALOG_1     19//A1
#define ANALOG_2     20//A2
#define ANALOG_3     21//A3
#define ANALOG_4     22//A4

/**************************************
 * ACTIVE THREAD DEPENDING CARD TYPE
 **************************************/

#ifdef TYPE_FTIR
  #define MODEL_ZIGBEE       //Serial Com on Zigbee antenna SZ05
  #define I2C_EEPROM 80      //I2C EEPROM memory 512k or 1M (m24512)
  
  #define IR_FAN      DIGITAL_5
  #define IR_SRC      DIGITAL_6
  #define PUMP_12V    DIGITAL_9
  
  #define TEMPERATURE_CTRL 1    //DS18B20+ OneWire inputs
    //#define PELTIER_PID  1    //Peltier Cooling Thread


    #define TEMP_ALU       ANALOG_0    //PID COLD
    #define TEMP_IR        ANALOG_1 
    #define TEMP_SINK      ANALOG_2
    #define TEMP_SENSOR    ANALOG_3    //PID HOT

    #define PELTIER_50     DIGITAL_7   //PWM
    #define PELTIER_75     DIGITAL_10  //PWM

//  #define IR_SENSE      1
    #define I2C_IR_INTENSITY  9   //change the digital input here with the I2 address of the ADC
    #define IR_INTENSITY      ANALOG_4   
  
//  #define PIEZO_DRV           1 //piezo driver on DAC1220 via SPI
    #define PIEZO_SELECT      DITGITAL_8     

  #define THR_MONITORING     1  // starts the blinking led and the watch dog counter 
  #define MONITORING_LED     DIGITAL_13
    
#endif

/***********************
 * SERIAL, LOGGER AND DEBUGGER
 ************************/

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
  #define PARAM_TEMP_ALU     3
  #define PARAM_TEMP_IR      4
  #define PARAM_TEMP_SENSOR  5
  #define PARAM_TEMP_SINK    6
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
#define MAX_EPOCH  (long int) 1500000000  //epoch cannot be forced externally  after  Fri, 14 Jul 2017 02:40:00 GMT
#define MIN_EPOCH  (long int) 1400000000  //epoch cannot be forced externally  before Tue, 13 May 2014 16:53:20 GMT
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
  
  /*
  delay(2000);
  pinMode(IR_FAN,OUTPUT);
  digitalWrite(IR_FAN,HIGH);  
  delay(3000);
  pinMode(IR_SRC,OUTPUT);
  digitalWrite(IR_SRC,HIGH);
  delay(2000);
    pinMode(PUMP_12V ,OUTPUT);
  digitalWrite(PUMP_12V ,HIGH);*/

}
