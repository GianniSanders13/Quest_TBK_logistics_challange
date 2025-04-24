#include "painlessMesh.h"
#include <QMC5883LCompass.h>

QMC5883LCompass compass;
Scheduler userScheduler;
painlessMesh mesh;

//Mesh network configuration
#define MESH_NAME     "Quest-Network"
#define MESH_PASSWORD "Quest-Password"
#define MESH_PORT      5555
//Ultrasoon
#define TRIG_PIN 20
#define ECHO_PIN 19
//Sensors
#define LEFT_SENSOR_PIN   17
#define MIDDLE_SENSOR_PIN 23
#define RIGHT_SENSOR_PIN  18
//Motors
#define LEFT_MOTOR      3  
#define LEFT_MOTOR_DIR  2   
#define RIGHT_MOTOR     11  
#define RIGHT_MOTOR_DIR 4
//FORWARD OR BACKWARD indication
#define FORWARD   LOW
#define BACKWARD  HIGH
// Motor control parameters
#define MOTOR_SPEED           220  //Max 255, original 220
#define TURN_SPEED_ADJUSTMENT 30
// Sensor and control parameters
#define DISTANCE_THRESHOLD_CM       20
#define ENCODER_MAX_COUNT           45
#define ENCODER_RESET_COUNT         3
#define ENCODER_RESET_INTERVAL_MS   300
#define DELAY_AFTER_MOVE_MS         200
#define SENSOR_READ_DELAY_MS        0  //Note: Gianni has turn off delay it was 10. Its better in following line
#define TRIG_PULSE_DURATION_US      10
#define TRIG_PULSE_DELAY_US         2
#define PULSE_TIMEOUT_US            30000



int Speed = 220;
int NormalAdjust = 660;
int SharpAdjust = 660;
int TurnLRDelay = 0;
int TurnBackDelay = 90;

void Send_Message(String Message) {
  mesh.sendBroadcast(Message);
}

void NetworkUpdate() {
  mesh.update();
}

void NewConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

void ChangedConnectionCallback() {
  Serial.printf("Connections changed\n");
}

void NodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Time adjusted by %d\n", offset);
}

int Ultrasoon_Check() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(TRIG_PULSE_DELAY_US);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(TRIG_PULSE_DURATION_US);
  digitalWrite(TRIG_PIN, LOW);
  int duration = pulseIn(ECHO_PIN, HIGH, PULSE_TIMEOUT_US);
  int distance = duration * 0.034 / 2;
  return distance;
}

void Stop() {
  analogWrite(RIGHT_MOTOR, LOW);
  analogWrite(LEFT_MOTOR, LOW);
}
void Left() {
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  analogWrite(RIGHT_MOTOR, Speed);
  analogWrite(LEFT_MOTOR, Speed - NormalAdjust);
}
void Right() {
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  analogWrite(RIGHT_MOTOR, Speed - NormalAdjust);
  analogWrite(LEFT_MOTOR, Speed);
}
void SharpLeft() {
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  analogWrite(RIGHT_MOTOR, Speed);
  analogWrite(LEFT_MOTOR, Speed - SharpAdjust);
}
void SharpRight() {
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  analogWrite(RIGHT_MOTOR, Speed - SharpAdjust);
  analogWrite(LEFT_MOTOR, Speed);
}
void Forward() {
  digitalWrite(RIGHT_MOTOR_DIR, FORWARD);
  digitalWrite(LEFT_MOTOR_DIR, FORWARD);
  analogWrite(RIGHT_MOTOR, Speed);
  analogWrite(LEFT_MOTOR, Speed);
}
void TurnLeft() {
  digitalWrite(RIGHT_MOTOR_DIR, LOW);
  digitalWrite(LEFT_MOTOR_DIR, HIGH);
  analogWrite(RIGHT_MOTOR, Speed);
  analogWrite(LEFT_MOTOR, Speed);
  delay(TurnLRDelay);
  digitalWrite(RIGHT_MOTOR_DIR, LOW);
  digitalWrite(LEFT_MOTOR_DIR, LOW);
}
void TurnRight() {
  digitalWrite(RIGHT_MOTOR_DIR, HIGH);
  digitalWrite(LEFT_MOTOR_DIR, LOW);
  analogWrite(RIGHT_MOTOR, Speed);
  analogWrite(LEFT_MOTOR, Speed);
  delay(TurnLRDelay);
  digitalWrite(RIGHT_MOTOR_DIR, LOW);
  digitalWrite(LEFT_MOTOR_DIR, LOW);
}
void TurnBack() {
  digitalWrite(RIGHT_MOTOR_DIR, HIGH);
  digitalWrite(LEFT_MOTOR_DIR, LOW);
  analogWrite(RIGHT_MOTOR, Speed);
  analogWrite(LEFT_MOTOR, Speed);
  delay(TurnBackDelay);
  digitalWrite(RIGHT_MOTOR_DIR, LOW);
  digitalWrite(LEFT_MOTOR_DIR, LOW);
}

void SensorCheck() {
  int s1 = !digitalRead(LEFT_SENSOR_PIN);
  int s2 = !digitalRead(MIDDLE_SENSOR_PIN);
  int s3 = !digitalRead(RIGHT_SENSOR_PIN);
  String sData = String(s1) + String(s2) + String(s3);
    //Serial.println(s1);
    //Serial.println(s2);
    //Serial.println(s3); 
  Serial.println(sData);
  int INTsData = sData.toInt();
  switch (INTsData) {
    case 0:  //000
      Stop();
      break;
    case 1:  //001
      SharpRight();
      Serial.println("scherp rechts");
      break;
    case 10:  //010
      Forward();
      Serial.println("rechtdoor");
      break;
    case 11:  //011
      Right();
      Serial.println("rechts");
      break;
    case 100:  //100
      SharpLeft();
      Serial.println("scherp links");
      break;
    case 101:  //101
      //Intersections: MC.TurnLeft or MC.TurnRight or MC.TurnBack
      Stop();
      break;
    case 110:  //110
      Left();
      Serial.println("links");
      break;
    case 111:  //111
      //Intersections: MC.TurnLeft or MC.TurnRight or MC.TurnBack
      Stop();
      break;
  }
}

void setup() {
  Serial.begin(2000000);
  delay(2000);

  Serial.println("Initiate");

  compass.init();

  //Mesh netwerk
  mesh.setDebugMsgTypes(ERROR | STARTUP);
  mesh.init(MESH_NAME, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onNewConnection(&NewConnectionCallback);
  mesh.onChangedConnections(&ChangedConnectionCallback);
  mesh.onNodeTimeAdjusted(&NodeTimeAdjustedCallback);

  //Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LEFT_SENSOR_PIN, INPUT);
  pinMode(MIDDLE_SENSOR_PIN, INPUT);
  pinMode(RIGHT_SENSOR_PIN, INPUT);
  pinMode(LEFT_MOTOR, OUTPUT);
  pinMode(LEFT_MOTOR_DIR, OUTPUT);
  pinMode(RIGHT_MOTOR, OUTPUT);
  pinMode(RIGHT_MOTOR_DIR, OUTPUT);
}

void loop() {
  //NetworkUpdate();
  SensorCheck();
  int Distance = Ultrasoon_Check();
  if (Distance < DISTANCE_THRESHOLD_CM) {
    Stop();
  }
  delay(SENSOR_READ_DELAY_MS);
}
