const int LF_L = 17;
const int LF_R = 18;
const int LF_Z = 23;

void setup() {
  pinMode(LF_L, INPUT);
  pinMode(LF_R, INPUT);
  pinMode(LF_Z, INPUT);
}

void loop() {
  int Links = digitalRead(LF_L);
  Serial.println(Links);
  delay(10);
}
