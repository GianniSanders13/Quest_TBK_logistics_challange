const int L_Motor_PWM = 3;
const int L_Motor_DIR = 2;

const int R_Motor_PWM = 11;
const int R_Motor_DIR = 4;

void setup() {
  pinMode(L_Motor_PWM, OUTPUT);
  pinMode(L_Motor_DIR, OUTPUT);
  pinMode(R_Motor_PWM, OUTPUT);
  pinMode(R_Motor_DIR, OUTPUT);
}

void loop() {
  digitalWrite(L_Motor_DIR, HIGH);
  analogWrite(L_Motor_PWM, 255);

  digitalWrite(R_Motor_DIR, LOW);
  analogWrite(R_Motor_PWM, 255);

  delay(2000);

  analogWrite(L_Motor_PWM, 0);
  analogWrite(R_Motor_PWM, 0);

  delay(2000);
} //test
