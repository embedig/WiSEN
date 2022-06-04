#include "WiSEN.h"
#include <base64.h>
#include <esp_task_wdt.h>

// Example using https://demo.thingsboard.io/
// Just register and get Access Token

//30 seconds WDT
#define WDT_TIMEOUT       30

#define RESPONSE_TIMEOUT  15000
#define INTERVAL_TIME     30000
#define STATUS_CONN        5000

#define ACCESS_TOKEN      "l2TTB8Y6i5XTCaC4AHBR"

unsigned long update_t, status_t;
unsigned long count = 0;
int loss_conn = 0;

void setup() {

  WiSEN.Init();
  
  Serial.begin(115200);
  delay(3000);
  Serial.println("Thingboard NBIoT Test!");

  Serial.printf("Configuring WDT to %ds\n", WDT_TIMEOUT);
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch

  Serial.printf("WiSEN RAM: %u Byte\n", ESP.getFreePsram());
  
  Serial.println("Initialize NBIoT Module...");
  // Define GPIO for NBIoT Module
  // UART_NUM_0  -> Using Hardware UART0 (UART_NUM_1 for UART1)
  // D10(GPIO21) -> NBIoT_RXD
  // D11(GPIO17) -> NBIoT_TXD
  // D12(GPIO16) -> NBIoT_WAKE
  // D13(GPIO15) -> NBIoT_RESET
  WiSEN.NBIoT.begin(UART_NUM_0, D10, D11, D12, D13);

  Serial.println("Connecting to network...");
  int conn_status, conn_count = 0;
  while( (conn_status = WiSEN.NBIoT.NetworkConnect()) != NETWORK_LINK_CONNECTED ) {
    if(conn_status == NETWORK_REGISTERING) {
      Serial.print("-");
    }
    else if(conn_status == NETWORK_LINK_CONNECTING) {
      Serial.print("+");
    }
    conn_count++;
    delay(1000); // 1s
    if(conn_count >= 300) { // wait for connection for 5 minutes
      Serial.println("Reboot!");
      Serial.flush();
      esp_restart();
    }
  }
  Serial.println("CONNECTED");

  // Sync date time
  WiSEN.NBIoT.SyncDateTime();

  // Set 512k http buffer in External RAM
  WiSEN.NBIoT.SetHttpBuffer(524288);
  
  loss_conn = 0;
  update_t = millis();
  status_t = update_t;

  Serial.println("Turn ON Sensor Power Supply.");
  WiSEN.SensorPower(ON);
}

void loop() {
  // Resetting WDT
  esp_task_wdt_reset();
  
  if( millis()-update_t >= INTERVAL_TIME ) {
    // Save update time
    update_t = millis();
    
    // Turn-ON status LED
    WiSEN.LED(ON);

    // random data for test
    int id = count++;
    float temp = random(3000, 3050) / 100.0;
    float humidity = random(700, 800) / 10.0;

    ExtRamJsonDocument doc(1024);
    doc["id"] = id;
    doc["rssi"] = WiSEN.NBIoT.GetRSSI();
    doc["temp"] = temp;
    doc["humidity"] = humidity;

    String DATA;
    serializeJson(doc, DATA);

    unsigned long start_t = millis();
    String HOST = "demo.thingsboard.io";
    String PATH = "/api/v1/" + String(ACCESS_TOKEN) + "/telemetry";
    String URI = HOST + PATH;
    
    // ***** HTTPS ****
    String HEADER;
    HEADER  = "POST " + PATH + " HTTP/1.1\r\n";
    HEADER += "Host: " + HOST + "\r\n";
    HEADER += "Content-Type: application/json\r\n";
    HEADER += "Content-Length: "+String(DATA.length())+"\r\n";
    
    WiSEN.NBIoT.HTTPS(443, URI, HEADER, DATA);
    
    Serial.printf("Request time %d ms\n", millis()-start_t);
  
    start_t = millis();
    while(millis()-start_t <= RESPONSE_TIMEOUT) {
      esp_task_wdt_reset(); // reset wdt during waiting response
      if(WiSEN.NBIoT.httpAvailable()) break;
      yield();
    }
    Serial.printf("Response time %d ms\n\n", millis()-start_t);
    
    if(WiSEN.NBIoT.httpAvailable()) {
      Serial.printf("Response Code: %d\n", WiSEN.NBIoT.httpCode());
      Serial.printf("Content-Length: %u\n", WiSEN.NBIoT.httpContentLen());
      
      Serial.println("+++++ RESPONSE START +++++");
      Serial.println("<Header>");
      Serial.println(WiSEN.NBIoT.httpHeader());
      if(WiSEN.NBIoT.httpContentLen() > 0) {
        Serial.println("<Body>");
        Serial.println(WiSEN.NBIoT.httpBody());
      }
      Serial.println("++++++ RESPONSE END ++++++");
    }
    else {
      Serial.println("+++++ RESPONSE Timeout +++++");
      update_t = millis() - INTERVAL_TIME;
    }
    
    WiSEN.LED(OFF);
  }

  // Check status connection
  if( millis()-status_t >= STATUS_CONN ) {
    status_t = millis();

    int conn_status = WiSEN.NBIoT.NetworkConnect();
    if(conn_status != NETWORK_LINK_CONNECTED) {
      Serial.print("-");
      loss_conn++;
      if(loss_conn >= 60) {
        loss_conn = 0;
        Serial.println("Reset NBIoT Module!");
        WiSEN.NBIoT.Reset();
      }
    }
    else {      
      String DateTime = WiSEN.GetDateTime("[RTC] %A, %B %d %Y %H:%M:%S");
      Serial.print(DateTime);
    
      // Get IP Address
      String IP_ADDRESS = WiSEN.NBIoT.GetIPAddr();
      Serial.print(" [IP] "); Serial.print(IP_ADDRESS);

      // Get RSSI
      int rssi = WiSEN.NBIoT.GetRSSI();
      Serial.printf( " [RSSI] %d dBm\n", rssi );
    }
  }
}
