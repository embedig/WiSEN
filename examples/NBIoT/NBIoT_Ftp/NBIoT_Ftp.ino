#include "WiSEN.h"

#define RESPONSE_TIMEOUT  20000 // http response timeout in ms
#define INTERVAL_TIME     10000 // data send interval time in ms
#define STATUS_CONN        5000 // time for status check in ms

unsigned long update_t, status_t;
unsigned long count = 0;
int loss_conn = 0;

void setup() {
  // Init WiSEN module
  WiSEN.Init();
  
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("NBIoT FTP Demo!");
  
  Serial.printf("Total Ext.RAM: %u Byte\n", ESP.getFreePsram());
  
  Serial.println("Initialize NBIoT Module...");
  // Define GPIO for NBIoT Module
  // UART_NUM_0  -> Using Hardware UART0 (UART_NUM_1 for UART1)
  // D10(GPIO21) -> NBIoT_RXD
  // D11(GPIO17) -> NBIoT_TXD
  // D12(GPIO16) -> NBIoT_WAKE
  // D13(GPIO15) -> NBIoT_RESET
  WiSEN.NBIoT.begin(UART_NUM_0, D10, D11, D12, D13);

  Serial.println("Set FTP buffer in external ram 512-kbyte.");
  // Set 512k ftp buffer in External RAM
  WiSEN.NBIoT.SetFtpBuffer(524288);

  // Check free external ram
  Serial.printf("Free Ext.RAM: %u Byte\n", ESP.getFreePsram());

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

    unsigned long start_t = millis();

    // Ftp connecting
    if(WiSEN.NBIoT.ftpStatus() == 0) {
      //String re_str = WiSEN.NBIoT.ftpLogin("test.rebex.net", 21, "demo", "password");
      String re_str = WiSEN.NBIoT.ftpLogin("embedig.com", 21, "noom", "Noise1019");
      Serial.println(re_str);
      delay(2000);
    }

    if( WiSEN.NBIoT.ftpStatus() ) {

      // *********************** NOTE **************************
      // Write/Delete not working for "test.rebex.net" because of read only allow
      // for overwrite using WiSEN.NBIoT.ftpFileWrite(filename, write_data);

      // Write Data
      String write_data = WiSEN.GetDateTime("%y-%m-%d %H:%M:%S") + ",DATA1,DATA2\r\n";
      bool st = WiSEN.NBIoT.ftpFileWrite("readme.txt", write_data, FILE_APPEND);
      if(!st) {
        Serial.println("Error: Write Data!");
      }

      // Get File size
      unsigned long file_size = WiSEN.NBIoT.ftpFileSize("readme.txt");
      Serial.printf("File Size: %u\n", file_size);

      // Example to read last 10 line
      unsigned long start_read = 0;
      int read_length = file_size; // max 8192
      if( file_size > write_data.length()*10 ) {
        read_length = write_data.length()*10;
        start_read = file_size - read_length;
      }
      WiSEN.NBIoT.ftpFileRead("readme.txt", start_read, read_length);
      
      Serial.printf("Request time %d ms\n", millis()-start_t);

      // Wait for data response
      start_t = millis();
      unsigned long count = 0, timeout_t = start_t;
      while(millis()-timeout_t <= RESPONSE_TIMEOUT) {
        if(count != WiSEN.NBIoT.ftp_count) {
          timeout_t = millis();
          count = WiSEN.NBIoT.ftp_count;
          //Serial.println(count);
          Serial.print("#");
        }
        if(WiSEN.NBIoT.ftpAvailable()) break;
        yield();
      }
      Serial.printf("\nResponse time %d ms\n\n", millis()-start_t);
      
      if(WiSEN.NBIoT.ftpAvailable()) {      
        Serial.println("+++++ RESPONSE START +++++");
        Serial.println("<Content>");
        Serial.println(WiSEN.NBIoT.ftpContent());
        Serial.println("++++++ RESPONSE END ++++++\n");
      }
      else {
        Serial.println("+++++ RESPONSE Timeout +++++\n");
      }

      // Example to delete file if size > 1k
      /*if(file_size > 1024) {
        bool sta = WiSEN.NBIoT.ftpFileDelete("readme.txt");
        if(sta)
          Serial.println("Deleted File!");
        else
          Serial.println("Error: Delete File!");
      }*/
      
      // Ftp Logout
      //Serial.println(WiSEN.NBIoT.ftpLogout());
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
