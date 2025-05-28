#include <WiFi.h>
#include <esp_now.h>
#include <SPI.h>
#include <MFRC522.h>

#define Own_ID_DEF 1
#define Dest_ID_DEF 9
#define Begin_Key_DEF 10
#define Message_Kind_DEF 1
#define End_Key_DEF 5

#define Station1 1
#define Station2 2
#define Station3 3
#define Station4 6

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
  uint8_t Data5;
  uint8_t End_Key;
} Message;

Message OutgoingMessage;
bool NewMessageReceived = false;
int UserInput = 0;
bool RestartLoop = false;

int Buf_Route = 0;
int Buf_1e_Station = 0;
int Buf_2e_Station = 0;
int Buf_3e_Station = 0;
int Buf_4e_Station = 0;

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

void FillNextStation(int value) {
  if (Buf_1e_Station == 0) {
      Buf_1e_Station = value;
  } else if (Buf_2e_Station == 0) {
      Buf_2e_Station = value;
  } else if (Buf_3e_Station == 0) {
      Buf_3e_Station = value;
  } else if (Buf_4e_Station == 0) {
      Buf_4e_Station = value;
  } else {
      Serial.println("*Alles vol!*");
  }
}

void RouteChoice(){
  UserInput = ValidateInput("Welke route?: 1, 2, 3, 4, 5, 6, 7 of 8", 1, 8);
  if (RestartLoop) return;
  Buf_Route = UserInput;
  Serial.print("Je hebt ingevuld: "); Serial.println(Buf_Route);
}

void StationChoice(int stationNumber, int stationValue) {
    String prompt = "Wil je naar station " + String(stationNumber) + "?: Ja(1) of Nee(2)";
    UserInput = ValidateInput(prompt, 1, 2);
    if (RestartLoop) return;
    Serial.print("Je hebt ingevuld: "); Serial.println(UserInput);
    if (UserInput == 1) {
        FillNextStation(stationValue);
    }
}

void StationChoices(){
  switch (Buf_Route){
    case 1:
      Serial.println("Mogelijke stations binnen routen zijn: 1, 2 en 3");
      StationChoice(1, Station1);
      StationChoice(2, Station2);
      StationChoice(3, Station3);
      break;

    case 2:
      Serial.println("Mogelijke stations binnen routen zijn: 4 en 3");
      StationChoice(4, Station4);
      StationChoice(3, Station3);
      break;

    case 3:
      Serial.println("Mogelijke stations binnen routen zijn: 1, 2 en 3");
      StationChoice(1, Station1);
      StationChoice(2, Station2);
      StationChoice(3, Station3);
      break;

    case 4:
      Serial.println("Mogelijke stations binnen routen zijn: 2 en 3");
      StationChoice(2, Station2);
      StationChoice(3, Station3);
      break;

    case 5:
      Serial.println("Mogelijke stations binnen routen zijn: 1, 4 en 3");
      StationChoice(1, Station1);
      StationChoice(4, Station4);
      StationChoice(3, Station3);
      break;

    case 6:
      Serial.println("Mogelijke stations binnen routen zijn: 2 en 3");
      StationChoice(2, Station2);
      StationChoice(3, Station3);
      break;

    case 7:
      Serial.println("Mogelijke stations binnen routen zijn: 1, 4 en 3");
      StationChoice(1, Station1);
      StationChoice(4, Station4);
      break;

    case 8:
      Serial.println("Mogelijke stations binnen routen zijn: 4 en 3");
      StationChoice(4, Station4);
      StationChoice(3, Station3);
      break;

    default:
      Buf_1e_Station = 0;
      Buf_2e_Station = 0;
      Buf_3e_Station = 0;
      Buf_4e_Station = 0;
      break;
  }
}

void ShowPlanner(){
  Serial.println("De volgende informatie is ingevuld:");
  Serial.print("Route: "); Serial.println(Buf_Route);
  Serial.print("Eerste station: "); Serial.println(Buf_1e_Station);
  Serial.print("Tweede station: "); Serial.println(Buf_2e_Station);
  Serial.print("Derde station: "); Serial.println(Buf_3e_Station);
  Serial.print("Vierde station: "); Serial.println(Buf_4e_Station);  
}

void SendMessage() {
  esp_err_t Result = esp_now_send(Mac1, (uint8_t *) &OutgoingMessage, sizeof(OutgoingMessage));
  if (Result == ESP_OK) {
    Serial.println("Bericht succesvol verzonden");
  } else {
    Serial.print("Verzendfout: "); Serial.println(Result);
  }
}

void MakeMessage(int b, int d, int s, int m, int r, int fs, int ss, int ts, int fos, int e){
  OutgoingMessage.Begin_Key = b;
  OutgoingMessage.Dest_ID = d;
  OutgoingMessage.Source_ID = s;
  OutgoingMessage.Message_Kind = m;
  OutgoingMessage.Data1 = r;
  OutgoingMessage.Data2 = fs;
  OutgoingMessage.Data3 = ss;
  OutgoingMessage.Data4 = ts;
  OutgoingMessage.Data5 = fos;
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
  Buf_Route = 0;
  Buf_1e_Station = 0;
  Buf_2e_Station = 0;
  Buf_3e_Station = 0;
  Buf_4e_Station = 0;
  RestartLoop = false;

  Serial.println("================================");
  Serial.println("Pizzatown vrachtwagen controle: ");
  Serial.println("*Druk op q om het menu te herstellen*");

  RouteChoice(); if (RestartLoop) return;
  StationChoices(); if (RestartLoop) return;

  ShowPlanner();
  MakeMessage(Begin_Key_DEF,
              Dest_ID_DEF,
              Own_ID_DEF,
              Message_Kind_DEF,
              Buf_Route,
              Buf_1e_Station,
              Buf_2e_Station,
              Buf_3e_Station,
              Buf_4e_Station,
              End_Key_DEF);
  SendMessage();
}
