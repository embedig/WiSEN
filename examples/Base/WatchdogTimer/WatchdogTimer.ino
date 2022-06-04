#include <esp_task_wdt.h>

//5 seconds WDT
#define WDT_TIMEOUT 5

int i = 0;
int last = millis();

void setup() {
  Serial.begin(115200);
  delay(3000);
  
  Serial.println("Configuring WDT...");
  
  esp_task_wdt_init(WDT_TIMEOUT, true); //enable panic so ESP32 restarts
  esp_task_wdt_add(NULL); //add current thread to WDT watch
}

void loop() {
  // resetting WDT every 1s, 20 times only
  if (millis() - last >= 1000) {
    last = millis();
    if(i < 20) {
      i++;
      Serial.printf("Resetting WDT... %ds\n", i);
      esp_task_wdt_reset();
      if (i == 20) {
        Serial.println("Stopping WDT reset. CPU should reboot in 5s");
      }
    }
    else {
      Serial.println("Wait for reboot...");
    }
  }
}
