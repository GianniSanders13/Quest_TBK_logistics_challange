#include <SPI.h>
#include <MFRC522.h>

#define SCK_PIN 13   // Serial Clock
#define MISO_PIN 12  // Master In Slave Out
#define MOSI_PIN 8   // Master Out Slave In
#define SS_PIN 10    // Slave Select
#define RST_PIN 5    // Reset

MFRC522 mfrc522(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();

  Serial.println(F("Scan een RFID-kaart met geformatteerde UID (ID,KSS,SID)..."));
}

void loop() {
  // Wacht op een nieuwe kaart
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Controleer of het UID 4 bytes is
  if (mfrc522.uid.size != 4) {
    Serial.println(F("UID is niet 4 bytes. Ongeldige kaart voor dit formaat."));
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }

  // Lees UID bytes
  byte uid0 = mfrc522.uid.uidByte[0];
  byte uid1 = mfrc522.uid.uidByte[1];
  byte uid2 = mfrc522.uid.uidByte[2];
  byte uid3 = mfrc522.uid.uidByte[3];

  // Decodeer naar ID, KSS en SID
  int id = (uid0 << 8) | uid1;
  byte kss = uid2;
  byte sid = uid3;

  // Toon resultaat
  Serial.print(F("Gelezen UID -> ID: "));
  Serial.print(id);
  Serial.print(F(", KSS: "));
  Serial.print(kss);
  Serial.print(F(", SID: "));
  Serial.println(sid);

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
