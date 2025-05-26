#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include <MFRC522.h>

#define Own_ID_DEF 1
#define Begin_Key_DEF 10
#define Message_Kind_DEF 1
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
int UserInput = 0;
bool RestartLoop = false;

int MaxCargo = 5;
int Car = 0;

int Buf_Dest_ID = 0;
int Buf_Message_Kind = 0;
int Buf_F_Station = 0;
int Buf_F_Amount = 0;
int Buf_S_Station = 0;
int Buf_S_Amount = 0;

int ValidateInput(String Prompt, int MinimumValue, int MaximumValue) {
  String Input;
  int Result = 0;
  bool Valid = false;

  while (!Valid) {
    Serial.println(Prompt);
    while (Serial.available() == 0) {}

    Input = Serial.readStringUntil('\n');
    Input.trim();

    if (Input == "q" || Input == "Q") {
      RestartLoop = true;
      return -1;
    }
    bool IsNumber = true;
    for (unsigned int i = 0; i < Input.length(); i++) {
      if (!isDigit(Input[i])) {
        IsNumber = false;
        break;
      }
    }

    if (!IsNumber) {
      Serial.println("Ongeldige waarde ingevoerd: voer een getal in!");
      continue;
    }

    Result = Input.toInt();
    if (Result < MinimumValue || Result > MaximumValue) {
      Serial.print("Getal is niet van goede waarde: voer getal in tussen ");
      Serial.print(MinimumValue);
      Serial.print(" en ");
      Serial.print(MaximumValue);
      Serial.println(".");
    } else {
      Valid = true;
    }
  }
  return Result;
}


void CarChoice(){
  UserInput = ValidateInput("Vrachtwagen 1 of 2?", 1, 2);
  if (RestartLoop) return;
  Car = UserInput;
  Serial.print("Je hebt ingevuld:");
  Serial.println(Car);
  if (Car == 1) Buf_Dest_ID = 3;
  if (Car == 2) Buf_Dest_ID = 4;
}

void FirstStation(){
  bool error = false;
  UserInput = ValidateInput("Welk station?: Pepperoni (1) Deeg(2), tomaat(3), kaas(4) of kip(5)? ", 1, 5);
  if (RestartLoop) return;
  Buf_F_Station = UserInput;
  Serial.print("Je hebt ingevuld: ");
  Serial.println(Buf_F_Station);
}

void FirstAmount(){
  UserInput = ValidateInput("Hoeveel wil je ophalen?: 1, 2, 3, 4 of 5", 1, 5);
  if (RestartLoop) return;
  Buf_F_Amount = UserInput;
  Serial.print("Je hebt ingevuld: ");
  Serial.println(Buf_F_Amount);
}

void SecondStationAndAmount(){
  int Choice = 0;
  if(MaxCargo - Buf_F_Amount > 0) {
    switch (Buf_F_Station) {
      case 1:
        UserInput = ValidateInput("Welk tweede station?: Deeg(2), tomaat(3), kaas(4) of kip(5)? ", 2, 5);
        if (RestartLoop) return;
        Buf_S_Station = UserInput;
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_S_Station);
        SecondAmount();
      break;
      case 2:
        UserInput = ValidateInput("Welk tweede station?: Tomaat(3), kaas(4) of kip(5)? ", 3, 5);
        if (RestartLoop) return;
        Buf_S_Station = UserInput;
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_S_Station);
        SecondAmount();
      break;
      case 3:
        UserInput = ValidateInput("Welk tweede station?: Kaas(4) of kip(5)? ", 4, 5);
        if (RestartLoop) return;
        Buf_S_Station = UserInput;
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_S_Station);
        SecondAmount();
      break;
      case 4:
        UserInput = ValidateInput("Wil je naar station kip?: Ja(1) of Nee(2)", 1, 2);
        if (RestartLoop) return;
        Choice = UserInput;
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
      UserInput = ValidateInput("Hoeveel wil je ophalen?: 1, 2, 3 of 4?", 1, 4);
      if (RestartLoop) return;
      break;
    case 2:
      UserInput = ValidateInput("Hoeveel wil je ophalen?: 1, 2 of 3?", 1, 3);
      if (RestartLoop) return;
      break;
    case 3:
      UserInput = ValidateInput("Hoeveel wil je ophalen?: 1 of 2?", 1, 2);
      if (RestartLoop) return;
      break;
    case 4:
      Serial.println("1 goed wordt opgehaald");
      Buf_S_Amount = 1;
      return;
  }
  Buf_S_Amount = UserInput;
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
  delay(3000);
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
  RestartLoop = false;

  Serial.println("================================");
  Serial.println("Industrie vrachtwagen controle: ");
  Serial.println("*Druk op q om het menu te herstellen*");

  CarChoice(); if (RestartLoop) return;
  FirstStation(); if (RestartLoop) return;
  FirstAmount(); if (RestartLoop) return;
  SecondStationAndAmount(); if (RestartLoop) return;

  ShowPlanner();
  MakeMessage(Begin_Key_DEF,
              Buf_Dest_ID,
              Own_ID_DEF,
              Message_Kind_DEF,
              Buf_F_Station,
              Buf_F_Amount,
              Buf_S_Station,
              Buf_S_Amount,
              End_Key_DEF);
  SendMessage();
}
