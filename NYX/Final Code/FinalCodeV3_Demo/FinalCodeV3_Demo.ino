#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// =====================================================
// NYX V3 - HARDWARE-LIMITED DEMO
// Real ultrasonic sensing + existing NYX eye designs.
// Body/wheel movement is NOT used because the movement
// hardware/battery is unavailable.
// =====================================================

#define TRIG_PIN 2
#define ECHO_PIN 3
#define PAN_SERVO 9
#define TILT_SERVO 10

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Servo panServo;
Servo tiltServo;

int currentPan = 90;
int currentTilt = 90;

const int CENTER_PAN = 90;
const int LEFT_PAN = 72;
const int RIGHT_PAN = 108;
const int CENTER_TILT = 90;
const int CURIOUS_TILT = 105;

const float OBJECT_DISTANCE_CM = 40.0;

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  panServo.attach(PAN_SERVO);
  tiltServo.attach(TILT_SERVO);
  panServo.write(CENTER_PAN);
  tiltServo.write(CENTER_TILT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED ERROR");
    while (1);
  }

  display.clearDisplay();
  display.display();

  Serial.println("=================================");
  Serial.println("NYX V3 REAL SENSOR DEMO");
  Serial.println("Ultrasonic: ACTIVE");
  Serial.println("Body movement: DISABLED");
  Serial.println("=================================");

  normalEyes();
  delay(1000);
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop() {
  float distance = getDistanceCM();

  Serial.print("Distance: ");
  if (distance < 0) {
    Serial.println("No valid reading");
  } else {
    Serial.print(distance, 1);
    Serial.println(" cm");
  }

  // Real ultrasonic detection
  if (distance > 0 && distance <= OBJECT_DISTANCE_CM) {
    objectDetected(distance);
  } else {
    clearPath(distance);
  }

  delay(150);
}

// =====================================================
// REAL HC-SR04 DISTANCE MEASUREMENT
// =====================================================
float getDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long duration = pulseIn(ECHO_PIN, HIGH, 30000UL);

  if (duration == 0) {
    return -1;
  }

  // Sound travels to the object and back, so divide by 2.
  float distance = duration * 0.0343 / 2.0;
  return distance;
}

// =====================================================
// OBJECT DETECTED - REAL SENSOR INPUT
// =====================================================
void objectDetected(float distance) {
  Serial.println("OBJECT DETECTED");
  Serial.print("Real distance: ");
  Serial.print(distance, 1);
  Serial.println(" cm");
  Serial.println("Body action: STOPPED / NOT CONNECTED");
  Serial.println("Head action: CURIOUS");

  curiousEyes();
  smoothPan(CENTER_PAN);
  smoothTilt(CURIOUS_TILT);
  delay(500);

  // Look toward the detected object.
  lookLeft();
  smoothPan(LEFT_PAN);
  delay(500);

  lookRight();
  smoothPan(RIGHT_PAN);
  delay(500);

  normalEyes();
  smoothPan(CENTER_PAN);
  smoothTilt(CENTER_TILT);
}

// =====================================================
// CLEAR PATH
// =====================================================
void clearPath(float distance) {
  normalEyes();
  smoothPan(CENTER_PAN);
  smoothTilt(CENTER_TILT);

  if (distance < 0) {
    Serial.println("Clear/No echo - continuing head idle behaviour");
  } else {
    Serial.print("Clear path: ");
    Serial.print(distance, 1);
    Serial.println(" cm");
  }

  // Since the body cannot move, demonstrate movement intent
  // through the serial monitor and head behaviour only.
  static unsigned long lastIdle = 0;
  if (millis() - lastIdle > 2500) {
    lastIdle = millis();
    idleBehaviour();
  }
}

// =====================================================
// IDLE BEHAVIOUR
// =====================================================
void idleBehaviour() {
  Serial.println("Idle: body movement would be FORWARD");

  blinkEyes();
  delay(300);

  lookLeft();
  smoothPan(LEFT_PAN);
  delay(500);

  normalEyes();
  smoothPan(CENTER_PAN);
  delay(300);

  lookRight();
  smoothPan(RIGHT_PAN);
  delay(500);

  normalEyes();
  smoothPan(CENTER_PAN);
}

// =====================================================
// SMOOTH PAN
// =====================================================
void smoothPan(int target) {
  if (currentPan == target) return;

  if (currentPan < target) {
    for (int pos = currentPan; pos <= target; pos++) {
      panServo.write(pos);
      delay(15);
    }
  } else {
    for (int pos = currentPan; pos >= target; pos--) {
      panServo.write(pos);
      delay(15);
    }
  }
  currentPan = target;
}

// =====================================================
// SMOOTH TILT
// =====================================================
void smoothTilt(int target) {
  if (currentTilt == target) return;

  if (currentTilt < target) {
    for (int pos = currentTilt; pos <= target; pos++) {
      tiltServo.write(pos);
      delay(15);
    }
  } else {
    for (int pos = currentTilt; pos >= target; pos--) {
      tiltServo.write(pos);
      delay(15);
    }
  }
  currentTilt = target;
}

// =====================================================
// EXISTING NYX EYE DESIGNS
// Kept from the existing NYX implementation.
// =====================================================
void drawEye(int x, int y, int w, int h) {
  display.fillRoundRect(x, y, w, h, 12, SSD1306_WHITE);
}

void normalEyes() {
  display.clearDisplay();
  drawEye(15, 12, 38, 40);
  drawEye(75, 12, 38, 40);
  display.display();
}

void blinkEyes() {
  display.clearDisplay();
  display.fillRoundRect(15, 30, 38, 8, 4, SSD1306_WHITE);
  display.fillRoundRect(75, 30, 38, 8, 4, SSD1306_WHITE);
  display.display();
  delay(90);
  normalEyes();
}

void lookLeft() {
  display.clearDisplay();
  drawEye(8, 12, 38, 40);
  drawEye(68, 12, 38, 40);
  display.display();
}

void lookRight() {
  display.clearDisplay();
  drawEye(22, 12, 38, 40);
  drawEye(82, 12, 38, 40);
  display.display();
}

void happyEyes() {
  display.clearDisplay();
  display.drawLine(14, 36, 24, 26, SSD1306_WHITE);
  display.drawLine(24, 26, 45, 36, SSD1306_WHITE);
  display.drawLine(74, 36, 84, 26, SSD1306_WHITE);
  display.drawLine(84, 26, 105, 36, SSD1306_WHITE);
  display.display();
}

void superHappyEyes() {
  display.clearDisplay();
  display.drawLine(12, 37, 23, 24, SSD1306_WHITE);
  display.drawLine(23, 24, 47, 37, SSD1306_WHITE);
  display.drawLine(73, 37, 85, 24, SSD1306_WHITE);
  display.drawLine(85, 24, 109, 37, SSD1306_WHITE);
  display.display();
}

void curiousEyes() {
  display.clearDisplay();
  display.fillRoundRect(10, 8, 43, 46, 13, SSD1306_WHITE);
  display.fillRoundRect(75, 13, 40, 41, 13, SSD1306_WHITE);
  display.display();
}

void worriedEyes() {
  display.clearDisplay();
  display.drawLine(14, 20, 25, 13, SSD1306_WHITE);
  display.drawLine(25, 13, 45, 20, SSD1306_WHITE);
  display.drawLine(74, 20, 85, 13, SSD1306_WHITE);
  display.drawLine(85, 13, 106, 20, SSD1306_WHITE);
  display.display();
}
