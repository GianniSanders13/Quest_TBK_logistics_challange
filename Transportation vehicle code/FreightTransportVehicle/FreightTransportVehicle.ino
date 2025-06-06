#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

// --------------------DEBUG --------------------------------------------------------

#define DEBUG false              // Enable or disable all DEBUG prints
#define DEBUG_RFID true         // RFID reader debug (what the readers received)
#define DEBUG_RFID_CHECK false  // DEBUG print of de RFID tag id
#define DEBUG_DRIVING false     // driving direct with sensor values.
#define DEBUG_FORMAT false
#define TAG_ACTION true
//------------------------------------------------------------------------------------

#define Own_ID_DEF 3
#define Begin_Key_DEF 10
#define End_Key_DEF 5
// Ultrasoon
#define TRIG_PIN 20
#define ECHO_PIN 19

// ESP32 pins
#define ESP_RED_PIN 14
#define ESP_GREEN_PIN 15
#define ESP_BLUE_PIN 16

// color defines ESP32 led function 
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

// Sensors
#define LEFT_SENSOR_PIN 17
#define RIGHT_SENSOR_PIN 18

// Motors
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

// Motor control settings
#define SPEED 225
#define NORMALADJUST 0
#define STEERING_SPEED 240
#define TURN_AROUND_DELAY 1200
#define TURN_DELAY 600

#define DISTANCE_THRESHOLD_CM 20
#define TRIG_PULSE_DURATION_US 10
#define TRIG_PULSE_DELAY_US 2
#define PULSE_TIMEOUT_US 30000

// STATION ID'S
#define PEPPERONI 8
#define DEEG 9
#define TOMAAT 10
#define KAAS 11
#define KIP 12

// Delay how long takes the packets is picked up
#define DELAY_1_Packets 2000
#define DELAY_2_Packets 2500
#define DELAY_3_Packets 3000
#define DELAY_4_Packets 3500
#define DELAY_5_Packets 4000

// RFID connection pins
#define SCK_PIN 13   // Serial Clock (SCK)
#define MISO_PIN 12  // Master In Slave Out (MISO)
#define MOSI_PIN 8   // Master Out Slave In (MOSI)
#define SS_PIN 10    // Slave Select (SS)
#define RST_PIN 5

// definition byte order RFID
#define UIDBYTE0 0
#define UIDBYTE1 1
#define UIDBYTE2 2
#define UIDBYTE3 3
// uid byte size

#define SIZE_UID 4
// uid byte

byte uidBytes[SIZE_UID];

// Format uid variables
uint16_t tagId;
uint8_t iS;
uint8_t sID;

// init RFID
MFRC522 rfid(SS_PIN, RST_PIN);

// For timing RFID read
unsigned long previousMillis = 0;  // Will store last time RFID is read
const long interval = 200;         // Interval of reading RFID in milliseconds

// Incomming wifi message struct
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

// struct for storing message
Message IncomingMessage;
Message StoredMessage;

// bool for new message flag
bool NewStoredMessage = false;
// bool for pizzastation flag

bool PizzariaStation = false;

// fomat Wifi message variables
uint8_t F_Station = 0;
uint8_t F_Amount = 0;
uint8_t S_Station = 0;
uint8_t S_Amount = 0;

// ---------- Functieprototypes ----------
// Callback voor ESP-NOW
void OnDataReceive(const uint8_t *mac, const uint8_t *incomingData, int len);
// Berichten decoderen
void DecodeMessage();
uint8_t StationDecode(uint8_t Station);
// Motorfuncties
void Stop();
void Forward();
void Left();
void Right();
void TurnAround();
void TurnLeft();
void TurnRight();
// Ultrasoonfunctie
int Ultrasoon_Check();
// Line follow functie
void SensorCheck();
// RFID functies
bool readRFIDReader();
bool formatRfidUid();
void rfidTagAction();
void stationAction(uint8_t Amount);
// arduinoooooooooo
void setup();
void loop();

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  Serial.println("Wifi Started");
  Serial.print("MAC adres receiver: ");
  Serial.println(WiFi.macAddress());

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init mislukt");
    return;
  }

  esp_now_register_recv_cb(OnDataReceive);

  // Ultrasoon
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Motor
  pinMode(LEFT_SENSOR_PIN, INPUT);
  pinMode(RIGHT_SENSOR_PIN, INPUT);
  pinMode(LEFT_MOTOR_DIR, OUTPUT);
  pinMode(RIGHT_MOTOR_DIR, OUTPUT);

  // PWM motot
  ledcSetup(LEFT_PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(LEFT_MOTOR, LEFT_PWM_CHANNEL);
  ledcSetup(RIGHT_PWM_CHANNEL, PWM_FREQ, PWM_RES);
  ledcAttachPin(RIGHT_MOTOR, RIGHT_PWM_CHANNEL);

  // RFID reader
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);  // SCK, MISO, MOSI, SS
  rfid.PCD_Init();                                 // init MFRC522
  //rfid.PCD_SetAntennaGain(MFRC522::PCD_RxGain::RxGain_max);

  // ESP32 led
  pinMode(ESP_RED_PIN, OUTPUT);
  pinMode(ESP_GREEN_PIN, OUTPUT);
  pinMode(ESP_BLUE_PIN, OUTPUT);

  Serial.println("Setup complete \n");
}

void loop() {
  unsigned long currentMillis = millis();  // keep time for RFID

  while (PizzariaStation) {  // if by the pizza station wait till new message
    Stop();
    if (NewStoredMessage) {
      DecodeMessage();
    }
  }

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (readRFIDReader()) {
      if (formatRfidUid()) {
        rfidTagAction();
      }
    }
  }

  if (Ultrasoon_Check() < DISTANCE_THRESHOLD_CM) {
    Stop();
  }

  SensorCheck();  // Line following
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

// Format incomming messages
void DecodeMessage() {
  F_Station = StationDecode(StoredMessage.Data1);
  F_Amount = StoredMessage.Data2;
  S_Station = StationDecode(StoredMessage.Data3);
  S_Amount = StoredMessage.Data4;
}

// Format station input to RFID station tag id
uint8_t StationDecode(uint8_t Station) {
  switch (Station) {
    case 0:
      Station = NULL;
      break;
    case 1:
      Station = PEPPERONI;
      break;
    case 2:
      Station = DEEG;
      break;
    case 3:
      Station = TOMAAT;
      break;
    case 4:
      Station = KAAS;
      break;
    case 5:
      Station = KIP;
      break;
    default:
      Station = NULL;
      break;
  }
  return Station;
}

// -----------------------------------RFID--------------------------------------------
// RFID read function
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

// RFID UID formater
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

// Logical RFID action
void rfidTagAction() {
  String debugMessage;
  if (sID == F_Station) {
    stationAction(F_Amount);
  }
  if (sID == S_Station) {
    stationAction(S_Amount);
  }

  if (sID == 7) {
    PizzariaStation = true;
  } else {
    PizzariaStation = false;
  }


#if DEBUG && TAG_ACTION
  Serial.print("Tag action: ");
  Serial.println(debugMessage);
  Serial.println();
#endif
}

void stationAction(uint8_t Amount) {
  switch (Amount) {
    case 0:
      Stop();
      break;
    case 1:
      Stop();
      delay(DELAY_1_Packets);
      break;
    case 2:
      Stop();
      delay(DELAY_2_Packets);
      break;
    case 3:
      Stop();
      delay(DELAY_3_Packets);
      break;
    case 4:
      Stop();
      delay(DELAY_4_Packets);
      break;
    case 5:
      Stop();
      delay(DELAY_5_Packets);
      break;
    default:
      Stop();
      break;
  }
}
//-----------------------------------------------------------------------------------------

// Line following sensor check and response
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
//------------------------------------------------

// Distance check
int Ultrasoon_Check() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(TRIG_PULSE_DELAY_US);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(TRIG_PULSE_DURATION_US);
  digitalWrite(TRIG_PIN, LOW);
  int duration = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT_US);
  return duration * 0.034 / 2;
}