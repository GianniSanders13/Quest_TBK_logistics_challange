#include "painlessMesh.h"
#include <QMC5883LCompass.h>

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

#define FORWARD LOW
#define BACKWARD HIGH

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

void setup() {
  Serial.begin(115200);
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
  int s1 = !digitalRead(LEFT_SENSOR_PIN);
  int s2 = !digitalRead(RIGHT_SENSOR_PIN);
  String sData = String(s1) + String(s2);
  Serial.println(sData);
  int INTsData = sData.toInt();

  switch (INTsData) {
    case 0: Forward(); break;
    case 1:
      Right();
      Serial.println("scherp rechts");
      break;
    case 10:
      Left();
      Serial.println("links");
      break;
    case 11: Stop(); break;
    default: Forward(); break;
  }
}

void loop() {
  mesh.update();
  SensorCheck();
  if (Ultrasoon_Check() < DISTANCE_THRESHOLD_CM) {
    Stop();
  }
  delay(SENSOR_READ_DELAY_MS);
}
