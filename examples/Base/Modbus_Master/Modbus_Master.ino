#include "WiSEN.h"

#include <SoftwareSerial.h> // http://librarymanager/All#EspSoftwareSerial
#include <ModbusMaster.h>   // http://librarymanager/All#ModbusMaster

SoftwareSerial RS485;
ModbusMaster SX1;

// https://www.meath-co.com/meter/product_detail.php?id=31&catID=14

void setup()
{
  // Initial WiSEN Hardware
  WiSEN.Init();
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("+++ Demo Modbus Mitsubishi Meter SX1 +++\n");
  
  RS485.begin(1200, SWSERIAL_8E1, RS485_RXD, RS485_TXD, false);
  if (!RS485) { // If the object did not initialize, then its configuration is invalid
    Serial.println("Invalid SoftwareSerial pin configuration, check config"); 
    while (1) { // Don't continue with invalid configuration
      delay (1000);
    }
  }

  Serial.println("Turn ON Sensor Power Supply.");
  WiSEN.SensorPower(ON);
}

void loop()
{
  // Modbus slave ID 177
  SX1.begin(177, RS485);
  
  // Read 16 holding registers
  uint8_t result = SX1.readHoldingRegisters(0x64, 16);
  if (result == SX1.ku8MBSuccess) {
    uint32_t sn = SX1.getResponseBuffer(0)<<16 | SX1.getResponseBuffer(1); // Address 0x64
    Serial.printf("Serial No: %u\n", sn);
    
    float V = SX1.getResponseBuffer(2)/100.0;   // Address 0x66
    Serial.printf("Line Voltage: %.2f Volt\n", V);

    float Freq = SX1.getResponseBuffer(5)/10.0; // Address 0x69
    Serial.printf("Line Frequency: %.1f Hz\n", Freq);

    float I = SX1.getResponseBuffer(12)/100.0;  // Address 0x70
    Serial.printf("Line Current: %.2f Amp\n", I);
    
    uint16_t P = SX1.getResponseBuffer(15);     // Address 0x73
    Serial.printf("Active Power: %d Watt\n", P);

    uint32_t wh = SX1.getResponseBuffer(10)<<16 | SX1.getResponseBuffer(11); // Address 0x6E
    Serial.printf("Active Energy: %.3f kWh\n", wh/1000.0);
  }
  Serial.println();
  delay(1000);
}
