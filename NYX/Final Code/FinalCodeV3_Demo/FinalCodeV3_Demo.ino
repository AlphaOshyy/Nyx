#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// =====================================================
// NYX V3 - PRESENTATION DEMO MODE
// Uses existing NYX V2 eye designs only.
// No ultrasonic sensor or wheel movement required.
// Sensor/movement events are simulated for demonstration.
// =====================================================

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
const int FRONT_TILT = 84;
const int BACK_TILT = 96;

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(9600);

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

  Serial.println("NYX V3 DEMO MODE READY");
  Serial.println("Movement and ultrasonic sensing are simulated.");

  normalEyes();
  delay(1000);
}

// =====================================================
// MAIN DEMO SEQUENCE
// =====================================================
void loop() {
  demoClearPath();
  demoObjectDetected();
  demoClearPath();
  demoFrontEdge();
  demoClearPath();
  demoBackEdge();
  demoIdle();
}

// =====================================================
// DEMO: CLEAR PATH
// =====================================================
void demoClearPath() {
  Serial.println("\n[DEMO] CLEAR PATH");
  Serial.println("Simulated distance: 80 cm");
  Serial.println("Simulated action: MOVING FORWARD");

  normalEyes();
  smoothTilt(CENTER_TILT);
  smoothPan(CENTER_PAN);
  delay(1800);
}

// =====================================================
// DEMO: OBJECT DETECTED
// =====================================================
void demoObjectDetected() {
  Serial.println("\n[DEMO] OBJECT DETECTED");
  Serial.println("Simulated distance: 25 cm");
  Serial.println("Action: STOP BODY (SIMULATED)");
  Serial.println("Expression: CURIOUS");

  curiousEyes();
  smoothPan(CENTER_PAN);
  smoothTilt(CURIOUS_TILT);
  delay(1200);

  Serial.println("Action: LOOK LEFT");
  lookLeft();
  smoothPan(LEFT_PAN);
  delay(900);

  Serial.println("Action: LOOK RIGHT");
  lookRight();
  smoothPan(RIGHT_PAN);
  delay(900);

  normalEyes();
  smoothPan(CENTER_PAN);
  smoothTilt(CENTER_TILT);
}

// =====================================================
// DEMO: FRONT EDGE
// =====================================================
void demoFrontEdge() {
  Serial.println("\n[DEMO] FRONT EDGE DETECTED");
  Serial.println("Simulated Front IR: DETECTED");
  Serial.println("Action: STOP BODY (SIMULATED)");
  Serial.println("Action: REVERSE + TURN (SIMULATED)");
  Serial.println("Expression: WORRIED");

  worriedEyes();
  smoothPan(CENTER_PAN);
  smoothTilt(FRONT_TILT);
  delay(1200);

  Serial.println("Action: RETURN TO SAFE POSITION");
  normalEyes();
  smoothTilt(CENTER_TILT);
  smoothPan(RIGHT_PAN);
  delay(700);
  smoothPan(CENTER_PAN);
  delay(700);
}

// =====================================================
// DEMO: BACK EDGE
// =====================================================
void demoBackEdge() {
  Serial.println("\n[DEMO] BACK EDGE DETECTED");
  Serial.println("Simulated Back IR: DETECTED");
  Serial.println("Action: STOP BODY (SIMULATED)");
  Serial.println("Action: MOVE FORWARD + TURN (SIMULATED)");
  Serial.println("Expression: WORRIED");

  worriedEyes();
  smoothPan(CENTER_PAN);
  smoothTilt(BACK_TILT);
  delay(1200);

  Serial.println("Action: RETURN TO SAFE POSITION");
  normalEyes();
  smoothTilt(CENTER_TILT);
  smoothPan(LEFT_PAN);
  delay(700);
  smoothPan(CENTER_PAN);
  delay(700);
}

// =====================================================
// DEMO: IDLE BEHAVIOUR
// =====================================================
void demoIdle() {
  Serial.println("\n[DEMO] AUTONOMOUS IDLE BEHAVIOUR");

  normalEyes();
  delay(700);

  blinkEyes();
  delay(500);

  lookLeft();
  smoothPan(LEFT_PAN);
  delay(900);

  normalEyes();
  smoothPan(CENTER_PAN);
  delay(700);

  lookRight();
  smoothPan(RIGHT_PAN);
  delay(900);

  normalEyes();
  smoothPan(CENTER_PAN);
  delay(700);

  superHappyEyes();
  delay(900);
  normalEyes();

  Serial.println("Demo cycle complete. Restarting...");
  delay(1200);
}

// =====================================================
// SMOOTH PAN
// =====================================================
void smoothPan(int target) {
  if (currentPan == target) return;

  if (currentPan < target) {
    for (int pos = currentPan; pos <= target; pos++) {
      panServo.write(pos);
      delay(20);
    }
  } else {
    for (int pos = currentPan; pos >= target; pos--) {
      panServo.write(pos);
      delay(20);
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
      delay(20);
    }
  } else {
    for (int pos = currentTilt; pos >= target; pos--) {
      tiltServo.write(pos);
      delay(20);
    }
  }
  currentTilt = target;
}

// =====================================================
// EXISTING NYX EYE DESIGNS FROM V2
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
