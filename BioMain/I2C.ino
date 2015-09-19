#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

#define WIRE_MAX_DEVICES 5
byte numberI2CDevices=0;
byte wireDeviceID[WIRE_MAX_DEVICES];

NIL_WORKING_AREA(waThreadWire,88);
NIL_THREAD(ThreadWire, arg) {

  nilThdSleepMilliseconds(1000);

  byte aByte=0;
  byte* wireFlag32=&aByte;
  unsigned int wireEventStatus=0;
  Wire.begin();

  while(true) {

    if (wireEventStatus%25==0) {
      wireUpdateList();
    }
    wireEventStatus++;
 
    nilThdSleepMilliseconds(500); 
           
  }
}

/********************
 * Utilities functions 
 **********************/

void wireWrite(uint8_t address, uint8_t _data ) {
  Wire.beginTransmission(address);
  Wire.write(_data);
  Wire.endTransmission();
}

void wireWrite(uint8_t address, uint8_t reg, uint8_t _data ) // used by 4-relay
{
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(_data);
  Wire.endTransmission();
}


void wireSMBWrite(uint8_t address,uint8_t command,uint8_t _data){
  Wire.beginTransmission(address);
  Wire.write(command);
  Wire.write(_data);
  Wire.endTransmission();
}


byte wireSMBRead(uint8_t address,uint8_t command){
  byte _data=0;
  Wire.beginTransmission(address);
  Wire.write(command);
  Wire.requestFrom(address, (uint8_t)1);
  if(Wire.available()) {
    _data = Wire.read();
  }
  Wire.endTransmission();
  return _data;  
}

byte wireRegRead(uint8_t address,uint8_t reg){
  byte _data=0;
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.endTransmission(false); // Send data to I2C dev with option for a repeated start
  Wire.requestFrom(address, (uint8_t)1);
  if(Wire.available()) {
    _data = Wire.read();
  }
  Wire.endTransmission();
  return _data;  
}

byte wireRead(uint8_t address) {
  byte _data = 0;
  Wire.requestFrom(address, (uint8_t)1);
  if(Wire.available()) {
    _data = Wire.read();
  }
  return _data;
}


int wireReadTwoBytesToInt(uint8_t address) {
  int i = 0;
  int _data = 0;
  int byteWithMSB;
  int byteWithLSB;

  Wire.requestFrom(address, (uint8_t)2);
  while(Wire.available()) {
    if (i > 2) return 0; // security mechanism
    else i++;
    byteWithMSB = Wire.read();
    byteWithLSB = Wire.read();
    _data = (byteWithMSB<<8) | byteWithLSB;
  }  
  return _data;
}


int wireReadFourBytesToInt(uint8_t address) {
  int i = 0;
  unsigned int _data = 0;
  uint8_t byteWithADD;
  uint8_t byteWithMSB;
  uint8_t byteWithLSB;
  uint8_t byteWithCFG;

  Wire.requestFrom(address, (uint8_t)4);
  while(Wire.available()) {
    if (i > 4) return 0; // security mechanism, see if sufficient or not (give false info about the FLUX if the case !!!!)
    else i++;
    byteWithMSB = Wire.read();
    byteWithLSB = Wire.read();
    byteWithCFG = Wire.read();
    byteWithADD = Wire.read();
    _data = (byteWithMSB<<8) | byteWithLSB;
  }
  return _data;
}

void wireInfo(Print* output) {
  //wireUpdateList();
  output->println("I2C");

  for (byte i=0; i<numberI2CDevices; i++) {
    output->print(i);
    output->print(F(": "));
    output->print(wireDeviceID[i]);
    output->print(F(" - "));
    output->println(wireDeviceID[i],BIN);
  }
}


void wireUpdateList() {
  // 16ms
  byte _data;
  byte currentPosition=0;
  // I2C Module Scan, from_id ... to_id
  for (byte i=0; i<=127; i++)
  {
    Wire.beginTransmission(i);
    Wire.write(&_data, 0);
    // I2C Module found out!
    if (Wire.endTransmission()==0)
    {
      // there is a device, we need to check if we should add or remove a previous device
      if (currentPosition<numberI2CDevices && wireDeviceID[currentPosition]==i) { // it is still the same device that is at the same position, nothing to do
        currentPosition++;
      } 
      else if (currentPosition<numberI2CDevices && wireDeviceID[currentPosition]<i) { // some device(s) disappear, we need to delete them
        wireRemoveDevice(currentPosition);
        i--;
      } 
      else if (currentPosition>=numberI2CDevices || wireDeviceID[currentPosition]>i) { // we need to add a device
        //Serial.print("add: ");        DEBUG POUR CONNAITRE L'ADRESSE DE L'I2C !!!!!!!!
        //Serial.println(i);
        wireInsertDevice(currentPosition, i);
        currentPosition++;
      }
      nilThdSleepMilliseconds(1);
    }
  }
  while (currentPosition<numberI2CDevices) {
    wireRemoveDevice(currentPosition);
  }
}

void wireRemoveDevice(byte id) {
  for (byte i=id; i<numberI2CDevices-1; i++) {
    wireDeviceID[i]=wireDeviceID[i+1];
  }
  numberI2CDevices--;
}

void wireInsertDevice(byte id, byte newDevice) {
  //Serial.println(id);

  if (numberI2CDevices<WIRE_MAX_DEVICES) {
    for (byte i=id+1; i<numberI2CDevices-1; i++) {
      wireDeviceID[i]=wireDeviceID[i+1];
    }
    wireDeviceID[id]=newDevice;
    numberI2CDevices++;
  } 
}

boolean wireDeviceExists(byte id) {
  for (byte i=0; i<numberI2CDevices; i++) {
    if (wireDeviceID[i]==id) return true;
  }
  return false; 
}


void sendRelay(byte id, byte value, byte* flag) {
  if (wireDeviceExists(id)) {
    if (!wireFlagStatus(flag, id))
    {
      setWireFlag(flag, id);
      wireWrite(id, 0x05, 0b00000100); // initialize CONFREG (0x05)
      wireWrite(id, 0x00, 0b00000000); // initialize IOREG (0x00)
    }
    wireWrite(id, 0x0A, value); // pin control
  }
  else
  {
    clearWireFlag(flag, id);
  }
}


// We will combine flags in a byte. Using pointer does not seems to improve
// memory size so we don't use pointer
void setWireFlag(byte *aByte, byte address) {
  *aByte |= (1 << (address & 0b00000111));
}

void clearWireFlag(byte *aByte, byte address) {
  *aByte &= ~(1 << (address & 0b00000111));
}

boolean wireFlagStatus(byte *aByte, byte address) {
  return *aByte & (1 << (address & 0b00000111));
}







