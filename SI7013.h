/* 
 Si7013 Temperature/Humidity Library
 SmokingResistor.com
 Date: July 19, 2014

 Derived from:
 SparkFun Electronics HTU21D Humidity Library
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 Get humidity and temperature from the Si7013 sensor.
 
 */
 

#if defined(ARDUINO) && ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>

#define SI7013_ADDRESS 0x40  //Unshifted 7-bit I2C address if AD0 is pulled low during bootup
#define SI7013_ADDRESS 0x41  //Unshifted 7-bit I2C address if AD0 is floating or pulled high during bootup

#define MEASURE_RH_HOLD  0xE5
#define MEASURE_RH_NOHOLD  0xF5
#define MEASURE_TEMP_HOLD  0xE3
#define MEASURE_TEMP_NOHOLD  0xF3
#define MEASURE_ANALOG  0xEE
#define READ_TEMP_FROM_PREV_RH  0xE0
#define RESET  0xFE
#define WRITE_USER_REG1  0xE6
#define READ_USER_REG1  0xE7
#define WRITE_USER_REG2  0x50
#define READ_USER_REG2  0x10
#define WRITE_USER_REG3  0x51
#define READ_USER_REG3  0x11
#define WRITE_THERMISTOR_COEFF  0xC5
#define READ_THERMISTOR_COEFF  0x84
#define READ_ID0  0xFA
#define READ_ID1  0x0F
#define READ_ID2  0xFC
#define READ_ID3  0xC9
#define READ_FIRMWARE  0x84
#define READ_FIRMWARE  0xB8

class SI7013 {

public:
  SI7013();

  //Public Functions
  bool begin();
  float readHumidity(void);
  float readTemperature(void);
  void setResolution(byte resBits);

  //Public Variables

private:
  //Private Functions

  byte read_user_register(void);
  byte check_crc(uint16_t message_from_sensor, uint8_t check_value_from_sensor);

  //Private Variables

};
