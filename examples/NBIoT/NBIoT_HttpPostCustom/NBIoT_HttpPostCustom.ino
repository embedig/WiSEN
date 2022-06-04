#include "WiSEN.h"

#define RESPONSE_TIMEOUT  20000 // http response timeout in ms
#define INTERVAL_TIME     15000 // data send interval time in ms
#define STATUS_CONN        5000 // time for status check in ms

unsigned long update_t, status_t;
unsigned long count = 0;
int loss_conn = 0;

void setup() {

  // Init WiSEN module
  WiSEN.Init();
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\nNBIoT HTTP POST with Header Demo!");
  
  Serial.printf("Total Ext.RAM: %u Byte\n", ESP.getFreePsram());

  Serial.println("Initialize NBIoT Module...");
  // Define GPIO for NBIoT Module
  // UART_NUM_0  -> Using Hardware UART0 (UART_NUM_1 for UART1)
  // D10(GPIO21) -> NBIoT_RXD
  // D11(GPIO17) -> NBIoT_TXD
  // D12(GPIO16) -> NBIoT_WAKE
  // D13(GPIO15) -> NBIoT_RESET
  WiSEN.NBIoT.begin(UART_NUM_0, D10, D11, D12, D13);

  Serial.println("Set HTTP buffer in external ram 512-kbyte.");
  // Set 512k http buffer in External RAM
  WiSEN.NBIoT.SetHttpBuffer(524288);

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
  
  // Check free external ram
  Serial.printf("Free Ext.RAM: %u Byte\n", ESP.getFreePsram());
  
  update_t = millis();
  status_t = update_t;

  //  Turn-on power supply for sensor and rs485 communication
  Serial.println("Turn ON Sensor Power Supply.");
  WiSEN.SensorPower(ON);
}

void loop() {
  if( millis()-update_t >= INTERVAL_TIME ) {
    update_t = millis();
    
    WiSEN.LED(ON);

    // prepare payload data for example
    int id = count++;
    String hwid = WiSEN.deviceId();
    String imsi = WiSEN.NBIoT.GetIMSI();
    String ip = WiSEN.NBIoT.GetIPAddr();
    int rssi = WiSEN.NBIoT.GetRSSI();
    // random data for example
    float temp = random(3000, 3050) / 100.0;
    float humidity = random(700, 800) / 10.0;

    ExtRamJsonDocument doc(1024);
    doc["id"] = id;
    doc["hwid"] = hwid;
    doc["imsi"] = imsi;
    doc["ip"] = ip;
    doc["rssi"] = rssi;
    doc["temp"] = temp;
    doc["humidity"] = humidity;

    String DATA;
    serializeJson(doc, DATA);

    unsigned long start_t = millis();
    String HOST = "postman-echo.com";
    String PATH = "/post";
    
    String URI = HOST + PATH;
    Serial.print("POST => http://"); Serial.println(URI);
    
    // ***** HTTP ****
    // Http Get Custom header
    String HEADER;
    HEADER  = "POST " + PATH + " HTTP/1.1\r\n";
    HEADER += "Host: " + HOST + "\r\n";
    HEADER += "Content-Type: application/json\r\n";
    HEADER += "Content-Length: "+String(DATA.length())+"\r\n";
    WiSEN.NBIoT.HTTP(80, URI, HEADER, DATA);
    
    Serial.printf("Request time %d ms\n", millis()-start_t);

    // Wait for response data ready
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
      
      int index_start = WiSEN.NBIoT.httpHeader().indexOf("Date: ");
      String val = WiSEN.NBIoT.httpHeader().substring(index_start+6);
      int index_end = val.indexOf("\r\n");
      val = val.substring(0,index_end);
      Serial.println("<Date>");
      Serial.println(val);

      if(WiSEN.NBIoT.httpContentLen() > 0) {
        Serial.println("<Body>");
        Serial.println(WiSEN.NBIoT.httpBody());
      }
      Serial.println("++++++ RESPONSE END ++++++\n");
    }
    else {
      Serial.println("+++++ RESPONSE Timeout +++++\n");
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
