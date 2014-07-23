/* 
 SI7013 Humidity Sensor Example Code
 SmokingResistor.com
 Date: July 19, 2014
 
 Derived from:
 SparkFun Electronics HTU21D Humidity Sensor Library
 License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).
 
 Uses the SI7013 library to display the current humidity and temperature
 Open serial monitor at 9600 baud to see readings. Errors 998 if not sensor is detected. Error 999 if CRC is bad.
  
 Hardware Connections (Breakoutboard to Arduino):
 -VCC = 5V
 -GND = GND
 -SDA = A4 
 -SCL = A5

 */

#include <Wire.h>
#include "SI7013.h"

//Create an instance of the object
SI7013 myHumidity;

void setup()
{
  Serial.begin(9600);
  Serial.println("SI7013 Example!");

  myHumidity.begin();
}

void loop()
{
  float humd = myHumidity.readHumidity();
  float temp = myHumidity.readTemperature();

  Serial.print("Time:");
  Serial.print(millis());
  Serial.print(" Temperature:");
  Serial.print(temp, 1);
  Serial.print("C");
  Serial.print(" Humidity:");
  Serial.print(humd, 1);
  Serial.print("%");

  Serial.println();
  delay(1000);
}
