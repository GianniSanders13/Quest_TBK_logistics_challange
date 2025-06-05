/*
 * File: IndustriegebiedCommunicatieModule
 * Author: Liam Grovenstein
 * Created: 28 05 2025
 * Description: Code that sends instructions to vehicle
 */

#include <WiFi.h>
#include <esp_now.h>

// --------------------DEBUG --------------------------------------------------------
#define DEBUG false              // Enable or disable all DEBUG prints
#define OUTGOING_MESSAGE true  // " prints full outgoing message
#define WIFI true              // " prints wifi related debug messages
//------------------------------------------------------------------------------------

#define Own_ID_DEF 1
#define Begin_Key_DEF 10
#define Message_Kind_DEF 1
#define End_Key_DEF 5
#define MAX_CARGO 8

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
  uint8_t Data6;
  uint8_t End_Key;
} Message;

//Global variables
uint8_t ReceiveAdress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
Message OutgoingMessage;
bool NewMessageReceived = false;
int UserInput = 0;
bool RestartLoop = false;
int Car = 0;
int Buf_Dest_ID = 0;
int Buf_1e_Station = 0;
int Buf_1e_Amount = 0;
int Buf_2e_Station = 0;
int Buf_2e_Amount = 0;
int Buf_3e_Station = 0;
int Buf_3e_Amount = 0;

//-------------------------------------Functie Prototypes--------------------------------------------------------------
int ValidateInput(String Prompt, int MinimumValue, int MaximumValue);
//void CarChoice();
void FirstStation();
void FirstAmount();
void SecondStationAndAmount();
void SecondAmount();
void ShowPlanner();
void MakeMessage(int B_Key, int D_ID, int S_ID, int M_Kind, int D1, int D2, int D3, int D4, int E_Key);
void ConfirmSending();
void SendMessage();

//-------------------------------------Setup and main loop--------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(3000);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    #if DEBUG && WIFI
    Serial.println("---------------WIFI DEBUG-----------------");
    Serial.println("ESP-NOW init mislukt");
    Serial.println("------------------------------------------");
    #endif
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, ReceiveAdress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  peerInfo.ifidx = WIFI_IF_STA;

  if (!esp_now_is_peer_exist(ReceiveAdress)) {
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      #if DEBUG && WIFI
      Serial.println("---------------WIFI DEBUG-----------------");
      Serial.println("Toevoegen van broadcast-peer mislukt");
      Serial.println("------------------------------------------");
      #endif
      return;
    }
  }
}

void loop() {
  RestartLoop = false;

  Serial.println("===================================================================================================");
  Serial.println("Industrie vrachtwagen controle: ");
  Serial.println("*Druk op q om het menu te herstellen*"); Serial.println(" ");

  //CarChoice(); if (RestartLoop) return;
  FirstStation(); if (RestartLoop) return;
  FirstAmount(); if (RestartLoop) return;
  SecondStationAndAmount(); if (RestartLoop) return;
  ThirdStationAndAmount(); if (RestartLoop) return;

  ShowPlanner();
  MakeMessage(Begin_Key_DEF,
              Buf_Dest_ID,
              Own_ID_DEF,
              Message_Kind_DEF,
              Buf_1e_Station,
              Buf_1e_Amount,
              Buf_2e_Station,
              Buf_2e_Amount,
              Buf_3e_Station,
              Buf_3e_Amount,
              End_Key_DEF);
  
  ConfirmSending(); if (RestartLoop) return;
  SendMessage();
}

//-------------------------------------User input validatie--------------------------------------------------------------
int ValidateInput(String Prompt, int MinimumValue, int MaximumValue){
  String Input;
  int Result = 0;
  bool Valid = false;
  while (!Valid){
    Serial.println(Prompt);
    while (Serial.available() == 0) {}
    Input = Serial.readStringUntil('\n');
    Input.trim();
    if (Input == "q" || Input == "Q"){
      RestartLoop = true;
      return -1;
    }
    bool IsNumber = true;
    for (unsigned int i = 0; i<Input.length(); i++){
      if (!isDigit(Input[i])){
        IsNumber = false;
        break;
      }
    }
    if (!IsNumber){
      Serial.println("Ongeldige waarde ingevoerd: voer een getal in!");
      continue;
    }
    Result = Input.toInt();
    if (Result<MinimumValue || Result>MaximumValue){
      Serial.print("Getal is niet van goede waarde: voer getal in tussen ");
      Serial.print(MinimumValue);
      Serial.print(" en ");
      Serial.println(MaximumValue);
    } else{
      Valid = true;
    }
  }
  return Result;
}

//-------------------------------------Menu choices--------------------------------------------------------------
/*void CarChoice(){
  UserInput = ValidateInput("Vrachtwagen 1 of 2?", 1, 2);
  if (RestartLoop) return;
  Car = UserInput;
  Serial.print("Je hebt ingevuld:");
  Serial.println(Car); Serial.println(" ");
  if (Car == 1) Buf_Dest_ID = 3;
  if (Car == 2) Buf_Dest_ID = 4;
}*/

void FirstStation(){
  bool error = false;
  UserInput = ValidateInput("Welk station?: Pepperoni (1) Deeg(2), tomaat(3), kaas(4) of kip(5)? ", 1, 5);
  if (RestartLoop) return;
  Buf_1e_Station = UserInput;
  Serial.print("Je hebt ingevuld: ");
  Serial.println(Buf_1e_Station); Serial.println(" ");
}

void FirstAmount() {
  String prompt = "Hoeveel wil je ophalen? (1 t/m " + String(MAX_CARGO) + "):";
  UserInput = ValidateInput(prompt, 1, MAX_CARGO);
  if (RestartLoop) return;
  Buf_1e_Amount = UserInput;
  Serial.print("Je hebt ingevuld: ");
  Serial.println(Buf_1e_Amount); Serial.println(" ");
}

void SecondStationAndAmount(){
  int Choice = 0;
  if(MAX_CARGO - Buf_1e_Amount > 0) {
    switch (Buf_1e_Station) {
      case 1:
        UserInput = ValidateInput("Welk tweede station?: Deeg(2), tomaat(3), kaas(4), kip(5) of geen(6)? ", 2, 6);
        if (RestartLoop) return;
        if(UserInput == 6){
          Buf_2e_Station = 0;
        } else{
          Buf_2e_Station = UserInput;
        }
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_2e_Station); Serial.println(" ");
        SecondAmount();
      break;
      case 2:
        UserInput = ValidateInput("Welk tweede station?: Tomaat(3), kaas(4), kip(5) of geen(6)? ", 3, 6);
        if (RestartLoop) return;
        if(UserInput == 6){
          Buf_2e_Station = 0;
        } else{
          Buf_2e_Station = UserInput;
        }
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_2e_Station); Serial.println(" ");
        SecondAmount();
      break;
      case 3:
        UserInput = ValidateInput("Welk tweede station?: Kaas(4), kip(5) of geen(6)? ", 4, 6);
        if (RestartLoop) return;
        if(UserInput == 6){
          Buf_2e_Station = 0;
        } else{
          Buf_2e_Station = UserInput;
        }
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_2e_Station); Serial.println(" ");
        SecondAmount();
      break;
      case 4:
        UserInput = ValidateInput("Wil je naar station kip?: Ja(1) of Nee(2)", 1, 2);
        if (RestartLoop) return;
        Choice = UserInput;
        Serial.print("Je hebt ingevuld: ");
        if(Choice == 1){
          Buf_2e_Station = 5;
        } else{
          Buf_2e_Station = 0;
        }
        Serial.println(Choice); Serial.println(" ");
        SecondAmount();
      break;
      case 5:
        Serial.println("Geen volgende station mogelijk!"); Serial.println(" ");
        Buf_2e_Station = 0;
        Buf_2e_Amount = 0;
      break;
    }
  } else {
    Serial.println("Geen volgende station mogelijk!"); Serial.println(" ");
    Buf_2e_Station = 0;
    Buf_2e_Amount = 0;
  }
}

void SecondAmount(){
  if(Buf_2e_Station == 0){
    Buf_2e_Amount = 0;
  } else{
    int max_second_amount = MAX_CARGO - Buf_1e_Amount;
    if (max_second_amount <= 0) {
      Serial.println("Geen ruimte meer voor extra goederen.");
      Buf_2e_Amount = 0;
      return;
    }

    String prompt = "Hoeveel wil je ophalen? (max " + String(max_second_amount) + "): ";
    UserInput = ValidateInput(prompt, 1, max_second_amount);
    if (RestartLoop) return;

    Buf_2e_Amount = UserInput;
    Serial.print("Je hebt ingevuld: ");
    Serial.println(Buf_2e_Amount); Serial.println(" ");
  }
}

void ThirdStationAndAmount(){
  int Choice = 0;
  if(MAX_CARGO - Buf_1e_Amount - Buf_2e_Amount > 0) {
    switch (Buf_2e_Station) {
      case 1:
        UserInput = ValidateInput("Welk derde station?: Deeg(2), tomaat(3), kaas(4), kip(5) of geen(6)? ", 2, 6);
        if (RestartLoop) return;
        if(UserInput == 6){
          Buf_3e_Station = 0;
        } else{
          Buf_3e_Station = UserInput;
        }
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_3e_Station); Serial.println(" ");
        ThirdAmount();
      break;
      case 2:
        UserInput = ValidateInput("Welk derde station?: Tomaat(3), kaas(4), kip(5) of geen(6)? ", 3, 6);
        if (RestartLoop) return;
        if(UserInput == 6){
          Buf_3e_Station = 0;
        } else{
          Buf_3e_Station = UserInput;
        }
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_3e_Station); Serial.println(" ");
        ThirdAmount();
      break;
      case 3:
        UserInput = ValidateInput("Welk derde station?: Kaas(4), kip(5) of geen(6)? ", 4, 6);
        if (RestartLoop) return;
        if(UserInput == 6){
          Buf_3e_Station = 0;
        } else{
          Buf_3e_Station = UserInput;
        }
        Serial.print("Je hebt ingevuld: ");
        Serial.println(Buf_3e_Station); Serial.println(" ");
        ThirdAmount();
      break;
      case 4:
        UserInput = ValidateInput("Wil je naar station kip?: Ja(1) of Nee(2)", 1, 2);
        if (RestartLoop) return;
        Choice = UserInput;
        Serial.print("Je hebt ingevuld: ");
        if(Choice == 1){
          Buf_3e_Station = 5;
        } else{
          Buf_3e_Station = 0;
        }
        Serial.println(Choice); Serial.println(" ");
        ThirdAmount();
      break;
      case 5:
        Serial.println("Geen volgende station mogelijk!"); Serial.println(" ");
        Buf_3e_Station = 0;
        Buf_3e_Amount = 0;
      break;
    }
  } else {
    Serial.println("Geen volgende station mogelijk!"); Serial.println(" ");
    Buf_3e_Station = 0;
    Buf_3e_Amount = 0;
  }
}

void ThirdAmount(){
  if(Buf_3e_Station == 0){
    Buf_3e_Amount = 0;
  } else{
    int max_third_amount = MAX_CARGO - Buf_1e_Amount - Buf_2e_Amount;
    if (max_third_amount <= 0) {
      Serial.println("Geen ruimte meer voor extra goederen.");
      Buf_3e_Amount = 0;
      return;
    }

    String prompt = "Hoeveel wil je ophalen? (max " + String(max_third_amount) + "): ";
    UserInput = ValidateInput(prompt, 1, max_third_amount);
    if (RestartLoop) return;

    Buf_3e_Amount = UserInput;
    Serial.print("Je hebt ingevuld: ");
    Serial.println(Buf_3e_Amount); Serial.println(" ");
  }
}

void ConfirmSending(){
  UserInput = ValidateInput("Wil je instructies versturen?: Ja(1) of Nee(q)", 1, 1); Serial.println(" ");
  if (RestartLoop) return;
}

//-------------------------------------Planner--------------------------------------------------------------
void ShowPlanner(){
  Serial.println("-------------Planner---------------");
  Serial.println("De volgende informatie is ingevuld:");
  Serial.print("Auto: "); Serial.println(Car);
  Serial.print("Eerste station: "); Serial.println(Buf_1e_Station);
  Serial.print("Hoeveelheid: "); Serial.println(Buf_1e_Amount);
  Serial.print("Tweede station: "); Serial.println(Buf_2e_Station);
  Serial.print("Hoeveelheid: "); Serial.println(Buf_2e_Amount);
  Serial.print("Derde station: "); Serial.println(Buf_3e_Station);
  Serial.print("Hoeveelheid: "); Serial.println(Buf_3e_Amount);
  Serial.println("-----------------------------------"); Serial.println(" ");
}

//-------------------------------------Wifi--------------------------------------------------------------
void SendMessage(){
  esp_err_t Result = esp_now_send(ReceiveAdress, (uint8_t *) &OutgoingMessage, sizeof(OutgoingMessage));
  if (Result == ESP_OK) {
    Serial.println("Bericht succesvol verzonden"); Serial.println(" ");
    #if DEBUG && OUTGOING_MESSAGE
      Serial.println("---------------Outgoing Message DEBUG-----------------");
      Serial.print  ("Begin key: "); Serial.println(OutgoingMessage.Begin_Key);
      Serial.print  ("Destination ID: "); Serial.println(OutgoingMessage.Dest_ID);
      Serial.print  ("Source ID: "); Serial.println(OutgoingMessage.Source_ID);
      Serial.print  ("Message kind: "); Serial.println(OutgoingMessage.Message_Kind);
      Serial.print  ("Data1: "); Serial.println(OutgoingMessage.Data1);
      Serial.print  ("Data2: "); Serial.println(OutgoingMessage.Data2);
      Serial.print  ("Data3: "); Serial.println(OutgoingMessage.Data3);
      Serial.print  ("Data4: "); Serial.println(OutgoingMessage.Data4);
      Serial.print  ("Data5: "); Serial.println(OutgoingMessage.Data5);
      Serial.print  ("Data6: "); Serial.println(OutgoingMessage.Data6);
      Serial.print("End key: "); Serial.println(OutgoingMessage.End_Key);
      Serial.println("------------------------------------------------------"); Serial.println(" ");
    #endif
  } else {
    Serial.print("Verzendfout: "); Serial.println(Result); Serial.println(" ");
  }
}

void MakeMessage(int B_Key, int D_ID, int S_ID, int M_Kind, int D1, int D2, int D3, int D4, int D5, int D6, int E_Key){
  OutgoingMessage.Begin_Key = B_Key; 
  OutgoingMessage.Dest_ID = D_ID;
  OutgoingMessage.Source_ID = S_ID;
  OutgoingMessage.Message_Kind = M_Kind;
  OutgoingMessage.Data1 = D1;
  OutgoingMessage.Data2 = D2;
  OutgoingMessage.Data3 = D3;
  OutgoingMessage.Data4 = D4;
  OutgoingMessage.Data5 = D5;
  OutgoingMessage.Data6 = D6;
  OutgoingMessage.End_Key = E_Key;
}
