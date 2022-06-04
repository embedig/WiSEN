#include <Arduino.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include "driver/adc.h"
#include "driver/uart.h"
#include "esp_adc_cal.h"

#define UART_BUF_SIZE 1024
#define MQTT_BUF_SIZE 2048
#define HTTP_BUF_SIZE 16384 // 16kbyte

#define TAG "WiSEN"

#define D0    37
#define D1    38
#define D2    39
#define D3    43
#define D4    44

#define D5    1  // RS485_RXD
#define D6    2  // RS485_TXD
#define D7    3
#define D8    4
#define D9    5

#define D10   21 // RXD
#define D11   17 // TXD
#define D12   16 // WAKE
#define D13   15 // RESET
#define D14   12  

#define D15    11 // TXD
#define D16    9  // RXD
#define D17    8
#define D18    7
#define D19    6

#define D20    35
#define D21    36

#define AN1   ADC2_CHANNEL_3 //GPIO14
#define AN2   ADC2_CHANNEL_2 //GPIO13

#define RS485_RXD   1
#define RS485_TXD   2

#define LED_PIN     34
#define SENSOR_PWR  33

#define ON    1
#define OFF   0

#define NETWORK_REGISTERING      0
#define NETWORK_LINK_CONNECTING  1
#define NETWORK_LINK_CONNECTED   2

#define FILE_APPEND 1

#ifndef WiSEN_DEBUG
// Debug output set to 0 to disable app debug output
#define WiSEN_DEBUG 1
#endif

#if WiSEN_DEBUG > 0
#define LOG_LIB(tag, ...)           \
  do                                \
  {                                 \
    if (tag)                        \
      Serial.printf("<%s> ", tag);  \
    Serial.printf(__VA_ARGS__);     \
  } while (0)
#else
#define LOG_LIB(...)
#endif

struct ExtRamAllocator {
  void* allocate(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }
  void deallocate(void* pointer) {
    heap_caps_free(pointer);
  }
  void* reallocate(void* ptr, size_t new_size) {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};

using ExtRamJsonDocument = BasicJsonDocument<ExtRamAllocator>;

class WiSENN21 {
  protected:
    static void WiSENN21_ISR_ROUTINE(void *pvParameters);
    static void loop_task(void *pvParameter);
    String expect_rx_str(String exp_str);
    bool expect_str(String exp_str);
    
  public:
    bool http_received;
    bool ftp_received;
    bool mqtt_received;

    char lastdata[32];
    
    struct WiSENN21_t {
      int buffer_length;
      char buffer[UART_BUF_SIZE];
      char mqtt[MQTT_BUF_SIZE];
    };
  
    WiSENN21_t *data;

    char *http_buffer;
    char *ftp_buffer;
    unsigned long http_count;
    unsigned long ftp_count;

    typedef void (*CallbackFunction) (String, String, int);
    typedef void (*HttpCallbackFunction) (String, String, int, unsigned long);
    typedef void (*FtpCallbackFunction) (String, unsigned long);

    CallbackFunction mqtt_cb = NULL;
    HttpCallbackFunction http_cb = NULL;
    FtpCallbackFunction ftp_cb = NULL;

    bool Reset();
    void begin(int UART_NUM, int RXD_PIN, int TXD_PIN, byte WAKE_PIN, byte RESET_PIN);
    bool AT(String input);
    bool AT(String input, unsigned long timeout);
    String AT(String input, String exp_str, unsigned long timeout);
    String AT(String input, String exp_start, String exp_end, unsigned long timeout);
    void InitModem();
    int NetworkConnect();
    bool IsNetworkRegistered();
    bool IsNetworkLinkConnected();
    int GetRSSI();
    String GetIMSI();
    String GetIMEI();
    String GetIPAddr();
    bool SyncDateTime();
    bool TimeSynced();
    bool IsModuleReset();

    // MQTT
    void setMQTTSUBHandler(CallbackFunction f);
    void mqttConnect(String server, String user, String pwd, int port, int keep_alive);
    void mqttConnect(String server, int port, int keep_alive);
    void mqttConnect(String server, int port);
    void mqttSub(String topic, int qos);
    void mqttUnSub(String topic);
    void mqttPub(String topic, String msg, byte qos, byte retained);
    void mqttDisconnect();
    byte mqttStatus();
    // HTTP
    void httpGET(String url);
    void httpGET(int port, String url);
    void httpsGET(String url);
    void httpsGET(int port, String url);
    void httpPOST(String url, String postdata);
    void httpPOST(int port, String url, String postdata);
    void httpsPOST(String url, String postdata);
    void httpsPOST(int port, String url, String postdata);
    void HTTP(int port, String url, String header);
    void HTTP(int port, String url, String header, String postdata);
    void HTTPS(int port, String url, String header);
    void HTTPS(int port, String url, String header, String postdata);
    void setHTTPHandler(HttpCallbackFunction f);
    void SetHttpBuffer(unsigned long size);
    bool httpAvailable();
    int httpCode();
    String httpHeader();
    String httpBody();
    uint32_t httpContentLen();
    // FTP
    void setFTPHandler(FtpCallbackFunction f);
    void SetFtpBuffer(unsigned long size);
    String ftpLogin(String server, int port, String user, String pwd);
    String ftpLogout();
    bool ftpStatus();
    uint32_t ftpFileSize(String filename);
    bool ftpFileRead(String filename, unsigned long offset, int len); // len: max 8192
    bool ftpFileWrite(String filename, String input, int append);
    bool ftpFileWrite(String filename, String input);
    bool ftpFileDelete(String filename);
    bool ftpAvailable();
    String ftpContent();

  private:
    byte RESET;
    byte WAKE;
    int UART;
    QueueHandle_t UART_queue;
    int data_length;
    int mqtt_length;
    unsigned long http_length;
    unsigned long ftp_length;
    bool time_synced;
    bool data_ready;
    bool http_ready;
    bool mqtt_ready;
    bool ftp_ready;
    bool mqtt_subscribe;
    bool http_recv;
    bool ftp_recv;
    bool module_reset;
    int module_error;
    unsigned long body_length;
    int http_respcode;
    String http_header;
    String http_body;
    String ftp_content;
};

class WiSENAPI {
  public:
    class WiSENN21 NBIoT;

    void Init();
    void SensorPower(bool pwr);
    void LED(bool st);
    String deviceId();
    String deviceMac();
    uint16_t ReadADC(byte AIN, byte average);
    float ReadADCVolt(byte AIN, byte average);
    float Get0to5V(byte AIN, byte average);
    float Get0to10V(byte AIN, byte average);
    float Get0to20mA(byte AIN, byte average);
    float GetTempNTC(byte AIN, uint32_t RO, int beta);
    uint32_t GetResNTC(byte AIN, byte average);
    float GetVoltNTC(byte AIN, byte average);
    String GetDateTime(String format);
};
extern WiSENAPI WiSEN;
