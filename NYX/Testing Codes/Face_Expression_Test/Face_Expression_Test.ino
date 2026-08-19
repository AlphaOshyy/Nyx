#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


void setup() {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}


void loop() {

  normalEyes();
  delay(1500);

  blinkEyes();
  delay(300);

  normalEyes();
  delay(1000);

  lookLeft();
  delay(800);

  lookRight();
  delay(800);

  happyEyes();
  delay(1500);

}


void drawEye(int x, int y, int w, int h) {

  display.fillRoundRect(x,y,w,h,12,SSD1306_WHITE);

}


void normalEyes() {

  display.clearDisplay();

  drawEye(15,12,38,40);
  drawEye(75,12,38,40);

  display.display();

}


void blinkEyes() {

  display.clearDisplay();

  display.fillRoundRect(15,30,38,8,4,SSD1306_WHITE);
  display.fillRoundRect(75,30,38,8,4,SSD1306_WHITE);

  display.display();

}


void lookLeft() {

  display.clearDisplay();

  drawEye(8,12,38,40);
  drawEye(68,12,38,40);

  display.display();

}


void lookRight() {

  display.clearDisplay();

  drawEye(22,12,38,40);
  drawEye(82,12,38,40);

  display.display();

}


void happyEyes() {

  display.clearDisplay();

  display.drawLine(15,35,25,25,SSD1306_WHITE);
  display.drawLine(25,25,45,35,SSD1306_WHITE);

  display.drawLine(75,35,85,25,SSD1306_WHITE);
  display.drawLine(85,25,105,35,SSD1306_WHITE);

  display.display();

}