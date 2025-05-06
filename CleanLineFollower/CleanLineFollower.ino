#include "painlessMesh.h"
#include <QMC5883LCompass.h>
#include <SPI.h>
#include <MFRC522.h>

bool DebugPrint = 1;

// Compass
QMC5883LCompass compass;
Scheduler userScheduler;
painlessMesh mesh;

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
#define SENSOR_READ_DELAY_MS 0

// RFID
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

void setup() {
  Serial.begin(2000000);
  delay(1000);
  Serial.println("Initiate");

  compass.init();

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

  SPI.begin();                          // init SPI bus
  rfid.PCD_Init();                      // init MFRC522

  // ledcSetup(ESP32_RED,   5000, 8);
  // ledcSetup(ESP32_GREEN, 5000, 8);
  // ledcSetup(ESP32_BLUE,  5000, 8);

  // ledcAttachPin(RED_PIN,   ESP32_RED);
  // ledcAttachPin(GREEN_PIN, ESP32_GREEN);
  // ledcAttachPin(BLUE_PIN,  ESP32_BLUE);
}

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
  if (DebugPrint){Serial.println("links sturen");}
}

void Right() {
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  ledcWrite(LEFT_PWM_CHANNEL, Speed);
  ledcWrite(RIGHT_PWM_CHANNEL, Speed - NormalAdjust);
  if (DebugPrint){Serial.println("rechts sturen");}
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
  int s1 = !digitalRead(LEFT_SENSOR_PIN);
  int s2 = !digitalRead(RIGHT_SENSOR_PIN);
  String sData = String(s1) + String(s2);
  if (DebugPrint){Serial.println(sData);}
  int INTsData = sData.toInt();

  switch (INTsData) {
    case 0: Forward(); break;
    case 1:
      Right();
      if (DebugPrint){Serial.println("Rechts");}
      break;
    case 10:
      Left();
      if (DebugPrint){Serial.println("links");}
      break;
    case 11: Stop(); break;
    default: Forward(); break;
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
        if (DebugPrint){Serial.print("RFID reader read: ");}
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
    if (DebugPrint){Serial.print("Tag: ");}
    if (DebugPrint){Serial.print(debugPrint);}
    if (DebugPrint){Serial.print(", Position Value: ");}
    if (DebugPrint){Serial.println(returnValue);}
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
    case 1: // Orange
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 100);
      ledcWrite(ESP32_BLUE,  255);
      break;
    case 2: // Yellow
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 0);
      ledcWrite(ESP32_BLUE,  255);
      break;
    case 3: // Green
      ledcWrite(ESP32_RED,   255);
      ledcWrite(ESP32_GREEN, 0);
      ledcWrite(ESP32_BLUE,  255);
      break;
    case 4: // Light Green
      ledcWrite(ESP32_RED,   200);
      ledcWrite(ESP32_GREEN, 0);
      ledcWrite(ESP32_BLUE,  200);
      break;
    case 5: // Cyan
      ledcWrite(ESP32_RED,   255);
      ledcWrite(ESP32_GREEN, 0);
      ledcWrite(ESP32_BLUE,  0);
      break;
    case 6: // Blue
      ledcWrite(ESP32_RED,   255);
      ledcWrite(ESP32_GREEN, 255);
      ledcWrite(ESP32_BLUE,  0);
      break;
    case 7: // Purple
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 255);
      ledcWrite(ESP32_BLUE,  0);
      break;
    case 8: // Pink
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 200);
      ledcWrite(ESP32_BLUE,  100);
      break;
    case 9: // White
      ledcWrite(ESP32_RED,   0);
      ledcWrite(ESP32_GREEN, 0);
      ledcWrite(ESP32_BLUE,  0);
      break;
    default: // OFF  
      ledcWrite(ESP32_RED,   255);
      ledcWrite(ESP32_GREEN, 255);
      ledcWrite(ESP32_BLUE,  255);
      break;
  }
}

void loop() {
  int Position = 0;

 // mesh.update();
  // String uid = readRFIDReader();
  // if (uid != "")                        // if their is a uid
  // {                     
  //   Position = uidCheck(uid);           // Get position value from uidCheck
  //   //ESP32LedCrontrol(Position);
  //   Stop();
  //   delay(5000);
  //   //ESP32LedCrontrol(6969);
  // }

  SensorCheck();
  if (Ultrasoon_Check() < DISTANCE_THRESHOLD_CM) {
    Stop();
  }
  delay(SENSOR_READ_DELAY_MS);
}
