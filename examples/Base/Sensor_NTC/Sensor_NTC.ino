#include "WiSEN.h"

// resistance at 25 degrees C
#define THERMISTORNOMINAL 10000       
// The beta coefficient of the thermistor (depend on ntc sensor, usually 3000-4000)
#define BCOEFFICIENT 3435
// The number of average
#define AVERAGE 10

void setup()
{  
  // Initial WiSEN Hardware
  WiSEN.Init();
  
  Serial.begin(115200);
  delay(1000);

  Serial.println("+++ Demo NTC Sensor Input +++\n");
  
  Serial.println(">>> Please Config Jumper for AN1 to NTC");
  Serial.println("*** J5 - OPEN");
  Serial.println("*** J6 - OPEN");
  Serial.println("*** J7 - CLOSE");
  Serial.println("*** J8 - CLOSE");
  Serial.println(">>> Please Config Jumper for AN2 to NTC");
  Serial.println("*** J1 - OPEN");
  Serial.println("*** J2 - OPEN");
  Serial.println("*** J3 - CLOSE");
  Serial.println("*** J4 - CLOSE");
}

void loop()
{
  uint16_t adc;
  uint32_t R;
  float VR, temp;

  // Turn ON Sensor Power Supply
  WiSEN.SensorPower(ON);
  
  // Read ADC on AN1
  Serial.println("NTC => <AN1>");
  adc = WiSEN.ReadADC(AN1, AVERAGE);
  Serial.printf("ADC = %u\n", adc);
  R = WiSEN.GetResNTC(AN1, AVERAGE);
  Serial.printf("Res: %u ohm\n", R);
  VR = WiSEN.GetVoltNTC(AN1, AVERAGE);
  Serial.printf("Volt = %.3f\n",VR);
  temp = WiSEN.GetTempNTC(AN1, THERMISTORNOMINAL, BCOEFFICIENT);
  Serial.printf("Temperature = %.1f °C\n", temp);
  Serial.println();
  
  // Read ADC on AN2
  Serial.println("NTC => <AN2>");
  adc = WiSEN.ReadADC(AN2, AVERAGE);
  Serial.printf("ADC = %u\n", adc);
  R = WiSEN.GetResNTC(AN2, AVERAGE);
  Serial.printf("Res: %u ohm\n", R);
  VR = WiSEN.GetVoltNTC(AN2, AVERAGE);
  Serial.printf("Volt = %.3f\n",VR);
  temp = WiSEN.GetTempNTC(AN2, THERMISTORNOMINAL, BCOEFFICIENT);
  Serial.printf("Temperature = %.1f °C\n", temp);
  Serial.println();

  // Turn OFF Sensor Power Supply
  WiSEN.SensorPower(OFF);
  delay(5000);
}
