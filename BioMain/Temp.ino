#if TEMPERATURE_CTRL

/*******************************************
 *  THREAD TEMPERATURE
 *  This module reads the temperature of the different temperature sensors.
 *  The sensor is a dallas 1-wire ds18b20.
 *  
 *  There is only one device per plug. Instead of searching for the 
 *  adresses of the devices, we use the skip rom command which allow 
 *  us to directly ask any device on the line without address.
 *  
 *  The sequence is as follow: 
 *  
 *  We ask the sensor to update its value :
 *  1. Issue a Reset pulse and observe the Presence of the thermometer
 *  2. Issue the Skip Rom command (0xCC)
 *  3. Issue the Convert T command (0×44)
 *  
 *  The conversion in 12 bits take 750ms, so we actually read the previous value :
 *  1. Issue a Reset pulse and observe the Presence of the thermometer
 *  2. Issue the Skip Rom command (0xCC)
 *  3. Issue the Read Scratchpad command (0xBE)
 *  4. Read the next two bytes which represent the temperature
 *********************************************/

#include <OneWire.h>

byte oneWireAddress[8];

void getTemperature(OneWire &ow, int parameter, byte errorBit, byte failedEvent, byte recoverEvent);

byte errorTemperature=0;

NIL_WORKING_AREA(waThreadTemp, 80);  // should be 50 without Serial.println
NIL_THREAD(ThreadTemp, arg) {
  nilThdSleepMilliseconds(200);


#ifdef TEMP_IR
  OneWire oneWire1(TEMP_IR);
  byte errorTempIR = false;
#endif

#ifdef TEMP_SINK
  OneWire oneWire2(TEMP_SINK);
  byte errorTempSink = false;
#endif

#ifdef TEMP_SENSOR
  OneWire oneWire3(TEMP_SENSOR);
  byte errorTempSensor = false;
#endif

#ifdef TEMP_ALU
  OneWire oneWire4(TEMP_ALU);
  byte errorTempAlu = false;
#endif

  while(true){
#ifdef TEMP_IR
    getTemperature(oneWire1, PARAM_TEMP_IR, 0, EVENT_TEMP_IR_FAILED, EVENT_TEMP_IR_RECOVER);
#endif

#ifdef TEMP_SINK
    getTemperature(oneWire2, PARAM_TEMP_SINK, 1, EVENT_TEMP_SINK_FAILED, EVENT_TEMP_SINK_RECOVER);
#endif

#ifdef TEMP_SENSOR
    getTemperature(oneWire3, PARAM_TEMP_SENSOR, 2, EVENT_TEMP_SENSOR_FAILED, EVENT_TEMP_SENSOR_RECOVER);
#endif

#ifdef TEMP_ALU
    getTemperature(oneWire4, PARAM_TEMP_ALU, 3, EVENT_TEMP_ALU_FAILED, EVENT_TEMP_ALU_RECOVER);
#endif

    nilThdSleepMilliseconds(5000);
  }
}


void getTemperature(OneWire &ow, int parameter, byte errorBit, byte failedEvent, byte recoverEvent) {
  byte data[2];
  // We ask to calculate the temperature
  byte present=ow.reset();
  ow.write(0xCC);
  ow.write(0x44);   
  // conversion of 12 bits takes 750mS, just wait here to be sure
  nilThdSleepMilliseconds(750);

  //We use the return of the reset function to check if the device is present
  // if(present == 0) => one error occured
  // if(*errorTemp) == false => The error has not been logged

  //if error & non logged
  if (present == 0){
    if (bitRead(errorTemperature, errorBit)!=1) {
      bitSet(errorTemperature, errorBit);
      writeLog(failedEvent,0);
      setParameter(parameter, ERROR_VALUE);
    }
  }
  else {  //no error
    //we can log new error again
    if (bitRead(errorTemperature, errorBit)==1) {
      bitClear(errorTemperature, errorBit);
      writeLog(recoverEvent,0);
    }
    //We get the new temperature
    present=ow.reset();
    if (present !=0) {
      ow.write(0xCC);
      ow.write(0xBE);
      data[0] = ow.read();
      data[1] = ow.read();
      present=ow.reset();
      if (present !=0) {
        int16_t raw = (data[1] << 8) | data[0];
        //float celsius = (float)raw / 16.0;
        setParameter(parameter, ((long)raw*625)/100);
      }
    }
  }
}  

#endif

