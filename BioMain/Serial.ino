#ifdef THR_SERIAL

#define SERIAL_BUFFER_LENGTH 32
char serialBuffer[SERIAL_BUFFER_LENGTH];
byte serialBufferPosition=0;

NIL_WORKING_AREA(waThreadSerial, 240); // minimum 128
NIL_THREAD(ThreadSerial, arg) {

  Serial.begin(9600);
  while(true) {
    while (Serial.available()) {
      // get the new byte:
      char inChar = (char)Serial.read(); 

      if (inChar==13 || inChar==10) {
        // this is a carriage return;
        if (serialBufferPosition>0) {
          printResult(serialBuffer, &Serial);
        } 
        serialBufferPosition=0;
        serialBuffer[0]='\0';
      } 
      else {
        if (serialBufferPosition<SERIAL_BUFFER_LENGTH) {
          serialBuffer[serialBufferPosition]=inChar;
          serialBufferPosition++;
          if (serialBufferPosition<SERIAL_BUFFER_LENGTH) {
            serialBuffer[serialBufferPosition]='\0';
          }
        }
      }  
    }
    nilThdSleepMilliseconds(30);
  }
}



#endif



