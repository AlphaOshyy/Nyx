#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>



// ---------------- ULTRASONIC ----------------
#define TRIG_PIN 2
#define ECHO_PIN 3

// ---------------- EDGE SENSORS ----------------
#define FRONT_IR 4
#define BACK_IR 5

// ---------------- MOTOR DRIVER ----------------
// L298N / similar dual H-bridge
#define ENA 6
#define IN1 7
#define IN2 8
#define ENB 13
#define IN3 11
#define IN4 12

// ---------------- HEAD SERVOS ----------------
#define PAN_SERVO 9
#define TILT_SERVO 10

// ---------------- OLED ----------------
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Servo panServo;
Servo tiltServo;

// =====================================================
// HEAD POSITIONS
// =====================================================

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
// MOVEMENT SETTINGS
// =====================================================

const int DRIVE_SPEED = 180;
const int TURN_SPEED = 170;

const int OBJECT_DISTANCE = 40;

// =====================================================
// TIMING
// =====================================================

unsigned long lastBlink = 0;
unsigned long nextBlink = 3500;
unsigned long lastIdleMove = 0;

int idleState = 0;

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // IR edge sensors
  pinMode(FRONT_IR, INPUT);
  pinMode(BACK_IR, INPUT);

  // Motor driver
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  // Head servos
  panServo.attach(PAN_SERVO);
  tiltServo.attach(TILT_SERVO);

  panServo.write(CENTER_PAN);
  tiltServo.write(CENTER_TILT);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED ERROR");
    while (1);
  }

  display.clearDisplay();
  display.display();

  randomSeed(analogRead(A0));

  normalEyes();

  Serial.println("NYX V3 READY");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  int frontIR = digitalRead(FRONT_IR);
  int backIR = digitalRead(BACK_IR);
  float distance = getDistance();

  Serial.print("Front IR: ");
  Serial.print(frontIR);
  Serial.print(" | Back IR: ");
  Serial.print(backIR);
  Serial.print(" | Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // ===================================================
  // FRONT EDGE - SAFETY HAS HIGHEST PRIORITY
  // ===================================================

  if (frontIR == 1) {

    stopMotors();
    worriedEyes();

    smoothPan(CENTER_PAN);
    smoothTilt(FRONT_TILT);

    // Move away from the detected front edge
    backward(DRIVE_SPEED);
    delay(450);
    stopMotors();

    // Turn away from the edge
    turnRight(TURN_SPEED);
    delay(450);
    stopMotors();

    smoothTilt(CENTER_TILT);
    normalEyes();

    return;
  }

  // ===================================================
  // BACK EDGE - SAFETY HAS HIGHEST PRIORITY
  // ===================================================

  if (backIR == 1) {

    stopMotors();
    worriedEyes();

    smoothPan(CENTER_PAN);
    smoothTilt(BACK_TILT);

    // Move away from the detected rear edge
    forward(DRIVE_SPEED);
    delay(450);
    stopMotors();

    // Turn away from the edge
    turnLeft(TURN_SPEED);
    delay(450);
    stopMotors();

    smoothTilt(CENTER_TILT);
    normalEyes();

    return;
  }

  // ===================================================
  // OBJECT / PERSON DETECTED
  // ===================================================

  if (distance > 0 && distance <= OBJECT_DISTANCE) {

    stopMotors();
    curiousEyes();

    smoothPan(CENTER_PAN);
    smoothTilt(CURIOUS_TILT);

    // Small reaction pause
    delay(250);

    // Look around while stopped
    smoothPan(LEFT_PAN);
    delay(300);
    smoothPan(RIGHT_PAN);
    delay(500);
    smoothPan(CENTER_PAN);

    smoothTilt(CENTER_TILT);
    normalEyes();

    // Turn to search for a clear path
    turnRight(TURN_SPEED);
    delay(500);
    stopMotors();

    return;
  }

  // ===================================================
  // CLEAR PATH - MOVE FORWARD
  // ===================================================

  forward(DRIVE_SPEED);

  // Head/face idle behaviour while moving
  idleAnimation();

  delay(30);
}

// =====================================================
// ULTRASONIC DISTANCE
// =====================================================

float getDistance() {

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2;
}

// =====================================================
// BODY MOVEMENT
// =====================================================

void forward(int speedValue) {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
}

void backward(int speedValue) {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
}

void turnLeft(int speedValue) {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
}

void turnRight(int speedValue) {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, speedValue);
  analogWrite(ENB, speedValue);
}

void stopMotors() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

// =====================================================
// SMOOTH PAN
// =====================================================

void smoothPan(int target) {

  if (currentPan == target) {
    return;
  }

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

  if (currentTilt == target) {
    return;
  }

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
// BASIC EYE
// =====================================================

void drawEye(int x, int y, int w, int h) {

  display.fillRoundRect(
    x, y, w, h, 12, SSD1306_WHITE
  );
}

// =====================================================
// NORMAL FACE
// =====================================================

void normalEyes() {

  display.clearDisplay();

  drawEye(15, 12, 38, 40);
  drawEye(75, 12, 38, 40);

  display.display();
}

// =====================================================
// BLINK
// =====================================================

void blinkEyes() {

  display.clearDisplay();

  display.fillRoundRect(15, 30, 38, 8, 4, SSD1306_WHITE);
  display.fillRoundRect(75, 30, 38, 8, 4, SSD1306_WHITE);

  display.display();
  delay(90);

  normalEyes();
}

// =====================================================
// LOOK LEFT
// =====================================================

void lookLeft() {

  display.clearDisplay();

  drawEye(8, 12, 38, 40);
  drawEye(68, 12, 38, 40);

  display.display();
}

// =====================================================
// LOOK RIGHT
// =====================================================

void lookRight() {

  display.clearDisplay();

  drawEye(22, 12, 38, 40);
  drawEye(82, 12, 38, 40);

  display.display();
}

// =====================================================
// HAPPY FACE
// =====================================================

void happyEyes() {

  display.clearDisplay();

  display.drawLine(14, 36, 24, 26, SSD1306_WHITE);
  display.drawLine(24, 26, 45, 36, SSD1306_WHITE);

  display.drawLine(74, 36, 84, 26, SSD1306_WHITE);
  display.drawLine(84, 26, 105, 36, SSD1306_WHITE);

  display.display();
}

// =====================================================
// SUPER HAPPY FACE
// =====================================================

void superHappyEyes() {

  display.clearDisplay();

  display.drawLine(12, 37, 23, 24, SSD1306_WHITE);
  display.drawLine(23, 24, 47, 37, SSD1306_WHITE);

  display.drawLine(73, 37, 85, 24, SSD1306_WHITE);
  display.drawLine(85, 24, 109, 37, SSD1306_WHITE);

  display.display();
}

// =====================================================
// CURIOUS FACE
// =====================================================

void curiousEyes() {

  display.clearDisplay();

  display.fillRoundRect(10, 8, 43, 46, 13, SSD1306_WHITE);
  display.fillRoundRect(75, 13, 40, 41, 13, SSD1306_WHITE);

  display.display();
}

// =====================================================
// WORRIED FACE
// =====================================================

void worriedEyes() {

  display.clearDisplay();

  display.drawLine(14, 20, 25, 13, SSD1306_WHITE);
  display.drawLine(25, 13, 45, 20, SSD1306_WHITE);

  display.drawLine(74, 20, 85, 13, SSD1306_WHITE);
  display.drawLine(85, 13, 106, 20, SSD1306_WHITE);

  display.display();
}

// =====================================================
// IDLE HEAD / FACE ANIMATION
// =====================================================

void idleAnimation() {

  unsigned long now = millis();

  // Random blink
  if (now - lastBlink >= nextBlink) {

    blinkEyes();

    lastBlink = millis();
    nextBlink = random(2500, 5000);
  }

  // Wait between head movements
  if (now - lastIdleMove < 3000) {
    return;
  }

  lastIdleMove = now;

  // Look left
  if (idleState == 0) {

    lookLeft();
    smoothPan(LEFT_PAN);
    idleState = 1;
    return;
  }

  // Centre
  if (idleState == 1) {

    normalEyes();
    smoothPan(CENTER_PAN);
    idleState = 2;
    return;
  }

  // Look right
  if (idleState == 2) {

    lookRight();
    smoothPan(RIGHT_PAN);
    idleState = 3;
    return;
  }

  // Centre
  if (idleState == 3) {

    normalEyes();
    smoothPan(CENTER_PAN);
    idleState = 4;
    return;
  }

  // Happy moment
  if (idleState == 4) {

    superHappyEyes();
    delay(500);
    normalEyes();
    idleState = 0;
    return;
  }
}
