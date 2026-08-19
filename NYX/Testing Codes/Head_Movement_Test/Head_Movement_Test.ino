#include <Servo.h>

Servo panServo;
Servo tiltServo;

void setup() {
  Serial.begin(9600);

  panServo.attach(9);
  tiltServo.attach(10);

  panServo.write(90);
  tiltServo.write(90);

  Serial.println("NYX Head Test");
  delay(1000);
}

void loop() {

  // Look Left
  Serial.println("Looking Left");
  panServo.write(30);
  delay(1000);

  // Center
  panServo.write(80);
  delay(700);

  // Look Right
  Serial.println("Looking Right");
  panServo.write(130);
  delay(1000);

  // Center
  panServo.write(80);
  delay(700);

  // Look Up
  Serial.println("Looking Up");
  tiltServo.write(50);
  delay(1000);

  // Center
  tiltServo.write(90);
  delay(700);

  // Look Down
  Serial.println("Looking Down");
  tiltServo.write(140);
  delay(1000);

  // Center
  tiltServo.write(90);
  delay(1500);
}