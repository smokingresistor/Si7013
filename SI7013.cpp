/*
 Si7013 Temperature/Humidity Library
 SmokingResistor.com
 Date: July 19, 2014

 Derived from:
 SparkFun Electronics HTU21D Humidity Sensor Library
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

 This library allows an Arduino to read from the Si7013 low-cost high-precision humidity sensor.
 Hardware Setup: The Si7013 lives on the I2C bus. Attach the SDA pin to A4, SCL to A5 of your Arduino board.
 
 Software:
 Call SI7013.Begin() in setup.
 SI7013.ReadHumidity() will return a float containing the humidity.
 SI7013.ReadTemperature() will return a float containing the temperature in Celsius.
 SI7013.SetResolution(byte: 0b.76543210) sets the resolution of the readings.
 SI7013.check_crc(message, check_value) verifies the 8-bit CRC generated by the sensor
 SI7013.read_user_register() returns the user register. Used to set resolution.
 */

#include <Wire.h>
#include "SI7013.h"

SI7013::SI7013()
{
  //Set initial values for private vars
}

//Begin
/*******************************************************************************************/
//Start I2C communication
bool SI7013::begin(void)
{
  Wire.begin();
}

//Read the humidity
/*******************************************************************************************/
//Calc humidity and return it to the user
//Returns 998 if I2C timed out 
//Returns 999 if CRC is wrong
float SI7013::readHumidity(void)
{
	//Request a humidity reading
	Wire.beginTransmission(SI7013_ADDRESS);
	Wire.write(MEASURE_RH_NOHOLD); //Measure humidity with no bus holding
	Wire.endTransmission();

	//Hang out while measurement is taken.
	delay(55);

	//Comes back in three bytes, data(MSB) / data(LSB) / Checksum
	Wire.requestFrom(SI7013_ADDRESS, 3);

	//Wait for data to become available
	int counter = 0;
	while(Wire.available() < 3)
	{
		counter++;
		delay(1);
		if(counter > 100) return 998; //Error out
	}

	byte msb, lsb, checksum;
	msb = Wire.read();
	lsb = Wire.read();
	checksum = Wire.read();

	/* //Used for testing
	byte msb, lsb, checksum;
	msb = 0x4E;
	lsb = 0x85;
	checksum = 0x6B;*/
	
	unsigned int rawHumidity = ((unsigned int) msb << 8) | (unsigned int) lsb;

	if(check_crc(rawHumidity, checksum) != 0) return(999); //Error out

	//sensorStatus = rawHumidity & 0x0003; //Grab only the right two bits
	rawHumidity &= 0xFFFC; //Zero out the status bits but keep them in place
	
	//Given the raw humidity data, calculate the actual relative humidity
	float tempRH = rawHumidity / (float)65536; //2^16 = 65536
	float rh = -6 + (125 * tempRH); //From page 14
	
	return(rh);
}

//Read the temperature
/*******************************************************************************************/
//Calc temperature and return it to the user
//Returns 998 if I2C timed out 
//Returns 999 if CRC is wrong
float SI7013::readTemperature(void)
{
	//Request the temperature
	Wire.beginTransmission(SI7013_ADDRESS);
	Wire.write(MEASURE_TEMP_NOHOLD);
	Wire.endTransmission();

	//Hang out while measurement is taken.
	delay(55);

	//Comes back in three bytes, data(MSB) / data(LSB) / Checksum
	Wire.requestFrom(SI7013_ADDRESS, 3);

	//Wait for data to become available
	int counter = 0;
	while(Wire.available() < 3)
	{
		counter++;
		delay(1);
		if(counter > 100) return 998; //Error out
	}

	unsigned char msb, lsb, checksum;
	msb = Wire.read();
	lsb = Wire.read();
	checksum = Wire.read();

	/* //Used for testing
	byte msb, lsb, checksum;
	msb = 0x68;
	lsb = 0x3A;
	checksum = 0x7C; */

	unsigned int rawTemperature = ((unsigned int) msb << 8) | (unsigned int) lsb;

	if(check_crc(rawTemperature, checksum) != 0) return(999); //Error out

	//sensorStatus = rawTemperature & 0x0003; //Grab only the right two bits
	rawTemperature &= 0xFFFC; //Zero out the status bits but keep them in place

	//Given the raw temperature data, calculate the actual temperature
	float tempTemperature = rawTemperature / (float)65536; //2^16 = 65536
	float realTemperature = -46.85 + (175.72 * tempTemperature); //From page 14

	return(realTemperature);  
}

//Set sensor resolution
/*******************************************************************************************/
//Sets the sensor resolution to one of four levels
//Page 12:
// 0/0 = 12bit RH, 14bit Temp
// 0/1 = 8bit RH, 12bit Temp
// 1/0 = 10bit RH, 13bit Temp
// 1/1 = 11bit RH, 11bit Temp
//Power on default is 0/0

void SI7013::setResolution(byte resolution)
{
  byte userRegister = read_user_register(); //Go get the current register state
  userRegister &= 0b01111110; //Turn off the resolution bits
  resolution &= 0b10000001; //Turn off all other bits but resolution bits
  userRegister |= resolution; //Mask in the requested resolution bits
  
  //Request a write to user register
  Wire.beginTransmission(SI7013_ADDRESS);
  Wire.write(WRITE_USER_REG1); //Write to the user register
  Wire.write(userRegister); //Write the new resolution bits
  Wire.endTransmission();
}

//Read the user register
byte SI7013::read_user_register(void)
{
  byte userRegister;
  
  //Request the user register
  Wire.beginTransmission(SI7013_ADDRESS);
  Wire.write(READ_USER_REG1); //Read the user register
  Wire.endTransmission();
  
  //Read result
  Wire.requestFrom(SI7013_ADDRESS, 1);
  
  userRegister = Wire.read();

  return(userRegister);  
}

//Give this function the 2 byte message (measurement) and the check_value byte from the SI7013
//If it returns 0, then the transmission was good
//If it returns something other than 0, then the communication was corrupted
//From: http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
//POLYNOMIAL = 0x0131 = x^8 + x^5 + x^4 + 1 : http://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks
#define SHIFTED_DIVISOR 0x988000 //This is the 0x0131 polynomial shifted to farthest left of three bytes

byte SI7013::check_crc(uint16_t message_from_sensor, uint8_t check_value_from_sensor)
{
  //Test cases from datasheet:
  //message = 0xDC, checkvalue is 0x79
  //message = 0x683A, checkvalue is 0x7C
  //message = 0x4E85, checkvalue is 0x6B

  uint32_t remainder = (uint32_t)message_from_sensor << 8; //Pad with 8 bits because we have to add in the check value
  remainder |= check_value_from_sensor; //Add on the check value

  uint32_t divsor = (uint32_t)SHIFTED_DIVISOR;

  for (int i = 0 ; i < 16 ; i++) //Operate on only 16 positions of max 24. The remaining 8 are our remainder and should be zero when we're done.
  {
    //Serial.print("remainder: ");
    //Serial.println(remainder, BIN);
    //Serial.print("divsor:    ");
    //Serial.println(divsor, BIN);
    //Serial.println();

    if( remainder & (uint32_t)1<<(23 - i) ) //Check if there is a one in the left position
      remainder ^= divsor;

    divsor >>= 1; //Rotate the divsor max 16 times so that we have 8 bits left of a remainder
  }

  return (byte)remainder;
}
