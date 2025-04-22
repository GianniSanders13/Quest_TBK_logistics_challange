const int ECHO = 19;
const int TRIG = 20;
float Tijd, Afstand;
//const int SDA = 21;
//const int SCL = 22;

void setup() {
  pinMode(ECHO, INPUT);  
	pinMode(TRIG, OUTPUT);  
	Serial.begin(9600); 
}

void loop() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  Tijd = pulseIn(ECHO, HIGH);
  Afstand = (Tijd*.0343)/2;
  Serial.println(Afstand);
  delay(100);
}
