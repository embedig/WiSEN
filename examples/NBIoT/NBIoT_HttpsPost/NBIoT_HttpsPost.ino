#include "WiSEN.h"

#define RESPONSE_TIMEOUT  20000 // http response timeout in ms
#define INTERVAL_TIME     15000 // data send interval time in ms
#define STATUS_CONN        5000 // time for status check in ms

unsigned long update_t, status_t;
unsigned long count = 0;
int loss_conn = 0;

void setup() {

  WiSEN.Init();
  
  Serial.begin(115200);
  delay(3000);
  Serial.println("NBIoT Test!");
  
  Serial.println("NBIoT HTTP GET Test!");
  
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
  
  loss_conn = 0;
  
  // Set 512k http buffer in External RAM
  WiSEN.NBIoT.SetHttpBuffer(524288);
  
  update_t = millis();
  status_t = update_t;

  Serial.println("Turn ON Sensor Power Supply.");
  WiSEN.SensorPower(ON);
}

void loop() {
  if( millis()-update_t >= INTERVAL_TIME ) {
    update_t = millis();
    
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

    String HOST = "postman-echo.com";
    String PATH = "/post";
    String URI = HOST + PATH;

    Serial.print("POST => https://"); Serial.println(URI);

    unsigned long start_t = millis();
    
    // ***** HTTPS ****
    // Https Get
    //WiSEN.NBIoT.httpsPOST(URI, DATA);
    // Https Get with port define
    //WiSEN.NBIoT.httpsPOST(443, URI, DATA);

    // Https Get Custom header
    String HEADER;
    HEADER = "POST " + PATH + " HTTP/1.1\r\n";
    HEADER += "Host: " + HOST + "\r\n"; 
    HEADER += "Content-Type: application/json\r\n";
    HEADER += "Content-Length: "+String(DATA.length())+"\r\n";
    WiSEN.NBIoT.HTTPS(443, URI, HEADER, DATA);
    
    Serial.printf("Request time %d ms\n", millis()-start_t);
  
    start_t = millis();
    while(millis()-start_t <= RESPONSE_TIMEOUT) {
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
