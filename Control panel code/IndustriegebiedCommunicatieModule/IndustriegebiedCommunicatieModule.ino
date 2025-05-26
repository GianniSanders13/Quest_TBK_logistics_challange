#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include <MFRC522.h>

#define Own_ID_DEF 1
#define Begin_Key_DEF 10
#define End_Key_DEF 5

uint8_t Mac1[] = {0x74, 0x4D, 0xBD, 0x77, 0x08, 0x1C};

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
Message OutgoingMessage;
bool NewMessageReceived = false;
String UserInput = "";
int MaxCargo = 5;
int Car = 0;

int Buf_Dest_ID = 0;
int Buf_Message_Kind = 0;
int Buf_F_Station = 0;
int Buf_F_Amount = 0;
int Buf_S_Station = 0;
int Buf_S_Amount = 0;

void CarChoice(){
  Serial.println("Vrachtwage 1 of 2?");
  while (Serial.available() == 0) {}
  UserInput = Serial.readStringUntil('\n');
  Car = UserInput.toInt();
  Serial.print("Je hebt ingevuld:");
  Serial.println(Car);
  if (Car == 1) Buf_Dest_ID = 3;
  if (Car == 2) Buf_Dest_ID = 4;
}

void FirstStation(){
  bool error = false;
  Serial.println("Welk station?: Pepperoni (1) Deeg(2), tomaat(3), kaas(4) of kip(5)? ");
  while (Serial.available() == 0) {}
  UserInput = Serial.readStringUntil('\n');
  Buf_F_Station = UserInput.toInt();
  Serial.print("Je hebt ingevuld: ");
  Serial.println(Buf_F_Station);
}

void FirstAmount(){
  Serial.println("Hoeveel wil je ophalen?: 1, 2, 3, 4 of 5");
  while (Serial.available() == 0) {}
  UserInput = Serial.readStringUntil('\n');
  Buf_F_Amount = UserInput.toInt();
  Serial.print("Je hebt ingevuld: ");
  Serial.println(Buf_F_Amount);
}

void SecondStationAndAmount(){
  int Choice = 0;
  if(MaxCargo - Buf_F_Amount > 0) {
    switch (Buf_F_Station) {
      case 1:
        Serial.println("Welk tweede station?: Deeg(2), tomaat(3), kaas(4) of kip(5)? ");
        while (Serial.available() == 0) {}
        UserInput = Serial.readStringUntil('\n');
        Buf_S_Station = UserInput.toInt();
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_S_Station);
        SecondAmount();
      break;
      case 2:
        Serial.println("Welk tweede station?: Tomaat(3), kaas(4) of kip(5)? ");
        while (Serial.available() == 0) {}
        UserInput = Serial.readStringUntil('\n');
        Buf_S_Station = UserInput.toInt();
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_S_Station);
        SecondAmount();
      break;
      case 3:
        Serial.println("Welk tweede station?: Kaas(4) of kip(5)? ");
        while (Serial.available() == 0) {}
        UserInput = Serial.readStringUntil('\n');
        Buf_S_Station = UserInput.toInt();
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_S_Station);
        SecondAmount();
      break;
      case 4:
        Serial.println("Wil je naar station kip?: Ja(1) of Nee(2)");
        while (Serial.available() == 0) {}
        UserInput = Serial.readStringUntil('\n');
        Choice = UserInput.toInt();
        Serial.print("Je hebt ingevuld: ");
        if(Choice){
          Buf_S_Station = 5;
        } else{
          Buf_S_Station = 0;
        }
        Serial.println(Buf_S_Station);
        SecondAmount();
      break;
      case 5:
        Serial.println("Geen volgende station mogelijk!");
        Buf_S_Station = 0;
        Buf_S_Amount = 0;
      break;
    }
  } else {
    Serial.println("Geen volgende station mogelijk!");
    Buf_S_Station = 0;
    Buf_S_Amount = 0;
  }
}

void SecondAmount(){
  switch (Buf_F_Amount) {
    case 1:
      Serial.println("Hoeveel wil je ophalen?: 1, 2, 3 of 4?");
      break;
    case 2:
      Serial.println("Hoeveel wil je ophalen?: 1, 2 of 3?");
      break;
    case 3:
      Serial.println("Hoeveel wil je ophalen?: 1 of 2?");
      break;
    case 4:
      Serial.println("1 goed wordt opgehaald");
      Buf_S_Amount = 1;
      return;
  }
  while (Serial.available() == 0) {}
  UserInput = Serial.readStringUntil('\n');
  Buf_S_Amount = UserInput.toInt();
  Serial.print("Je hebt ingevuld: ");
  Serial.println(Buf_S_Amount);
}

void ShowPlanner(){
  Serial.println("De volgende informatie is ingevuld:");
  Serial.print("Auto: "); Serial.println(Car);
  Serial.print("Eerste station: "); Serial.println(Buf_F_Station);
  Serial.print("Hoeveelheid: "); Serial.println(Buf_F_Amount);
  Serial.print("Tweede station: "); Serial.println(Buf_S_Station);
  Serial.print("Hoeveelheid: "); Serial.println(Buf_S_Amount);
}

void SendMessage() {
  esp_err_t Result = esp_now_send(Mac1, (uint8_t *) &OutgoingMessage, sizeof(OutgoingMessage));
  if (Result == ESP_OK) {
    Serial.println("Bericht succesvol verzonden");
  } else {
    Serial.print("Verzendfout: "); Serial.println(Result);
  }
}

void OnDataReceive(const uint8_t * MacAdress, const uint8_t *IncomingBytes, int Length) {
  char MacAdressString[18];
  snprintf(MacAdressString, sizeof(MacAdressString), "%02X:%02X:%02X:%02X:%02X:%02X",
           MacAdress[0], MacAdress[1], MacAdress[2], MacAdress[3], MacAdress[4], MacAdress[5]);
  Serial.println("Iets ontvangen");
  NewMessageReceived = true;


 memcpy(&IncomingMessage, IncomingBytes, sizeof(IncomingMessage));

  Serial.println(IncomingMessage.Begin_Key);
  Serial.println(IncomingMessage.Dest_ID);
  Serial.println(IncomingMessage.Source_ID);
  Serial.println(IncomingMessage.Message_Kind);
  Serial.print("Data1: "); Serial.println(IncomingMessage.Data1);
  Serial.print("Data2: "); Serial.println(IncomingMessage.Data2);
  Serial.print("Data3: "); Serial.println(IncomingMessage.Data3);
  Serial.print("Data4: "); Serial.println(IncomingMessage.Data4);
  Serial.println(IncomingMessage.End_Key);
}

void MakeMessage(int b, int d, int s, int m, int fs, int fa, int ss, int sa, int e){
  OutgoingMessage.Begin_Key = b;
  OutgoingMessage.Dest_ID = d;
  OutgoingMessage.Source_ID = s;
  OutgoingMessage.Message_Kind = m;
  OutgoingMessage.Data1 = fs;
  OutgoingMessage.Data2 = fa;
  OutgoingMessage.Data3 = ss;
  OutgoingMessage.Data4 = sa;
  OutgoingMessage.End_Key = e;
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init mislukt");
    return;
  }
  esp_now_register_recv_cb(OnDataReceive);

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, Mac1, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Peer toevoegen mislukt");
    return;
  }
}

void loop() {
  Serial.println("================================");
  Serial.println("Industrie vrachtwagen controle: ");
  CarChoice();
  FirstStation();
  FirstAmount();
  SecondStationAndAmount();
  ShowPlanner();
  MakeMessage(Begin_Key_DEF,
              Buf_Dest_ID,
              Own_ID_DEF,
              1,
              Buf_F_Station,
              Buf_F_Amount,
              Buf_S_Station,
              Buf_S_Amount,
              End_Key_DEF);
  SendMessage();
}
