#include <Wire.h>
#include <Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Servo panServo;

#define TRIG 2
#define ECHO 3

int currentPan = 90;
int targetPan = 90;

unsigned long lastBlink = 0;
unsigned long lastScan = 0;
unsigned long idleStart = 0;

bool personDetected = false;


enum Emotion {
  HAPPY,
  CURIOUS,
  SLEEPY,
  ANGRY
};

Emotion emotion = HAPPY;



void setup() {

  Serial.begin(9600);

  panServo.attach(9);
  panServo.write(90);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  randomSeed(analogRead(A0));

}



void loop() {

  float distance = getDistance();


  if(distance < 60) {

    emotion = CURIOUS;
    targetPan = 90;
    idleStart = millis();

  }
  else {

    if(millis() - idleStart > 20000)
      emotion = SLEEPY;
    else
      emotion = HAPPY;


    if(millis() - lastScan > 3000) {

      targetPan = random(65,115);

      lastScan = millis();

    }

  }


  smoothServo();


  drawEyes();


  // Blink only when not sleeping
  if(emotion != SLEEPY) {

    if(millis() - lastBlink > random(4000,8000)) {

      blink();

      lastBlink = millis();

    }

  }

}



float getDistance() {

  digitalWrite(TRIG,LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG,HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG,LOW);


  long time = pulseIn(ECHO,HIGH,30000);

  if(time == 0)
    return 999;


  return time * 0.034 / 2;

}



void smoothServo() {

  if(currentPan < targetPan)
    currentPan++;

  if(currentPan > targetPan)
    currentPan--;


  panServo.write(currentPan);

  delay(10);

}



void drawEyes() {

  display.clearDisplay();


  int eyeMove = map(currentPan,65,115,-5,5);


  if(emotion == HAPPY) {

    drawHappyEye(18 + eyeMove,18);
    drawHappyEye(78 + eyeMove,18);

  }


  if(emotion == CURIOUS) {

    drawNormalEye(15 + eyeMove,10,36,45);
    drawNormalEye(77 + eyeMove,10,36,45);

  }


  if(emotion == SLEEPY) {

    drawSleepEye(18 + eyeMove,30);
    drawSleepEye(78 + eyeMove,30);

  }


  if(emotion == ANGRY) {

    drawAngryEye(18 + eyeMove,18);
    drawAngryEye(78 + eyeMove,18);

  }


  display.display();

}



void drawNormalEye(int x,int y,int w,int h) {

  display.fillRoundRect(x,y,w,h,12,SSD1306_WHITE);

}



void drawHappyEye(int x,int y) {

  display.drawLine(x,y+25,x+15,y+10,SSD1306_WHITE);
  display.drawLine(x+15,y+10,x+32,y+25,SSD1306_WHITE);

}



void drawSleepEye(int x,int y) {

  display.fillRoundRect(x,y,32,6,3,SSD1306_WHITE);

}



void drawAngryEye(int x,int y) {

  display.drawLine(x,y+5,x+35,y+20,SSD1306_WHITE);

  display.drawLine(x,y+20,x+35,y+5,SSD1306_WHITE);

}



void blink() {

  display.clearDisplay();


  display.fillRoundRect(18,30,32,5,3,SSD1306_WHITE);
  display.fillRoundRect(78,30,32,5,3,SSD1306_WHITE);

  display.display();

  delay(120);

}