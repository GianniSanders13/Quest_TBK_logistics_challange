/*
 * File: PizzatownCommunicatieModule
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

//Message defines
#define Own_ID_DEF 1
#define Dest_ID_DEF 9
#define Begin_Key_DEF 10
#define Message_Kind_DEF 1
#define End_Key_DEF 5

//SID tage defines
#define Station1 1
#define Station2 2
#define Station3 3
#define Station4 6

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

//Global variables
uint8_t ReceiveAdress[] = {0xEC, 0xDA, 0x3B, 0x60, 0x0B, 0x2C};
Message OutgoingMessage;
bool NewMessageReceived = false;
int UserInput = 0;
bool RestartLoop = false;
bool Done = false;
int Buf_Route = 0;
int Buf_1e_Station = 0;
int Buf_2e_Station = 0;
int Buf_3e_Station = 0;

//-------------------------------------Functie Prototypes--------------------------------------------------------------
int ValidateInput(String Prompt, int MinimumValue, int MaximumValue);
int ValidateInputFromList(String Prompt, int* ValidOptions, int OptionCount);
void FillNextStation(int value);
void FirstStation();
void SecondStation();
void ThirdStation();
void ShowPlanner();
void SendMessage();
void MakeMessage(int B_Key, int D_ID, int S_ID, int M_Kind, int D1, int D2, int D3, int D4, int E_Key);
void Reset();
void ConfirmSending();
void ConnectRoute();

//-------------------------------------Setup and main loop--------------------------------------------------------------
void setup(){
  Serial.begin(115200);
  delay(3000);
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK){
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

  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    #if DEBUG && WIFI
    Serial.println("---------------WIFI DEBUG-----------------");
    Serial.println("Peer toevoegen mislukt");
    Serial.println("------------------------------------------");
    #endif
    return;
  }
}

void loop(){
  RestartLoop = false;
  Reset();

  Serial.println("================================");
  Serial.println("Pizzatown vrachtwagen controle: ");
  Serial.println("*Druk op q om het menu te herstellen*"); Serial.println(" ");

  FirstStation();
  if(!Done){SecondStation();}
  if(!Done){ThirdStation();}

  ConnectRoute();
  ShowPlanner();
  MakeMessage(Begin_Key_DEF,
              Dest_ID_DEF,
              Own_ID_DEF,
              Message_Kind_DEF,
              Buf_Route,
              Buf_1e_Station,
              Buf_2e_Station,
              Buf_3e_Station,
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

//-------------------------------------Buffer filling and cleaning-------------------------------------------------
void FillNextStation(int value){
  if (Buf_1e_Station == 0) {
      Buf_1e_Station = value;
  } else if (Buf_2e_Station == 0){
      Buf_2e_Station = value;
  } else if (Buf_3e_Station == 0){
      Buf_3e_Station = value;
  } else{
      Serial.println("*Alles vol!*"); Serial.println(" ");
  }
}

void Reset(){
  Buf_Route = 0;
  Done = false;
  Buf_1e_Station = 0;
  Buf_2e_Station = 0;
  Buf_3e_Station = 0;
}
//-------------------------------------Menu choices--------------------------------------------------------------
void FirstStation(){
  UserInput = ValidateInput("Naar welk huis wil je?: 1, 2, 3 of 4", 1, 4);
  if (RestartLoop) return;
  Serial.print("Je hebt ingevuld: "); Serial.println(UserInput); Serial.println(" ");
  if(UserInput == 4){
    FillNextStation(6);
  }
  else{
    FillNextStation(UserInput);
  }
  if(UserInput == 3){
    Done = true;
  }
}

void SecondStation(){
  if(Buf_1e_Station == 3){
    return;
  }
  else{
    UserInput = ValidateInput("Wil je naar een tweede huis?: Ja(1) of Nee(2)", 1, 2);
    if (RestartLoop) return;
    Serial.print("Je hebt ingevuld: "); Serial.println(UserInput); Serial.println(" ");
    if(UserInput == 1){
      switch(Buf_1e_Station){
        case 1:
          UserInput = ValidateInput("Naar welk huis wil je?: 2, 3 of 4", 2, 4);
          if (RestartLoop) return;
          Serial.print("Je hebt ingevuld: "); Serial.println(UserInput); Serial.println(" ");
          if(UserInput == 4){
            FillNextStation(6);
          }
          else{
            FillNextStation(UserInput);
          }
          if(UserInput == 3){
            Done = true;
          }
        break;
        case 2:
          UserInput = ValidateInput("Wil je naar huis 3?: Ja(1) of Nee(2)", 1, 2);
          if (RestartLoop) return;
          Serial.print("Je hebt ingevuld: "); Serial.println(UserInput); Serial.println(" ");
          if(UserInput == 1){
            FillNextStation(3);
          }
          Done = true;
        break;
        case 6:
          UserInput = ValidateInput("Wil je naar huis 3?: Ja(1) of Nee(2)", 1, 2);
          if (RestartLoop) return;
          Serial.print("Je hebt ingevuld: "); Serial.println(UserInput); Serial.println(" ");
          if(UserInput == 1){
            FillNextStation(3);
          }
          Done = true;
        break;
      }
    }
    else{
      Done = true;
    }
  } 
}

void ThirdStation(){
  if(Buf_2e_Station == 3){
    return;
  }
  else{
    UserInput = ValidateInput("Wil je naar een derde huis?: Ja(1) of Nee(2)", 1, 2);
    if (RestartLoop) return;
    Serial.print("Je hebt ingevuld: "); Serial.println(UserInput); Serial.println(" ");
    if(UserInput == 1){
      UserInput = ValidateInput("Wil je naar huis 3?: Ja(1) of Nee(2)", 1, 2);
      if (RestartLoop) return;
      Serial.print("Je hebt ingevuld: "); Serial.println(UserInput); Serial.println(" ");
      if(UserInput == 1){
        FillNextStation(3);
      }
      Done = true;
    }
    else{
      Done = true;
    }
  }
}

void ConfirmSending(){
  UserInput = ValidateInput("Wil je instructies versturen?: Ja(1) of Nee(q)", 1, 1); Serial.println(" ");
  if (RestartLoop) return;
}

//-------------------------------------Route connect--------------------------------------------------------
void ConnectRoute(){
  if(Buf_1e_Station == 1 && Buf_2e_Station == 0 && Buf_3e_Station == 0){
    Buf_Route = 1;
  }
  if(Buf_1e_Station == 1 && Buf_2e_Station == 2 && Buf_3e_Station == 0){
    Buf_Route = 1;
  }
  if(Buf_1e_Station == 1 && Buf_2e_Station == 2 && Buf_3e_Station == 3){
    Buf_Route = 1;
  }
  if(Buf_1e_Station == 1 && Buf_2e_Station == 3 && Buf_3e_Station == 0){
    Buf_Route = 1;
  }
  if(Buf_1e_Station == 1 && Buf_2e_Station == 6 && Buf_3e_Station == 0){
    Buf_Route = 7;
  }
  if(Buf_1e_Station == 1 && Buf_2e_Station == 6 && Buf_3e_Station == 3){
    Buf_Route = 7;
  }
  if(Buf_1e_Station == 2 && Buf_2e_Station == 0 && Buf_3e_Station == 0){
    Buf_Route = 4;
  }
  if(Buf_1e_Station == 2 && Buf_2e_Station == 3 && Buf_3e_Station == 0){
    Buf_Route = 4;
  }
  if(Buf_1e_Station == 3 && Buf_2e_Station == 0 && Buf_3e_Station == 0){
    Buf_Route = 2;
  }
  if(Buf_1e_Station == 6 && Buf_2e_Station == 0 && Buf_3e_Station == 0){
    Buf_Route = 2;
  }
  if(Buf_1e_Station == 6 && Buf_2e_Station == 3 && Buf_3e_Station == 0){
    Buf_Route = 2;
  }
}

//-------------------------------------Planner--------------------------------------------------------------
void ShowPlanner(){
  Serial.println("-------------Planner---------------");
  Serial.println("De volgende informatie is ingevuld:");
  Serial.print("Route: "); Serial.println(Buf_Route);
  Serial.print("Eerste station: "); Serial.println(Buf_1e_Station);
  Serial.print("Tweede station: "); Serial.println(Buf_2e_Station);
  Serial.print("Derde station: "); Serial.println(Buf_3e_Station); 
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
      Serial.print("End key: "); Serial.println(OutgoingMessage.End_Key);
      Serial.println("------------------------------------------------------"); Serial.println(" ");
    #endif
  } else {
    Serial.print("Verzendfout: "); Serial.println(Result); Serial.println(" ");
  }
}

void MakeMessage(int B_Key, int D_ID, int S_ID, int M_Kind, int D1, int D2, int D3, int D4, int E_Key){
  OutgoingMessage.Begin_Key = B_Key; 
  OutgoingMessage.Dest_ID = D_ID;
  OutgoingMessage.Source_ID = S_ID;
  OutgoingMessage.Message_Kind = M_Kind;
  OutgoingMessage.Data1 = D1;
  OutgoingMessage.Data2 = D2;
  OutgoingMessage.Data3 = D3;
  OutgoingMessage.Data4 = D4;
  OutgoingMessage.End_Key = E_Key;
}
