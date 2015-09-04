#ifdef PELTIER_PID
#ifdef TEMPERATURE_CTRL

#include <PID_v1.h>

//#define DEBUG_PID

void pid_ctrl();
void PIDSetup();

int count=0;
double coolingRegInput;
double coolingRegOutput;
double coolingRegSetpoint;

unsigned long coolingRegWindowStartTime;
//Specify the heating regulation links and initial tuning parameters //Kp=100; Ti=0.2 Td=5 are initial testing param.
//PID object definition can be found in PID library (to include for compilation)
PID coolingRegPID(&coolingRegInput, &coolingRegOutput, &coolingRegSetpoint, 7000,15,1600, DIRECT);


NIL_WORKING_AREA(waThread_Peltier, 128); // minimum of 16 The momory change with time
NIL_THREAD(Thread_Peltier, arg) 
{
  

  nilThdSleepMilliseconds(5000); 
  //initialize the Peltier elements
  pinMode(PELTIER_50, OUTPUT);
  pinMode(PELTIER_75, OUTPUT);
  //Todo : update heatingSetup when a parameter is changed
  PIDSetup();
  
  while(TRUE){
     
     #ifdef DEBUG_PID
     if(count%30==0)
      Serial.print(getParameter(PARAM_TEMP_SENSOR));
    count=(count+1)%101;
    #endif
    
    pid_ctrl();
    nilThdSleepMilliseconds(500);  //refresh every 500ms
  }
}


/*Temperature PID Control addressing relay*/
void pid_ctrl()
{
  float exactPresentTime;
  
    coolingRegInput = -1*getParameter(PARAM_TEMP_SENSOR);
    coolingRegSetpoint = -1*getParameter(PARAM_TARGET_SENSOR_TEMP);
    coolingRegPID.Compute();        // the computation takes only 30ms!
    // turn the output pin on/off based on pid output
    exactPresentTime = millis();

  if (exactPresentTime - coolingRegWindowStartTime > getParameter(PARAM_PID_REGULATION_TIME_WINDOW)) { 
    //time to shift the Relay Window
    coolingRegWindowStartTime += getParameter(PARAM_PID_REGULATION_TIME_WINDOW);
  }
  
   if((coolingRegOutput > exactPresentTime - coolingRegWindowStartTime) 
    //&& (getParameter(PARAM_TEMP_SINK)<getParameter(PARAM_TEMP_SINK_MAX))
    && (getParameter(PARAM_TEMP_SENSOR)>getParameter(PARAM_TEMP_SENSOR_MIN))
    && (getParameter(PARAM_TEMP_SENSOR) != 0xFF) && (getParameter(PARAM_TEMP_SINK)!=0xFF))
  {
    Serial.println("on");
    digitalWrite(PELTIER_75, HIGH);
    digitalWrite(PELTIER_50, HIGH);
  }  
  else 
  {
    Serial.println("off");
    digitalWrite(PELTIER_75, LOW);
    digitalWrite(PELTIER_50, LOW);
  } 
  
}


// see the rest of oliver's code for sanity checks

void PIDSetup()
{ 
  coolingRegPID.SetOutputLimits(0, getParameter(PARAM_PID_REGULATION_TIME_WINDOW));          //what is heating regulation time windows ???
  //turn the PID on, cf. PID library
  coolingRegPID.SetMode(AUTOMATIC);                 
  //set PID sampling time to 10000ms                   //possibly set a timer condition with a nilsleep instead
  coolingRegPID.SetSampleTime(10000);

  coolingRegWindowStartTime = millis();
  // heatingRegSetpoint = getParameter(PARAM_TEMP_MAX);
}

#endif
#endif
