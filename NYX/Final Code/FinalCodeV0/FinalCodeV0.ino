#include <Wire.h>
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Servo panServo;
Servo tiltServo;

const int trigPin = 2;
const int echoPin = 3;


void setup() {

  Serial.begin(9600);

  panServo.attach(9);
  tiltServo.attach(10);

  panServo.write(90);
  tiltServo.write(90);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

}


void loop() {

  float distance = getDistance();

  Serial.println(distance);


  if(distance < 50) {

    curiousEyes();

    panServo.write(90);
    tiltServo.write(80);

    delay(500);

  }
  else {

    idleEyes();

    tiltServo.write(90);

    scanHead();

  }

}


float getDistance(){

  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin,LOW);

  long duration = pulseIn(echoPin,HIGH,30000);

  if(duration == 0)
    return 999;

  return duration * 0.0343 / 2;

}


void scanHead(){

  panServo.write(60);
  delay(400);

  panServo.write(90);
  delay(300);

  panServo.write(120);
  delay(400);

}


void idleEyes(){

  display.clearDisplay();

  display.fillRoundRect(18,14,32,36,10,SSD1306_WHITE);
  display.fillRoundRect(78,14,32,36,10,SSD1306_WHITE);

  display.display();

}


void curiousEyes(){

  display.clearDisplay();

  display.fillRoundRect(14,8,38,46,12,SSD1306_WHITE);
  display.fillRoundRect(76,8,38,46,12,SSD1306_WHITE);

  display.display();

}