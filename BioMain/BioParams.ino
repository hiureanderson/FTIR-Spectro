#define ERROR_VALUE  -32768


// Definition of all events to be logged
#define EVENT_ARDUINO_BOOT           1
#define EVENT_ARDUINO_SET_SAFE       2

#define EVENT_TEMP_IR_FAILED        50
#define EVENT_TEMP_IR_RECOVER       51
#define EVENT_TEMP_SINK_FAILED      52
#define EVENT_TEMP_SINK_RECOVER     53
#define EVENT_TEMP_SENSOR_FAILED    54
#define EVENT_TEMP_SENSOR_RECOVER   55
#define EVENT_TEMP_ALU_FAILED       54
#define EVENT_TEMP_ALU_RECOVER      55

#define EVENT_ERROR_NOT_FOUND_ENTRY_N  150


void resetParameters() { 
  
#ifdef     TEMPERATURE_CTRL
  setAndSaveParameter(PARAM_TEMP_IR, ERROR_VALUE);
  setAndSaveParameter(PARAM_TEMP_SINK, ERROR_VALUE);
  setAndSaveParameter(PARAM_TEMP_SENSOR, ERROR_VALUE);
  setAndSaveParameter(PARAM_TEMP_ALU, ERROR_VALUE);
  setAndSaveParameter(PARAM_TEMP_IR_MAX, 7000);
  setAndSaveParameter(PARAM_TEMP_SINK_MAX, 7000);
  
  #ifdef TEMP_PID
  setAndSaveParameter(PARAM_TARGET_SENSOR_TEMP, -500);
  setAndSaveParameter(PARAM_PID_REGULATION_TIME_WINDOW, 5000);
  setAndSaveParameter(PARAM_TEMP_SENSOR_MIN, -1000);  
  //setAndSaveParameter(PARAM_MIN_TEMPERATURE, 1000);  // not used but could be used for safety
  //setAndSaveParameter(PARAM_MAX_TEMPERATURE, 4000);  // not used but could be used for safety  
  #endif
  
#endif

  setAndSaveParameter(PARAM_STATUS, 0); // 0b0000 1111 activate food_control, ph_control, gas_control, stepper_control

}
