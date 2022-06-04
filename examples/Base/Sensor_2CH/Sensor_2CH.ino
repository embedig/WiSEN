#include "WiSEN.h"

// The number of average
#define AVERAGE 10

void setup()
{  
  // Initial WiSEN Hardware
  WiSEN.Init();
  
  Serial.begin(115200);
  delay(200);

  Serial.println("+++ Demo 0-10V Sensor Input +++\n");
  
  Serial.println(">>> Please Config Jumper for AN1 to 0-20mA");
  Serial.println("*** J5 - CLOSE");
  Serial.println("*** J6 - CLOSE");
  Serial.println("*** J7 - OPEN");
  Serial.println("*** J8 - OPEN");
  Serial.println(">>> Please Config Jumper for AN2 to 0-10V");
  Serial.println("*** J1 - OPEN");
  Serial.println("*** J2 - CLOSE");
  Serial.println("*** J3 - OPEN");
  Serial.println("*** J4 - OPEN");
}


void loop()
{ 
  // ON LED Status
  WiSEN.LED(ON);

  // Turn ON Sensor Power Supply
  WiSEN.SensorPower(ON);
  
  // Connect Sensor to AN1
  Serial.println("Sensor (0-20mA) => <AN1>");
  Serial.printf("ADC = %u\n", WiSEN.ReadADC(AN1, AVERAGE));
  // Get Analog Input A1 (0-20mA mode) in average 10
  float ain1 = WiSEN.Get0to20mA(AN1, AVERAGE);
  Serial.printf("Sensor = %.2f mA\n", ain1);

  // Connect Sensor to AN2
  Serial.println("Sensor (0-10V) => <AN2>");
  Serial.printf("ADC = %u\n", WiSEN.ReadADC(AN2, AVERAGE));
  // Get Analog Input A2 (0-10V mode) in average 10 
  float ain2 = WiSEN.Get0to10V(AN2, AVERAGE);
  Serial.printf("Sensor = %.2f Volt\n", ain2);

  Serial.println();

  //Turn OFF Sensor Power Supply.
  WiSEN.SensorPower(OFF);
  // Wait 2s befor OFF LED status
  delay(2000);
  // OFF LED Status
  WiSEN.LED(OFF);
  delay(5000);
}
