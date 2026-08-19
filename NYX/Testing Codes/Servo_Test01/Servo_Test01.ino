#include <Servo.h>

Servo servo;

void setup() {
  Serial.begin(9600);

  servo.attach(9);
  servo.write(90);

  Serial.println("=== NYX Servo Test 1 ===");
  delay(1000);
}

void loop() {

  Serial.println("Center");
  servo.write(90);
  delay(1000);

  Serial.println("Left");
  servo.write(0);
  delay(1000);

  Serial.println("Center");
  servo.write(90);
  delay(1000);

  Serial.println("Right");
  servo.write(180);
  delay(1000);
}