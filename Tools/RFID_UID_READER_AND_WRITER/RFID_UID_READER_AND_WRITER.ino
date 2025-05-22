#include <SPI.h>
#include <MFRC522.h>

#define SCK_PIN 13
#define MISO_PIN 12
#define MOSI_PIN 8
#define SS_PIN 10
#define RST_PIN 5

MFRC522 mfrc522(SS_PIN, RST_PIN);

byte newUid[4];
bool uidReady = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
  mfrc522.PCD_Init();

  Serial.println(F("=== RFID klaar ==="));
  Serial.println(F("Scan een kaart om UID te lezen."));
  Serial.println(F("Wil je een nieuwe UID schrijven?"));
  Serial.println(F("Geef input in het formaat: ID (0-65535), KSS (0-255), SID (0-255)"));
  Serial.println(F("Voorbeeld: 258,12,34"));
  Serial.println();
}

void loop() {
  // Input check
  if (!uidReady && Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    int id = 0, kss = 0, sid = 0;
    int count = sscanf(input.c_str(), "%d,%d,%d", &id, &kss, &sid);

    if (count == 3 && id >= 0 && id <= 65535 && kss >= 0 && kss <= 255 && sid >= 0 && sid <= 255) {
      newUid[0] = (id >> 8) & 0xFF;
      newUid[1] = id & 0xFF;
      newUid[2] = kss;
      newUid[3] = sid;

      uidReady = true;
      Serial.println(F("\n--- Nieuwe UID opgesteld ---"));
      Serial.print(F("Nieuwe UID -> ID: "));
      Serial.print(id);
      Serial.print(F(", KSS: "));
      Serial.print(kss);
      Serial.print(F(", SID: "));
      Serial.println(sid);
      Serial.println(F("Houd een kaart bij de lezer om te schrijven...\n"));
    } else {
      Serial.println(F(">> Ongeldige invoer. Gebruik het formaat: ID,KSS,SID (bijv. 258,12,34)\n"));
    }
    return;
  }

  // RFID check
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // UID uitlezen
  Serial.println(F("=== Kaart gedetecteerd ==="));
  Serial.print(F("UID (hex): "));
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  int currentId = (mfrc522.uid.uidByte[0] << 8) | mfrc522.uid.uidByte[1];
  byte currentKss = mfrc522.uid.uidByte[2];
  byte currentSid = mfrc522.uid.uidByte[3];

  Serial.print(F("Geformatteerd -> ID: "));
  Serial.print(currentId);
  Serial.print(F(", KSS: "));
  Serial.print(currentKss);
  Serial.print(F(", SID: "));
  Serial.println(currentSid);
  Serial.println();

  // Als schrijfmodus actief is
  if (uidReady) {
    if (mfrc522.MIFARE_SetUid(newUid, 4, true)) {
      Serial.println(F("✅ Nieuwe UID succesvol geschreven!\n"));
    } else {
      Serial.println(F("❌ Fout bij schrijven van nieuwe UID.\n"));
    }
    uidReady = false;
    Serial.println(F("Voer opnieuw ID,KSS,SID in als je een nieuwe UID wilt schrijven."));
    Serial.println();
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}
