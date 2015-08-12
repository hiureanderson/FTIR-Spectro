// Nice way to make some monitoring about activity. This should be the lower priority process
// If the led is "stable" (blinks 500 times per seconds) it means there are not too
// many activities on the microcontroler

#define MAX_THREADS 10

boolean threads[MAX_THREADS];



NIL_WORKING_AREA(waThreadMonitoring, 0);
NIL_THREAD(ThreadMonitoring, arg) {
  // we should not start the watchdog too quickly ...
  nilThdSleepMilliseconds(10000);

  // we activate the watchdog
  // we need to make a RESET all the time otherwise automatic reboot: wdt_reset();
  wdt_enable(WDTO_8S);



#ifdef MONITORING_LED
  pinMode(MONITORING_LED, OUTPUT);   
#endif
  while (TRUE) {
#ifdef MONITORING_LED

    digitalWrite(MONITORING_LED,HIGH);
    nilThdSleepMilliseconds(1000);
    digitalWrite(MONITORING_LED,LOW);
    nilThdSleepMilliseconds(1000);
#endif

    autoreboot++;
    if (autoreboot<AUTOREBOOT) {
      wdt_reset();
    } 
    else {
      if (autoreboot==AUTOREBOOT) {
        saveParameters();
        setSafeConditions(true);
      }
    }
  }
}



NIL_THREADS_TABLE_BEGIN()

#ifdef THR_ZIGBEE
NIL_THREADS_TABLE_ENTRY(NULL, ThreadZigbee, NULL, waThreadZigbee, sizeof(waThreadZigbee))
#endif

#ifdef I2C_EEPROM
NIL_THREADS_TABLE_ENTRY(NULL, ThreadI2C_EEPROM, NULL, waThreadI2C_EEPROM, sizeof(waThreadI2C_EEPROM))
#endif


#ifdef THR_LINEAR_LOGS
NIL_THREADS_TABLE_ENTRY(NULL, ThreadLogger, NULL, waThreadLogger, sizeof(waThreadLogger))
#endif


#ifdef TEMPERATURE_CTRL
NIL_THREADS_TABLE_ENTRY(NULL, ThreadTemp, NULL, waThreadTemp, sizeof(waThreadTemp))   
#ifdef PELTIER_PID  
NIL_THREADS_TABLE_ENTRY(NULL, Thread_Peltier, NULL, waThread_Peltier, sizeof(waThread_Peltier))  
#endif    
#endif


#ifdef THR_SERIAL
NIL_THREADS_TABLE_ENTRY(NULL, ThreadSerial, NULL, waThreadSerial, sizeof(waThreadSerial))
#endif

#if defined(I2C_LCD)
NIL_THREADS_TABLE_ENTRY(NULL, ThreadWire, NULL, waThreadWire, sizeof(waThreadWire))
#endif

#ifdef THR_MONITORING
NIL_THREADS_TABLE_ENTRY(NULL, ThreadMonitoring, NULL, waThreadMonitoring, sizeof(waThreadMonitoring))
#endif

NIL_THREADS_TABLE_END()
