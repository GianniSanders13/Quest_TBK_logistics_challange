#include <esp_now.h>
#include <WiFi.h>

#define Own_ID_DEF 3  // Pas dit aan naar 4 als dit vrachtwagen 2 is
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
  uint8_t End_Key;
} Message;

Message IncomingMessage;

void OnDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&IncomingMessage, incomingData, sizeof(IncomingMessage));
  Serial.println("Bericht ontvangen:");

  // Check op geldigheid (optioneel)
  if (IncomingMessage.Begin_Key == Begin_Key_DEF && IncomingMessage.End_Key == End_Key_DEF) {
    Serial.printf("Van: %d -> Voor: %d\n", IncomingMessage.Source_ID, IncomingMessage.Dest_ID);
    Serial.printf("Type: %d\n", IncomingMessage.Message_Kind);
    Serial.printf("Eerste station: %d, Hoeveel: %d\n", IncomingMessage.Data1, IncomingMessage.Data2);
    Serial.printf("Tweede station: %d, Hoeveel: %d\n", IncomingMessage.Data3, IncomingMessage.Data4);
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
