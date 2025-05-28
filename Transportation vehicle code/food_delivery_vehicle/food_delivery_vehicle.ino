#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

// --------------------DEBUG --------------------------------------------------------
#define DEBUG true             // Enable or disable all DEBUG prints
#define DEBUG_RFID true        // RFID reader debug (what the readers received)
#define DEBUG_RFID_CHECK true  // DEBUG print of de RFID tag id
#define DEBUG_DRIVING false    // driving direct with sensor values.
#define DEBUG_FORMAT false
#define TAG_ACTION true
#define DEBUG_TAG_STEERING_DIRECTION false
#define DEBUG_ROUTEPREP true
//------------------------------------------------------------------------------------
#define PICK_UP_DELAY 10000
#define PIZZARIA_STATION_ID 4

#define Own_ID_DEF 3
#define Begin_Key_DEF 10
#define End_Key_DEF 5

#define MAX_ROUTE_LENGTH 14
// Ultrasoon
#define TRIG_PIN 20
#define ECHO_PIN 19
// Ultrasoon variables
#define DISTANCE_THRESHOLD_CM 20
#define TRIG_PULSE_DURATION_US 10
#define TRIG_PULSE_DELAY_US 2
#define PULSE_TIMEOUT_US 30000

// Line sensors
#define LEFT_SENSOR_PIN 17
#define RIGHT_SENSOR_PIN 18

// Motors
#define LEFT_MOTOR 3
#define LEFT_MOTOR_DIR 2
#define RIGHT_MOTOR 11
#define RIGHT_MOTOR_DIR 4

// Motor direction
#define FORWARD HIGH
#define BACKWARD LOW

// PWM motor settings
#define PWM_FREQ 1000
#define PWM_RES 8  // 0-255
#define LEFT_PWM_CHANNEL 0
#define RIGHT_PWM_CHANNEL 1

// Motor control settings
#define SPEED 230
#define NORMALADJUST 0
#define STEERING_SPEED 220
#define TURN_AROUND_DELAY 1150
#define TURN_DELAY 600

// RFID Reader
#define SCK_PIN 13   // Serial Clock (SCK)
#define MISO_PIN 12  // Master In Slave Out (MISO)
#define MOSI_PIN 8   // Master Out Slave In (MOSI)
#define SS_PIN 10    // Slave Select (SS)
#define RST_PIN 5    // Reset pin

//ESP32 ledcontrol
#define ESP_RED_PIN 14
#define ESP_GREEN_PIN 15
#define ESP_BLUE_PIN 16
// colors esp led
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

// UID byte defines
#define UIDBYTE0 0
#define UIDBYTE1 1
#define UIDBYTE2 2
#define UIDBYTE3 3
#define SIZE_UID 4
byte uidBytes[SIZE_UID];

// RFID format
uint16_t tagId = 5;
uint8_t iS;
uint8_t sID;
uint16_t oldTagId = 5;

//RFID init
MFRC522 rfid(SS_PIN, RST_PIN);

// timing for RFID reader
unsigned long previousMillis = 0;  // Will store last time RFID is read
const long interval = 200;         // Interval of reading RFID in milliseconds

// RFID maping structure
struct rfidMapStruct {
  uint16_t tagId;
  uint16_t lastTagId;
  uint16_t leftTag;
  uint16_t straightTag;
  uint16_t rightTag;
};
// RFID tag map
rfidMapStruct tagMap[] = {
  // Intersections
  // Tag 1
  { 1, 16, 2, 0, 6 },
  { 1, 6, 16, 2, 0 },
  { 1, 2, 0, 6, 16 },

  // Tag 2
  { 2, 1, 0, 12, 8 },
  { 2, 8, 1, 0, 12 },
  { 2, 12, 8, 1, 0 },

  // Tag 3
  { 3, 12, 0, 13, 9 },
  { 3, 9, 12, 0, 13 },
  { 3, 13, 9, 12, 0 },

  // Tag 4
  { 4, 13, 0, 14, 10 },
  { 4, 10, 13, 0, 14 },
  { 4, 14, 10, 13, 0 },

  // Tag 5
  { 5, 14, 11, 6, 0 },
  { 5, 11, 6, 0, 14 },
  { 5, 6, 0, 14, 11 },

  // Tag 6
  { 6, 5, 15, 1, 7 },
  { 6, 15, 1, 7, 5 },
  { 6, 1, 7, 5, 15 },
  { 6, 7, 5, 15, 1 },

  // Tag 7
  { 7, 6, 8, 0, 10 },
  { 7, 8, 0, 10, 6 },
  { 7, 10, 6, 8, 0 },

  // Tag 8
  { 8, 7, 2, 9, 0 },
  { 8, 2, 9, 0, 7 },
  { 8, 9, 0, 7, 2 },

  // Tag 9
  { 9, 8, 3, 17, 0 },
  { 9, 17, 0, 8, 3 },
  { 9, 3, 17, 0, 8 },

  // Tag 10
  { 10, 17, 4, 7, 0 },
  { 10, 4, 7, 0, 17 },
  { 10, 7, 0, 17, 4 },

  // Tag 11
  { 11, 16, 15, 5, 0 },
  { 11, 15, 5, 0, 16 },
  { 11, 5, 0, 16, 15 },

  // stations
  // Tag 12
  { 12, 2, 0, 3, 0 },
  { 12, 3, 0, 2, 0 },

  // Tag 13 (blauw station)
  { 13, 3, 0, 4, 0 },
  { 13, 4, 0, 3, 0 },

  // Tag 14 (blauw station)
  { 14, 4, 0, 5, 0 },
  { 14, 5, 0, 4, 0 },

  // Tag 15 (blauw station)
  { 15, 11, 0, 6, 0 },
  { 15, 6, 0, 11, 0 },


  // Tag 16 (blauw station)
  { 16, 11, 0, 1, 0 },
  { 16, 1, 0, 11, 0 },

  // Tag 17 (blauw station)
  { 17, 10, 0, 9, 0 },
  { 17, 9, 0, 10, 0 }

};

const int mapLength = sizeof(tagMap) / sizeof(tagMap[0]);

char routeCounter = 0;
// uint16_t testRoute[] = { 15, 6, 7, 8, 2, 12, 3, 13, 4, 14, 5, 11 };  // for testing
// uint8_t routeSize = sizeof(testRoute) / sizeof(testRoute[0]);

struct rfidRouteMapStruct {
  uint8_t RouteIndication;
  uint16_t route[MAX_ROUTE_LENGTH];
  uint8_t length;
};

rfidRouteMapStruct routeMap[] = {
  { 0, { 15 }, 1 },
  { 1, { 6, 1, 2, 12, 3, 13, 4, 14, 5, 11, 15 }, 11 },
  { 2, { 6, 7, 8, 9, 17, 10, 4, 14, 5, 11, 15 }, 11 },
  { 3, { 6, 7, 8, 2, 12, 3, 13, 4, 14, 5, 11, 15 }, 12 },
  { 4, { 6, 7, 8, 9, 3, 13, 4, 14, 5, 11, 15 }, 11 },
  { 5, { 6, 7, 8, 2, 12, 3, 9, 17, 10, 4, 14, 5, 11, 15 }, 14 },
  { 6, { 6, 1, 2, 8, 9, 3, 13, 4, 14, 5, 11, 15 }, 12 },
  { 7, { 6, 1, 2, 12, 3, 9, 17, 10, 4, 14, 5, 11, 15 }, 13 },
  { 8, { 6, 1, 2, 8, 9, 17, 10, 4, 14, 5, 11, 15 }, 12 }

};
const int RouteMaplength = sizeof(routeMap) / sizeof(routeMap[0]);

uint16_t CurrentRoute[MAX_ROUTE_LENGTH];
uint8_t routelength;

typedef struct Message {
  uint8_t Begin_Key;
  uint8_t Dest_ID;
  uint8_t Source_ID;
  uint8_t Message_Kind;
  uint8_t Data1;  //(Route)
  uint8_t Data2;  //(1e station > 0)
  uint8_t Data3;  //(2e station > 0, of 0 wanneer nvt)
  uint8_t Data4;  //(3e station > 0, of 0 wanneer nvt)
  uint8_t Data5;  //(4e station > 0, of 0 wanneer nvt)
  uint8_t End_Key;
} Message;

// struct for storing message
Message IncomingMessage;
Message StoredMessage;

// bool for new message flag
bool NewStoredMessage = false;
// bool for pizzastation flag
bool PizzariaStation = false;

uint8_t routeIndicator = 0;
uint8_t stop1;
uint8_t stop2;
uint8_t stop3;
uint8_t stop4;

// --- Motor functies ---
void Stop();
void Forward();
void Left();
void Right();
void TurnAround();
void TurnLeft();
void TurnRight();

// --- Sensor functies ---
int Ultrasoon_Check();
void SensorCheck();

// --- RFID functies ---
bool readRFIDReader();
bool formatRfidUid();

// --- ESP32 LED control ---
void ESP32LedCrontrol(int color);

// Callback voor ESP-NOW
void OnDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len);
// Berichten decoderen
void formatMessage();

// arduinoooooooooo
void setup();
void loop();
//-----------------------------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  delay(5000);

  WiFi.mode(WIFI_STA);
  Serial.println("Wifi Started");
  Serial.print("MAC adres receiver: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init mislukt");
    return;
  }

  esp_now_register_recv_cb(OnDataReceive);
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
  //rfid.PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_max);

  pinMode(ESP_RED_PIN, OUTPUT);
  pinMode(ESP_GREEN_PIN, OUTPUT);
  pinMode(ESP_BLUE_PIN, OUTPUT);
  routePrep();
}

void loop() {
  unsigned long currentMillis = millis();
  int Position = 0;

  while (PizzariaStation) {  // if by the pizza station wait till new message
    Stop();
    if (NewStoredMessage) {
      formatMessage();
      routePrep();
    }
  }

  // if (routeCounter == routeSize) {
  //   routeCounter = 0;
  // }

  // mesh.                                          ();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (readRFIDReader()) {
      if (formatRfidUid()) {
        rfidTagAction();
      }
    }
  }

  SensorCheck();
  if (Ultrasoon_Check() < DISTANCE_THRESHOLD_CM) {
    Stop();
  }
}

//-----------------------------------Wifi---------------------------------------------------------------
void OnDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&IncomingMessage, incomingData, sizeof(IncomingMessage));
  Serial.println("Bericht ontvangen:");
  if (IncomingMessage.Begin_Key == Begin_Key_DEF && IncomingMessage.End_Key == End_Key_DEF) {
    if (PizzariaStation) {
      NewStoredMessage = true;
      StoredMessage = IncomingMessage;
      PizzariaStation = false;
    } else {
      NewStoredMessage = false;
    }
    Serial.printf("Van: %d -> Voor: %d\n", IncomingMessage.Source_ID, IncomingMessage.Dest_ID);
    Serial.printf("Type: %d\n", IncomingMessage.Message_Kind);
    Serial.printf("Eerste station: %d, Hoeveel: %d\n", IncomingMessage.Data1, IncomingMessage.Data2);
    Serial.printf("Tweede station: %d, Hoeveel: %d\n", IncomingMessage.Data3, IncomingMessage.Data4);
  } else {
    Serial.println("Ongeldig berichtformaat");
  }
}

// // Format incomming messages
void formatMessage() {
  routeIndicator = StoredMessage.Data1;
  stop1 = StoredMessage.Data2;
  stop2 = StoredMessage.Data3;
  stop3 = StoredMessage.Data4;
  stop4 = StoredMessage.Data5;
}

// --- Motor functies ---
void Stop() {
  ledcWrite(LEFT_PWM_CHANNEL, 0);
  ledcWrite(RIGHT_PWM_CHANNEL, 0);
}

void Forward() {
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  ledcWrite(LEFT_PWM_CHANNEL, SPEED);
  ledcWrite(RIGHT_PWM_CHANNEL, SPEED);
}

void Left() {
  digitalWrite(LEFT_MOTOR_DIR, BACKWARD);
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  ledcWrite(LEFT_PWM_CHANNEL, NORMALADJUST);
  ledcWrite(RIGHT_PWM_CHANNEL, STEERING_SPEED);
}

void Right() {
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  digitalWrite(RIGHT_MOTOR_DIR, BACKWARD);
  ledcWrite(LEFT_PWM_CHANNEL, STEERING_SPEED);
  ledcWrite(RIGHT_PWM_CHANNEL, NORMALADJUST);
}

void TurnAround() {
  digitalWrite(LEFT_MOTOR_DIR, BACKWARD);
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  ledcWrite(LEFT_PWM_CHANNEL, STEERING_SPEED);
  ledcWrite(RIGHT_PWM_CHANNEL, STEERING_SPEED);
  delay(TURN_AROUND_DELAY);
}

void TurnLeft() {
  digitalWrite(LEFT_MOTOR_DIR, BACKWARD);
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  ledcWrite(LEFT_PWM_CHANNEL, STEERING_SPEED);
  ledcWrite(RIGHT_PWM_CHANNEL, STEERING_SPEED);
  delay(TURN_DELAY);
}

void TurnRight() {
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  digitalWrite(RIGHT_MOTOR_DIR, BACKWARD);
  ledcWrite(LEFT_PWM_CHANNEL, STEERING_SPEED);
  ledcWrite(RIGHT_PWM_CHANNEL, STEERING_SPEED);
  delay(TURN_DELAY);
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
      Forward();
      debugMessage = "Forward";
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

bool readRFIDReader() {

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
  oldTagId = tagId;
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
  Serial.print("OldTagId: ");
  Serial.println(oldTagId);
  Serial.println("---------------------------------------------------");
#endif
  return true;
}

void routePrep() {
  char route = 0;
  for (int i = 0; i < RouteMaplength; i++) {
    if (routeMap[i].RouteIndication == routeIndicator) {
      route = i;
      routelength = routeMap[i].length;
      for (uint8_t j = 0; j < routelength; j++)
        CurrentRoute[j] = routeMap[i].route[j];
    }
  }
  routeCounter = 0;
#if DEBUG && DEBUG_ROUTEPREP
  Serial.print("Route: ");
  Serial.println(route);
  Serial.print("CurrentRoute (length: ");
  Serial.print(routelength);
  Serial.print("): ");
  for (uint8_t j = 0; j < routelength; j++) {
    Serial.print(CurrentRoute[j]);
    if (j < routelength - 1) Serial.print(" -> ");
  }
  Serial.println();
#endif
}

bool intersection(uint16_t nextTagId) {
  String debugMessage = "tag not found in bitmap";
  bool returnValue;
  for (int i = 0; i < mapLength; i++) {
    if (tagMap[i].tagId == tagId && tagMap[i].lastTagId == oldTagId) {  //
      if (tagMap[i].straightTag == nextTagId) {
        debugMessage = "Go forward to tag";
        Forward();
        ESP32LedCrontrol(2);  // groen
        returnValue = true;
      } else if (tagMap[i].leftTag == nextTagId) {
        debugMessage = "Go left to tag";
        TurnLeft();
        ESP32LedCrontrol(3);  // blauw
        returnValue = true;
      } else if (tagMap[i].rightTag == nextTagId) {
        debugMessage = "Go right to tag";
        TurnRight();
        returnValue = true;
      } else if (tagMap[i].lastTagId == nextTagId) {
        debugMessage = "Turn around for tag";
        TurnAround();
        ESP32LedCrontrol(5);  // paars
        returnValue = true;
      } else {
        debugMessage = "Next tag not connected to this intersection";
        ESP32LedCrontrol(1);  // rood
        returnValue = false;
      }
    }
  }
#if DEBUG && DEBUG_TAG_STEERING_DIRECTION
  Serial.print("Direction on tag: ");
  Serial.println(debugMessage);
  Serial.println();
#endif

  return returnValue;
}

void rfidTagAction() {
  String debugMessage;
  switch (iS) {
    case 0:
      Stop();
      debugMessage = "Tag intersection";
      if (intersection(CurrentRoute[routeCounter]))
        routeCounter++;
      break;
    case 1:
      Stop();
      debugMessage = "Tag station";

      if (sID == stop1 || sID == stop2 || sID == stop3 || sID == stop4) {
        delay(PICK_UP_DELAY);
        Serial.println("Hier pizza op je focus");
      } else if (sID == PIZZARIA_STATION_ID) {
        PizzariaStation = true;
        Serial.println("jum jum pizza");
        break;

      } else if (intersection(CurrentRoute[routeCounter])) {
        routeCounter++;
        Serial.print("ben ik er nou nog niet");
      }
      break;
    default:
      debugMessage = "Tag action Unknown";
  }

#if DEBUG && TAG_ACTION
  Serial.print("Tag action: ");
  Serial.println(debugMessage);
  Serial.println();
#endif
}
