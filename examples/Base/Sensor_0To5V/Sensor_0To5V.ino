#include "WiSEN.h"

// The number of average
#define AVERAGE 10

void setup()
{  
  // Initial WiSEN Hardware
  WiSEN.Init();
  
  Serial.begin(115200);
  delay(1000);

  Serial.println("+++ Demo 0-5V Sensor Input +++\n");
  
  Serial.println(">>> Please Config Jumper for AN1 to 0-5V");
  Serial.println("*** J5 - OPEN");
  Serial.println("*** J6 - OPEN");
  Serial.println("*** J7 - CLOSE");
  Serial.println("*** J8 - OPEN");
  Serial.println(">>> Please Config Jumper for AN2 to 0-5V");
  Serial.println("*** J1 - OPEN");
  Serial.println("*** J2 - OPEN");
  Serial.println("*** J3 - CLOSE");
  Serial.println("*** J4 - OPEN");
  
  Serial.println("Turn ON Sensor Power Supply.");
  WiSEN.SensorPower(ON);
}


void loop()
{
  // Connect Sensor to AN1
  Serial.println("Sensor (0-5V) => <AN1>");
  Serial.printf("ADC = %u\n", analogRead(AN1));
  // Get Analog Input A1 (0-5V mode) in average 10
  float ain1 = WiSEN.Get0to5V(AN1, AVERAGE);
  Serial.printf("AN1 (0-5V) = %.3f Volt\n", ain1);

  // Connect Sensor to AN2
  Serial.println("Sensor (0-5V) => <AN2>");
  Serial.printf("ADC = %u\n", analogRead(AN2));
  // Get Analog Input A2 (0-5V mode) in average 10
  float ain2 = WiSEN.Get0to5V(AN2, AVERAGE);
  Serial.printf("AN1 (0-5V) = %.3f Volt\n", ain2);
  
  Serial.println();
  delay(1000);
}
