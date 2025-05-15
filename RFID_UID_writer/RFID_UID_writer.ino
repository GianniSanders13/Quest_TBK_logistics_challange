#include <SPI.h>
#include <MFRC522.h>

#define SCK_PIN 13   // Serial Clock
#define MISO_PIN 12  // Master In Slave Out
#define MOSI_PIN 8   // Master Out Slave In
#define SS_PIN 10    // Slave Select
#define RST_PIN 5    // Reset

MFRC522 mfrc522(SS_PIN, RST_PIN);

byte newUid[4];
bool uidReady = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();

  Serial.println(F("Enter values as follows (decimal, comma-separated):"));
  Serial.println(F("ID (0-65535), KSS (0-255), SID (0-255). Example: 258,12,34"));
}

void loop() {
  if (!uidReady && Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    int id = 0, kss = 0, sid = 0;
    int count = sscanf(input.c_str(), "%d,%d,%d", &id, &kss, &sid);

    if (count == 3 && id >= 0 && id <= 65535 && kss >= 0 && kss <= 255 && sid >= 0 && sid <= 255) {
      newUid[0] = (id >> 8) & 0xFF; // High byte of ID
      newUid[1] = id & 0xFF;        // Low byte of ID
      newUid[2] = kss;
      newUid[3] = sid;

      uidReady = true;
      Serial.println(F("New UID constructed. Hold a card near the reader..."));
    } else {
      Serial.println(F("Invalid input. Please enter: ID,KSS,SID (e.g. 258,12,34)"));
    }
    return;
  }

  if (!uidReady) return;

  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  Serial.print(F("Current card UID:"));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  int currentId = (mfrc522.uid.uidByte[0] << 8) | mfrc522.uid.uidByte[1];
  byte currentKss = mfrc522.uid.uidByte[2];
  byte currentSid = mfrc522.uid.uidByte[3];

  Serial.print(F("Split UID -> ID: "));
  Serial.print(currentId);
  Serial.print(F(", KSS: "));
  Serial.print(currentKss);
  Serial.print(F(", SID: "));
  Serial.println(currentSid);

  if (mfrc522.MIFARE_SetUid(newUid, 4, true)) {
    Serial.println(F("New UID successfully written!"));
  } else {
    Serial.println(F("Error writing new UID."));
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  uidReady = false;
  Serial.println(F("\nEnter new UID values (ID,KSS,SID):"));
}
