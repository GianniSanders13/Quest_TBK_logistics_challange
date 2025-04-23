#include "painlessMesh.h"
#include <QMC5883LCompass.h>

QMC5883LCompass compass;
Scheduler userScheduler;
painlessMesh mesh;

//Mesh network configuration
#define MESH_NAME "Quest-Network"
#define MESH_PASSWORD "Quest-Password"
#define MESH_PORT 5555
//Ultrasoon
#define TRIG_PIN 20
#define ECHO_PIN 19
//Sensors
#define MIDDLE_SENSOR_PIN 23
#define LEFT_SENSOR_PIN 17
#define RIGHT_SENSOR_PIN 18
//Motors
#define PWM_B_PIN 11
#define DIR_B_PIN 4
#define PWM_A_PIN 3
#define DIR_A_PIN 2
// Motor control parameters
#define MOTOR_SPEED 220 //Max 255, original 220
#define TURN_SPEED_ADJUSTMENT 30
// Sensor and control parameters
#define DISTANCE_THRESHOLD_CM 20
#define ENCODER_MAX_COUNT 45
#define ENCODER_RESET_COUNT 3
#define ENCODER_RESET_INTERVAL_MS 300
#define DELAY_AFTER_MOVE_MS 200
#define SENSOR_READ_DELAY_MS 0  //Note: Gianni has turn off delay it was 10. Its better in following line 
#define TRIG_PULSE_DURATION_US 10
#define TRIG_PULSE_DELAY_US 2
#define PULSE_TIMEOUT_US 30000
//FORWARD OR BACKWARD indication 
#define FORWARD HIGH
#define BACKWARD LOW

int Speed = 220;
int NormalAdjust = 30;
int SharpAdjust = 60;
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
  analogWrite(PWM_A_PIN, LOW);
  analogWrite(PWM_B_PIN, LOW);
}
void Left() {
  digitalWrite(DIR_A_PIN, HIGH);
  digitalWrite(DIR_B_PIN, HIGH);
  analogWrite(PWM_A_PIN, Speed - NormalAdjust);
  analogWrite(PWM_B_PIN, Speed - NormalAdjust);
}
void Right() {
  digitalWrite(DIR_A_PIN, HIGH);
  digitalWrite(DIR_B_PIN, HIGH);
  analogWrite(PWM_A_PIN, Speed - NormalAdjust);
  analogWrite(PWM_B_PIN, Speed - NormalAdjust);   
}
void SharpLeft() {
  digitalWrite(DIR_A_PIN, HIGH);
  digitalWrite(DIR_B_PIN, HIGH);
  analogWrite(PWM_A_PIN, Speed - SharpAdjust);
  analogWrite(PWM_B_PIN, Speed - SharpAdjust);
}
void SharpRight() {
  digitalWrite(DIR_A_PIN, HIGH);
  digitalWrite(DIR_B_PIN, HIGH);
  analogWrite(PWM_A_PIN, Speed - SharpAdjust);
  analogWrite(PWM_B_PIN, Speed - SharpAdjust);
}
void Forward() {
  digitalWrite(DIR_A_PIN, HIGH);
  digitalWrite(DIR_B_PIN, HIGH);
  analogWrite(PWM_A_PIN, Speed);
  analogWrite(PWM_B_PIN, Speed);
}
void TurnLeft() {
  digitalWrite(DIR_A_PIN, LOW);
  digitalWrite(DIR_B_PIN, HIGH);
  analogWrite(PWM_A_PIN, Speed);
  analogWrite(PWM_B_PIN, Speed);
  delay(TurnLRDelay);
  digitalWrite(DIR_A_PIN, LOW);
  digitalWrite(DIR_B_PIN, LOW);
}
void TurnRight() {
  digitalWrite(DIR_A_PIN, HIGH);
  digitalWrite(DIR_B_PIN, LOW);
  analogWrite(PWM_A_PIN, Speed);
  analogWrite(PWM_B_PIN, Speed);
  delay(TurnLRDelay);
  digitalWrite(DIR_A_PIN, LOW);
  digitalWrite(DIR_B_PIN, LOW);
}
void TurnBack() {
  digitalWrite(DIR_A_PIN, HIGH);
  digitalWrite(DIR_B_PIN, LOW);
  analogWrite(PWM_A_PIN, Speed);
  analogWrite(PWM_B_PIN, Speed);
  delay(TurnBackDelay);
  digitalWrite(DIR_A_PIN, LOW);
  digitalWrite(DIR_B_PIN, LOW);    
}

void SensorCheck() {
    int s1 = digitalRead(LEFT_SENSOR_PIN);
    int s2 = digitalRead(MIDDLE_SENSOR_PIN);
    int s3 = digitalRead(RIGHT_SENSOR_PIN);
    String sData = String(s1) + String(s2) + String(s3);
    int INTsData = sData.toInt();
    switch(INTsData) {
      case 0:   //000
        Stop();
        break;
      case 1:   //001
        SharpRight();
        break;
      case 10:  //010
        Forward();
        break;
      case 11:  //011
        Right();
        break;
      case 100: //100
        SharpLeft();
        break;
      case 101: //101
        //Intersections: MC.TurnLeft or MC.TurnRight or MC.TurnBack
        Stop();
        break;
      case 110: //110
        Left();
        break;
      case 111: //111
        //Intersections: MC.TurnLeft or MC.TurnRight or MC.TurnBack
        Stop();
        break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.printf("Initiate");

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
    pinMode(PWM_B_PIN, OUTPUT);
    pinMode(DIR_B_PIN, OUTPUT);
    pinMode(PWM_A_PIN, OUTPUT);
    pinMode(DIR_A_PIN, OUTPUT);
}

void loop() {
  NetworkUpdate();
  SensorCheck();
  int Distance = Ultrasoon_Check();
  if (Distance < DISTANCE_THRESHOLD_CM) {
  Stop();
  }
  delay(SENSOR_READ_DELAY_MS);
}
