#include "WiSEN.h"

// The number of average
#define AVERAGE 10

float pressure = 0;

void setup()
{  
  // Initial WiSEN Hardware
  WiSEN.Init();
  
  Serial.begin(115200);
  delay(1000);

  Serial.println("+++ Demo 0-20mA Sensor Input +++\n");
  
  Serial.println(">>> Please Config Jumper for AN1 to 0-20mA");
  Serial.println("*** J5 - CLOSE");
  Serial.println("*** J6 - CLOSE");
  Serial.println("*** J7 - OPEN");
  Serial.println("*** J8 - OPEN");
  Serial.println(">>> Please Config Jumper for AN2 to 0-20mA");
  Serial.println("*** J1 - CLOSE");
  Serial.println("*** J2 - CLOSE");
  Serial.println("*** J3 - OPEN");
  Serial.println("*** J4 - OPEN");
  
  Serial.println("Turn ON Sensor Power Supply.");
  WiSEN.SensorPower(ON);
}

void loop()
{
  // Connect 0-20mA Sensor to AN1
  Serial.println("Sensor (0-20mA) => <AN1>");
  Serial.printf("ADC = %u\n", WiSEN.ReadADC(AN1, AVERAGE));
  // Get Analog Input A1 (0-20mA mode) in average 10
  float ain1_current = WiSEN.Get0to20mA(AN1, AVERAGE);
  Serial.printf("Sensor = %.2f mA\n", ain1_current);
  
  // Example connect 0-20mA pressure sensor 0-250 bar to AN1
  pressure = mapfloat(ain1_current, 0, 20, 0, 250);
  Serial.printf("Pressure = %.1f bar\n", pressure);
  //Current between 0-20 mA is distributed as pressure between 0-250 bar.

  // Connect 4-20mA Sensor to AN2
  Serial.println("Sensor (0-20mA) => <AN2>");
  Serial.printf("ADC = %u\n", WiSEN.ReadADC(AN2, AVERAGE));
  // Get Analog Input A2 (0-20mA mode) in average 10
  float ain2_current = WiSEN.Get0to20mA(AN2, AVERAGE);
  Serial.printf("Current = %.2f mA\n", ain2_current);
  
  // Example connect 4-20mA pressure sensor 0-250 bar to AN2
  pressure = mapfloat(ain2_current, 4, 20, 0, 250);
  Serial.printf("Pressure = %.1f bar\n", pressure);
  //Current between 4-20 mA is distributed as pressure between 0-250 bar.
  //12 mA should be 125 bar equivalent

  Serial.println();
  delay(1000);
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}