#include <SPI.h>
#include <MFRC522.h>

#define TAG_1 "A22D3A51"
#define TAG_2 "A2341A51"
#define TAG_3 "A28D4A51"
#define TAG_4 "608ED155"


#define DEBUG_RFID true
#define DEBUG_RFID_CHECK true

#define SS_PIN 10
#define RST_PIN 5

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() 
{
  Serial.begin(9600);

  SPI.begin();                          // init SPI bus
  rfid.PCD_Init();                      // init MFRC522
}

void loop() {
  int position = 0;

  String uid = readRFIDReader();        // Read RFID reader
  if (uid != "")                        // if their is a uid
  {                     
    position = uidCheck(uid);           // Get position value from uidCheck
  }
  
}

String readRFIDReader() {
  String uidStr = "";

  if (rfid.PICC_IsNewCardPresent()) 
  {
    if (rfid.PICC_ReadCardSerial()) 
    {
      for (int i = 0; i < rfid.uid.size; i++) 
      {
        if (rfid.uid.uidByte[i] < 0x10) uidStr += "0";
        uidStr += String(rfid.uid.uidByte[i], HEX);
      }

      uidStr.toUpperCase();    // Make the UID uppercase
      rfid.PICC_HaltA();       // Halt PICC
      rfid.PCD_StopCrypto1();  // Stop encryption

      // Debug prints  
      if (DEBUG_RFID) {
        Serial.print("RFID reader read: ");     
        Serial.println(uidStr);
      }
    }
  }
  return uidStr;
}

int uidCheck(String uidStr) {
  String debugPrint = "Tag onbekend"; 
  int returnValue = 0; 

  if (uidStr == TAG_1) {
    debugPrint = "Tag 1";
    returnValue = 1;
  } 
  else if (uidStr == TAG_2) {
    debugPrint = "Tag 2";
    returnValue = 2;
  } 
  else if (uidStr == TAG_3) {
    debugPrint = "Tag 3";
    returnValue = 3;
  } 
  else if (uidStr == TAG_4) {
    debugPrint = "Tag 4";
    returnValue = 4;
  }

  // Debug prints  
  if (DEBUG_RFID_CHECK)
  {
    Serial.print("Tag: ");
    Serial.print(debugPrint);
    Serial.print(", Position Value: ");
    Serial.println(returnValue);
  }

  return returnValue;
}

