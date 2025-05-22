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

// Pin definitions
#define TRIG_PIN         20
#define ECHO_PIN         19
#define LEFT_SENSOR_PIN  17
#define RIGHT_SENSOR_PIN 18
#define PWM_B_PIN        11
#define DIR_B_PIN        4
#define PWM_A_PIN        3
#define DIR_A_PIN        2

// #define RED_PIN          5
// #define GREEN_PIN        6
// #define BLUE_PIN         7

// Motor control constants
#define MOTOR_SPEED           220
#define TURN_SPEED_ADJUSTMENT 30

// Sensor constants
#define DISTANCE_THRESHOLD_CM     20
#define TRIG_PULSE_DURATION_US    10
#define TRIG_PULSE_DELAY_US       2
#define PULSE_TIMEOUT_US          30000

// LED and direction states
#define LED_ON    HIGH
#define LED_OFF   LOW
#define FORWARD  HIGH
#define BACKWARD LOW

// Control state
bool shouldStop = false;

// Sensor readings
int distance;
int leftSensorValue;
int rightSensorValue;

// --- Motor control functions ---
void stopMotors() {
    analogWrite(PWM_A_PIN, 0);
    analogWrite(PWM_B_PIN, 0);
    // digitalWrite(RED_PIN, LED_ON);
    // digitalWrite(GREEN_PIN, LED_OFF);
    // digitalWrite(BLUE_PIN, LED_OFF);
}

void moveForward() {
    digitalWrite(DIR_A_PIN, FORWARD);
    digitalWrite(DIR_B_PIN, FORWARD);
    analogWrite(PWM_A_PIN, MOTOR_SPEED);
    analogWrite(PWM_B_PIN, MOTOR_SPEED);
    // digitalWrite(RED_PIN, LED_OFF);
    // digitalWrite(GREEN_PIN, LED_ON);
    // digitalWrite(BLUE_PIN, LED_OFF);
}

void turnLeft() {
    digitalWrite(DIR_A_PIN, FORWARD);
    digitalWrite(DIR_B_PIN, BACKWARD);
    analogWrite(PWM_A_PIN, MOTOR_SPEED - TURN_SPEED_ADJUSTMENT);
    analogWrite(PWM_B_PIN, MOTOR_SPEED - TURN_SPEED_ADJUSTMENT);
    // digitalWrite(RED_PIN, LED_OFF);
    // digitalWrite(GREEN_PIN, LED_OFF);
    // digitalWrite(BLUE_PIN, LED_ON);
}

void turnRight() {
    digitalWrite(DIR_A_PIN, BACKWARD);
    digitalWrite(DIR_B_PIN, FORWARD);
    analogWrite(PWM_A_PIN, MOTOR_SPEED - TURN_SPEED_ADJUSTMENT);
    analogWrite(PWM_B_PIN, MOTOR_SPEED - TURN_SPEED_ADJUSTMENT);
    // digitalWrite(RED_PIN, LED_OFF);
    // digitalWrite(GREEN_PIN, LED_OFF);
    // digitalWrite(BLUE_PIN, LED_ON);
}

// --- Sensor functions ---
void checkUltrasonicSensor() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(TRIG_PULSE_DELAY_US);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(TRIG_PULSE_DURATION_US);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT_US);
    distance = duration * 0.034 / 2;
}

void checkLineSensors() {
    leftSensorValue = digitalRead(LEFT_SENSOR_PIN);
    rightSensorValue = digitalRead(RIGHT_SENSOR_PIN);
}

void processMotorControl() {
    if ((distance < DISTANCE_THRESHOLD_CM && distance > 0) || shouldStop) {
        stopMotors();
    } else {
        if (leftSensorValue == LOW && rightSensorValue == HIGH) {
            turnRight();
        } else if (leftSensorValue == HIGH && rightSensorValue == LOW) {
            turnLeft();
        } else {
            moveForward();
        }
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

// --- Setup ---
void setup() {
    Serial.begin(115200);

    // Setup pins
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(LEFT_SENSOR_PIN, INPUT);
    pinMode(RIGHT_SENSOR_PIN, INPUT);
    pinMode(PWM_B_PIN, OUTPUT);
    pinMode(DIR_B_PIN, OUTPUT);
    pinMode(PWM_A_PIN, OUTPUT);
    pinMode(DIR_A_PIN, OUTPUT);


    // pinMode(RED_PIN, OUTPUT);
    // pinMode(GREEN_PIN, OUTPUT);
    // pinMode(BLUE_PIN, OUTPUT);

    	SPI.begin();                          // init SPI bus
        rfid.PCD_Init();                      // init MFRC522
}

// --- Loop ---
void loop() {
    checkUltrasonicSensor();
    checkLineSensors();
    processMotorControl();

    int position = 0;

    String uid = readRFIDReader();        // Read RFID reader
    if (uid != "")                        // if their is a uid
    {                     
        position = uidCheck(uid);         // Get position value from uidCheck
    }
}
