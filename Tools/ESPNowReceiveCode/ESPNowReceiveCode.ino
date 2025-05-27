#include <esp_now.h>
#include <WiFi.h>

#define Own_ID_DEF 9  // Pas dit aan naar 4 als dit vrachtwagen 2 is
#define Begin_Key_DEF 10
#define End_Key_DEF 5

typedef struct Message {
  uint8_t Begin_Key;
  uint8_t Dest_ID;
  uint8_t Source_ID;
  uint8_t Message_Kind;
  uint8_t Data1;
  uint8_t Data2;
  uint8_t Data3;
  uint8_t Data4;
  uint8_t Data5; 
  uint8_t End_Key;
} Message;

Message IncomingMessage;

void OnDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&IncomingMessage, incomingData, sizeof(IncomingMessage));
  Serial.println("Bericht ontvangen:");

  if (IncomingMessage.Begin_Key == Begin_Key_DEF && IncomingMessage.End_Key == End_Key_DEF) {
    Serial.print("Van: "); Serial.println(IncomingMessage.Source_ID);
    Serial.print("Type: "); Serial.println(IncomingMessage.Message_Kind);
    Serial.print("Data1: "); Serial.println(IncomingMessage.Data1);
    Serial.print("Data2: "); Serial.println(IncomingMessage.Data2);
    Serial.print("Data3: "); Serial.println(IncomingMessage.Data3);
    Serial.print("Data4: "); Serial.println(IncomingMessage.Data4);
    Serial.print("Data5: "); Serial.println(IncomingMessage.Data5);
  } else {
    Serial.println("Ongeldig berichtformaat");
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println("Ontvanger gestart");
  Serial.print("MAC adres ontvanger: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init mislukt");
    return;
  }

  esp_now_register_recv_cb(OnDataReceive);
}

void loop() {
  // Ontvanger doet niks actiefs in loop
}
