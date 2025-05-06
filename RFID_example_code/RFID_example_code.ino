#include <SPI.h>
#include <MFRC522.h>

#define TAG_1 "F7910A3E"
#define TAG_2 "32920A3E"
#define TAG_3 "936E0A3E"
#define TAG_4 "536E0A3E"

#define DEBUG_RFID true
#define DEBUG_RFID_CHECK true

#define SS_PIN 10
#define RST_PIN 5

#define ESP32_RED    0
#define ESP32_GREEN  1
#define ESP32_BLUE   2

#define RED_PIN      14
#define GREEN_PIN    15
#define BLUE_PIN     16


// Color position defines
#define ESP32_LED_RED        0
#define ESP32_LED_ORANGE     1
#define ESP32_LED_YELLOW     2
#define ESP32_LED_GREEN      3
#define ESP32_LED_LIGHTGREEN 4
#define ESP32_LED_CYAN       5
#define ESP32_LED_BLUE       6
#define ESP32_LED_PURPLE     7
#define ESP32_LED_PINK       8
#define ESP32_LED_WHITE      9
#define ESP32_LED_OFF        10

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() 
{
  Serial.begin(9600);

  SPI.begin();                          // init SPI bus
  rfid.PCD_Init();                      // init MFRC522

  ledcSetup(ESP32_RED,   5000, 8);
  ledcSetup(ESP32_GREEN, 5000, 8);
  ledcSetup(ESP32_BLUE,  5000, 8);

  ledcAttachPin(RED_PIN,   ESP32_RED);
  ledcAttachPin(GREEN_PIN, ESP32_GREEN);
  ledcAttachPin(BLUE_PIN,  ESP32_BLUE);
}

void loop() {
  int position = 0;

  String uid = readRFIDReader();        // Read RFID reader
  if (uid != "")                        // if their is a uid
  {                     
    position = uidCheck(uid);           // Get position value from uidCheck
    ESP32LedCrontrol(position);
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

void ESP32LedCrontrol(int position) {
  switch(position) {
    case 0: // Red
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 255);
      ledcWrite(ESP32_BLUE,  255);
      break;
    case 1: // Green
      ledcWrite(ESP32_RED,   255);
      ledcWrite(ESP32_GREEN, 0);
      ledcWrite(ESP32_BLUE,  255);
      break;
    case 2: // Blue
      ledcWrite(ESP32_RED,   255);
      ledcWrite(ESP32_GREEN, 255);
      ledcWrite(ESP32_BLUE,  0);
      break;
    case 3: // Yellow (Red + Green)
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 0);
      ledcWrite(ESP32_BLUE,  255);
      break;
    case 4: // Purple (Red + Blue)
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 255);
      ledcWrite(ESP32_BLUE,  0);
      break;
    case 5: // Cyan (Green + Blue)
      ledcWrite(ESP32_RED,   255);
      ledcWrite(ESP32_GREEN, 0);
      ledcWrite(ESP32_BLUE,  0);
      break;
    case 6: // White (Red + Green + Blue)
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 0);
      ledcWrite(ESP32_BLUE,  0);
      break;
    case 7: // Orange (mostly Red + some Green)
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 100);
      ledcWrite(ESP32_BLUE,  255);
      break;
    case 8: // Pink (Red + a bit of Blue)
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 200);
      ledcWrite(ESP32_BLUE,  100);
      break;
    case 9: // Light Green
      ledcWrite(ESP32_RED,   200);
      ledcWrite(ESP32_GREEN, 0);
      ledcWrite(ESP32_BLUE,  200);
      break;
    default: // OFF (all colors off)
      ledcWrite(ESP32_RED,   255);
      ledcWrite(ESP32_GREEN, 255);
      ledcWrite(ESP32_BLUE,  255);
      break;
  }
}

