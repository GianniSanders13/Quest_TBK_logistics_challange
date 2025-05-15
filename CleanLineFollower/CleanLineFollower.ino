#include "painlessMesh.h"
#include <SPI.h>
#include <MFRC522.h>

Scheduler userScheduler;
painlessMesh mesh;

// --------------------DEBUG --------------------------------------------------------

#define DEBUG false  // Enable or disable all DEBUG prints

#define DEBUG_RFID true         // RFID reader debug (what the readers received)
#define DEBUG_RFID_CHECK false  // DEBUG print of de RFID tag id
#define DEBUG_DRIVING false     // driving direct with sensor values.
#define DEBUG_FORMAT true
//------------------------------------------------------------------------------------

struct intersectionMapStruct {
  uint16_t tagId;
  uint16_t leftTag;
  uint16_t straightTag;
  uint16_t rightTag;
  uint16_t backwardTag;
};

uint16_t testRoute[] = { 11, 6, 7, 8, 2, 3, 4, 5, 11 };
const int testRouteLength = sizeof(testRoute) / sizeof(testRoute[0]);

intersectionMapStruct tagMap[] = {

  { 1, 2, 0, 6, 11 },  // tag 1
  { 2, 0, 3, 8, 1 },   // tag 2
  { 3, 0, 4, 9, 2 },   // tag 3
  { 4, 0, 5, 10, 3 },  // tag 4
  { 5, 11, 6, 0, 4 },  // tag 5
  { 6, 1, 7, 5, 11 },  // tag 6
  { 7, 8, 0, 10, 6 },  // tag 7
  { 8, 2, 9, 0, 7 },   // tag 8
  { 9, 3, 10, 0, 8 },  // tag 9
  { 10, 4, 7, 0, 9 },  // tag 10
  { 11, 0, 1, 6, 5 }   // tag 11
};
const int mapLength = sizeof(tagMap) / sizeof(tagMap[0]);

char routeCounter = 0;


// Mesh instellingen
#define MESH_NAME "Quest-Network"
#define MESH_PASSWORD "Quest-Password"
#define MESH_PORT 5555

// Ultrasoon
#define TRIG_PIN 20
#define ECHO_PIN 19

// Sensors
#define LEFT_SENSOR_PIN 17
#define RIGHT_SENSOR_PIN 18

// Motors - jouw originele namen
#define LEFT_MOTOR 3
#define LEFT_MOTOR_DIR 2
#define RIGHT_MOTOR 11
#define RIGHT_MOTOR_DIR 4

#define FORWARD HIGH
#define BACKWARD LOW

// PWM instellingen
#define PWM_FREQ 1000
#define PWM_RES 8  // 0-255
#define LEFT_PWM_CHANNEL 0
#define RIGHT_PWM_CHANNEL 1

// Parameters
int Speed = 220;
int NormalAdjust = Speed;
#define DISTANCE_THRESHOLD_CM 20
#define TRIG_PULSE_DURATION_US 10
#define TRIG_PULSE_DELAY_US 2
#define PULSE_TIMEOUT_US 30000

#define SCK_PIN 13   // Serial Clock (SCK)
#define MISO_PIN 12  // Master In Slave Out (MISO)
#define MOSI_PIN 8   // Master Out Slave In (MOSI)
#define SS_PIN 10    // Slave Select (SS)
#define RST_PIN 5

#define ESP_RED_PIN 14
#define ESP_GREEN_PIN 15
#define ESP_BLUE_PIN 16

#define ESP32_LED_RED 0
#define ESP32_LED_ORANGE 1
#define ESP32_LED_YELLOW 2
#define ESP32_LED_GREEN 3
#define ESP32_LED_LIGHTGREEN 4
#define ESP32_LED_CYAN 5
#define ESP32_LED_BLUE 6
#define ESP32_LED_PURPLE 7
#define ESP32_LED_PINK 8
#define ESP32_LED_WHITE 9
#define ESP32_LED_OFF 10


#define UIDBYTE0 0
#define UIDBYTE1 1
#define UIDBYTE2 2
#define UIDBYTE3 3

#define SIZE_UID 4
byte uidBytes[SIZE_UID];

uint16_t tagId;
uint8_t iS;
uint8_t sID;


MFRC522 rfid(SS_PIN, RST_PIN);

unsigned long previousMillis = 0;  // Will store last time RFID is read
const long interval = 200;         // Interval of reading RFID in milliseconds

// --- Motor functies ---
void Stop() {
  ledcWrite(LEFT_PWM_CHANNEL, 0);
  ledcWrite(RIGHT_PWM_CHANNEL, 0);
}

void Forward() {
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  ledcWrite(LEFT_PWM_CHANNEL, Speed);
  ledcWrite(RIGHT_PWM_CHANNEL, Speed);
}

void Left() {
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  ledcWrite(LEFT_PWM_CHANNEL, Speed - NormalAdjust);
  ledcWrite(RIGHT_PWM_CHANNEL, Speed);
}

void Right() {
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  ledcWrite(LEFT_PWM_CHANNEL, Speed);
  ledcWrite(RIGHT_PWM_CHANNEL, Speed - NormalAdjust);
}

// --- Sensor functies ---
int Ultrasoon_Check() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(TRIG_PULSE_DELAY_US);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(TRIG_PULSE_DURATION_US);
  digitalWrite(TRIG_PIN, LOW);
  int duration = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT_US);
  return duration * 0.034 / 2;
}

void SensorCheck() {
  String debugMessage;
  int s1 = !digitalRead(LEFT_SENSOR_PIN);
  int s2 = !digitalRead(RIGHT_SENSOR_PIN);
  String sData = String(s1) + String(s2);
  int INTsData = sData.toInt();

  switch (INTsData) {
    case 0:
      Forward();
      debugMessage = "Forward";
      break;
    case 1:
      Right();
      debugMessage = "Right";
      break;
    case 10:
      Left();
      debugMessage = "Left";
      break;
    case 11:
      Stop();
      debugMessage = "Stop";
      break;
    default:
      Forward();
      debugMessage = "Forward";
      break;
  }

#if DEBUG && DEBUG_DRIVING
  Serial.print("sensor value: ");
  Serial.println(sData);
  Serial.print("Direction: ");
  Serial.println(debugMessage);
  Serial.println();

#endif
}

bool readRFIDReader() {
  // byte uidSize = rfid.uid.size;
  if (rfid.PICC_IsNewCardPresent()) {
    if (rfid.PICC_ReadCardSerial()) {
      for (int i = 0; i < SIZE_UID; i++) {
        uidBytes[i] = rfid.uid.uidByte[i];
      }
      rfid.PICC_HaltA();       // Halt PICC
      rfid.PCD_StopCrypto1();  // Stop encryption

#if DEBUG && DEBUG_RFID
      Serial.print("RFID UID: ");
      for (byte i = 0; i < SIZE_UID; i++) {
        if (uidBytes[i] < 0x10) Serial.print("0");
        Serial.print(uidBytes[i], BIN);
        if (i < SIZE_UID - 1) Serial.print(":");
      }
      Serial.println("\n");

#endif

      return true;
    }
  }
  return false;
}

bool formatRfidUid() {

  tagId = ((uint16_t)uidBytes[UIDBYTE0] << 8) | uidBytes[UIDBYTE1];
  iS = uidBytes[UIDBYTE2];
  sID = uidBytes[UIDBYTE3];

#if DEBUG && DEBUG_FORMAT

  Serial.println("---------------UID Format ID Tag-----------------");
  Serial.print("Tag Id: ");
  Serial.println(tagId);
  Serial.print("iS (intersection (0) or station (1)): ");
  Serial.println(iS);
  Serial.print("Station ID: ");
  Serial.println(sID);
  Serial.println("---------------------------------------------------");
#endif
  return true;
}

void ESP32LedCrontrol(int color) {
  switch (color) {
    case 0:  // UIT
      digitalWrite(ESP_RED_PIN, HIGH);
      digitalWrite(ESP_GREEN_PIN, HIGH);
      digitalWrite(ESP_BLUE_PIN, HIGH);
      break;

    case 1:  // ROOD
      digitalWrite(ESP_RED_PIN, LOW);
      digitalWrite(ESP_GREEN_PIN, HIGH);
      digitalWrite(ESP_BLUE_PIN, HIGH);
      break;

    case 2:  // GROEN
      digitalWrite(ESP_RED_PIN, HIGH);
      digitalWrite(ESP_GREEN_PIN, LOW);
      digitalWrite(ESP_BLUE_PIN, HIGH);
      break;

    case 3:  // BLAUW
      digitalWrite(ESP_RED_PIN, HIGH);
      digitalWrite(ESP_GREEN_PIN, HIGH);
      digitalWrite(ESP_BLUE_PIN, LOW);
      break;

    case 4:  // GEEL (rood + groen)
      digitalWrite(ESP_RED_PIN, LOW);
      digitalWrite(ESP_GREEN_PIN, LOW);
      digitalWrite(ESP_BLUE_PIN, HIGH);
      break;

    case 5:  // MAGENTA (rood + blauw)
      digitalWrite(ESP_RED_PIN, LOW);
      digitalWrite(ESP_GREEN_PIN, HIGH);
      digitalWrite(ESP_BLUE_PIN, LOW);
      break;

    case 6:  // CYAAN (groen + blauw)
      digitalWrite(ESP_RED_PIN, HIGH);
      digitalWrite(ESP_GREEN_PIN, LOW);
      digitalWrite(ESP_BLUE_PIN, LOW);
      break;

    case 7:  // WIT (rood + groen + blauw)
      digitalWrite(ESP_RED_PIN, LOW);
      digitalWrite(ESP_GREEN_PIN, LOW);
      digitalWrite(ESP_BLUE_PIN, LOW);
      break;
  }
}
bool intersection( uint16_t nextTagId) {
  for (int i = 0; i < mapLength; i++) {
    if (tagMap[i].tagId == tagId) {
      if (tagMap[i].straightTag == nextTagId) {
        Serial.println("Ga rechtdoor");

        return true;
      } else if (tagMap[i].leftTag == nextTagId) {
        Serial.println("Ga linksaf");
        return true;
      } else if (tagMap[i].rightTag == nextTagId) {
        Serial.println("Ga rechtsaf");

        return true;
      } else if (tagMap[i].backwardTag == nextTagId) {
        Serial.println("Keer om");

        return true;
      } else {
        Serial.println("Volgende tag niet verbonden aan dit kruispunt!");
        return false;
      }
    }
  }
  Serial.println("Huidige tag niet gevonden in kaart!");
  return false;
}

void rfidTagAction() {
  switch (iS) {
    case 0:
      Serial.println("Tag intersection");
      if (intersection(testRoute[routeCounter]))
        routeCounter++;

      break;
    case 1:
      Serial.println("Tag station");

      break;
    default:
      Serial.println("Tag action Unknown");
  }
}


void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Initiate");

  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_NAME, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onNewConnection([](uint32_t nodeId) {
    Serial.printf("New Connection, nodeId = %u\n", nodeId);
  });
  mesh.onChangedConnections([]() {
    Serial.println("Connections changed");
  });
  mesh.onNodeTimeAdjusted([](int32_t offset) {
    Serial.printf("Time adjusted by %d\n", offset);
  });

  // Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LEFT_SENSOR_PIN, INPUT);
  pinMode(RIGHT_SENSOR_PIN, INPUT);
  pinMode(LEFT_MOTOR_DIR, OUTPUT);
  pinMode(RIGHT_MOTOR_DIR, OUTPUT);

  // PWM setup met jouw pinnen
  ledcSetup(LEFT_PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(LEFT_MOTOR, LEFT_PWM_CHANNEL);

  ledcSetup(RIGHT_PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(RIGHT_MOTOR, RIGHT_PWM_CHANNEL);

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);  // SCK, MISO, MOSI, SS
  rfid.PCD_Init();                                 // init MFRC522

  pinMode(ESP_RED_PIN, OUTPUT);
  pinMode(ESP_GREEN_PIN, OUTPUT);
  pinMode(ESP_BLUE_PIN, OUTPUT);
}

void loop() {
  unsigned long currentMillis = millis();
  int Position = 0;

  if (routeCounter == (sizeof(testRoute) + 1)) {
    routeCounter = 0;
  }


  // mesh.                                          ();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (readRFIDReader()) {
      if (formatRfidUid()) {
        rfidTagAction();
        //Position = uidCheck(uid);  // Get position value from uidCheck
        //ESP32LedCrontrol(Position);
        //Stop();
        //delay(5000);
        //ESP32LedCrontrol(6969);
      }
    }
  }
  SensorCheck();
  if (Ultrasoon_Check() < DISTANCE_THRESHOLD_CM) {
    Stop();
  }
}
