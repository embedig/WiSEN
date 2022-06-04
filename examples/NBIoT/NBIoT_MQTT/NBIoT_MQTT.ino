#include "WiSEN.h"

#define INTERVAL_TIME     10000 // data send interval time in ms
#define STATUS_CONN        5000 // time for status check in ms

#define MQTT_BROKER  "broker.hivemq.com"
#define MQTT_PORT    1883
#define MQTT_USER    ""
#define MQTT_PASSWD  ""

unsigned long update_t, status_t;
unsigned long count = 0;
int loss_conn = 0;

void mqtt_receice(String topic, String msg, int len) {
  Serial.println("++++++ MQTT SUB ++++++");
  Serial.print("<Topic> ");
  Serial.println(topic);
  Serial.print("<Message> ");
  Serial.println(msg);
  Serial.print("<LEN> ");
  Serial.println(len);
  Serial.println("++++++++++++++++++++++\n");
}

void setup() {

  WiSEN.Init();
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("NBIoT MQTT Demo!");
  
  Serial.printf("Total Ext.RAM: %u Byte\n", ESP.getFreePsram());
  
  Serial.println("Initialize NBIoT Module...");
  // Define GPIO for NBIoT Module
  // UART_NUM_0  -> Using Hardware UART0 (UART_NUM_1 for UART1)
  // D10(GPIO21) -> NBIoT_RXD
  // D11(GPIO17) -> NBIoT_TXD
  // D12(GPIO16) -> NBIoT_WAKE
  // D13(GPIO15) -> NBIoT_RESET
  WiSEN.NBIoT.begin(UART_NUM_0, D10, D11, D12, D13);

  Serial.println("Setup MQTT Handler...");
  WiSEN.NBIoT.setMQTTSUBHandler(mqtt_receice);

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

  // Turn-on power supply for sensor and rs485 communication
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

    // Save json data in external ram
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

    // Send data if status connected
    if(WiSEN.NBIoT.mqttStatus()) WiSEN.NBIoT.mqttPub("wisen-topic", DATA, 1, 0);
    
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

      if(WiSEN.NBIoT.mqttStatus() == 0 && IP_ADDRESS != "") {
        Serial.println("Connecting to MQTT server...");
        WiSEN.NBIoT.mqttConnect(MQTT_BROKER, MQTT_USER, MQTT_PASSWD, MQTT_PORT, 60);
        Serial.println("Subscribe to <wisen-topic>");
        // mqtt subscribe to same topic
        WiSEN.NBIoT.mqttSub("wisen-topic", 1);
      }
    }
  }
}
