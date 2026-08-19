const int soundPin = A0;

void setup() {
  Serial.begin(9600);
  Serial.println("NYX Sound Test");
}

void loop() {

  int soundLevel = analogRead(soundPin);

  Serial.print("Sound: ");
  Serial.println(soundLevel);

  delay(50);
}