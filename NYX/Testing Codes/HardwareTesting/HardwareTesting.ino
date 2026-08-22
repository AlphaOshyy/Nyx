#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// =====================================================
// NYX TEST V4
// Working OLED base
// 3 Touch Sensors
// Pan Servo Only
// Ultrasonic
// 2 Buzzers
// =====================================================

// ================= OLED =================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// ================= PAN SERVO =================

Servo panServo;

#define PAN_PIN 9

// D10 NOT USED
// Tilt mechanism is broken

// ================= TOUCH =================

#define TOUCH_HEAD 4
#define TOUCH_LEFT 5
#define TOUCH_RIGHT 11

// ================= ULTRASONIC =================

#define TRIG_PIN 2
#define ECHO_PIN 3

// ================= BUZZERS =================

#define BUZZER_LEFT A0
#define BUZZER_RIGHT A1

// ================= TOUCH STATES =================

int lastHead = LOW;
int lastLeft = LOW;
int lastRight = LOW;

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  Serial.println();
  Serial.println("======================");
  Serial.println("NYX TEST V4");
  Serial.println("======================");

  // ===================================================
  // OLED
  // EXACT SAME INITIALIZATION AS WORKING TEST
  // ===================================================

  display.begin(
    SSD1306_SWITCHCAPVCC,
    0x3C
  );

  delay(100);

  normalEyes();

  Serial.println(
    "OLED STARTED"
  );

  // ===================================================
  // PAN SERVO
  // ===================================================

  panServo.attach(
    PAN_PIN
  );

  panServo.write(
    80
  );

  Serial.println(
    "PAN SERVO READY"
  );

  // ===================================================
  // TOUCH
  // ===================================================

  pinMode(
    TOUCH_HEAD,
    INPUT
  );

  pinMode(
    TOUCH_LEFT,
    INPUT
  );

  pinMode(
    TOUCH_RIGHT,
    INPUT
  );

  Serial.println(
    "TOUCH D4 READY"
  );

  Serial.println(
    "TOUCH D5 READY"
  );

  Serial.println(
    "TOUCH D11 READY"
  );

  // ===================================================
  // ULTRASONIC
  // ===================================================

  pinMode(
    TRIG_PIN,
    OUTPUT
  );

  pinMode(
    ECHO_PIN,
    INPUT
  );

  digitalWrite(
    TRIG_PIN,
    LOW
  );

  Serial.println(
    "ULTRASONIC READY"
  );

  // ===================================================
  // BUZZERS
  // ===================================================

  pinMode(
    BUZZER_LEFT,
    OUTPUT
  );

  pinMode(
    BUZZER_RIGHT,
    OUTPUT
  );

  noTone(
    BUZZER_LEFT
  );

  noTone(
    BUZZER_RIGHT
  );

  Serial.println(
    "BUZZER A0 READY"
  );

  Serial.println(
    "BUZZER A1 READY"
  );

  // ===================================================
  // BUZZER TEST
  // ===================================================

  delay(500);

  Serial.println(
    "A0 TEST"
  );

  tone(
    BUZZER_LEFT,
    800,
    250
  );

  delay(350);

  Serial.println(
    "A1 TEST"
  );

  tone(
    BUZZER_RIGHT,
    1000,
    250
  );

  delay(350);

  Serial.println(
    "BOTH TEST"
  );

  tone(
    BUZZER_LEFT,
    700
  );

  tone(
    BUZZER_RIGHT,
    1000
  );

  delay(300);

  noTone(
    BUZZER_LEFT
  );

  noTone(
    BUZZER_RIGHT
  );

  Serial.println();
  Serial.println(
    "NYX READY"
  );
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  // ===================================================
  // TOUCH
  // ===================================================

  int head =
    digitalRead(
      TOUCH_HEAD
    );

  int left =
    digitalRead(
      TOUCH_LEFT
    );

  int right =
    digitalRead(
      TOUCH_RIGHT
    );

  // ===================================================
  // HEAD
  // ===================================================

  if (
    head == HIGH &&
    lastHead == LOW
  ) {

    Serial.println(
      "HEAD TOUCH"
    );

    happyEyes();

    panServo.write(
      80
    );

    headSound();
  }

  // ===================================================
  // LEFT
  // ===================================================

  if (
    left == HIGH &&
    lastLeft == LOW
  ) {

    Serial.println(
      "LEFT TOUCH"
    );

    lookLeft();

    panServo.write(
      30
    );

    leftSound();

    delay(300);

    panServo.write(
      80
    );
  }

  // ===================================================
  // RIGHT
  // ===================================================

  if (
    right == HIGH &&
    lastRight == LOW
  ) {

    Serial.println(
      "RIGHT TOUCH"
    );

    lookRight();

    panServo.write(
      130
    );

    rightSound();

    delay(300);

    panServo.write(
      80
    );
  }

  // ===================================================
  // ALL THREE
  // ===================================================

  if (
    head == HIGH &&
    left == HIGH &&
    right == HIGH
  ) {

    if (
      lastHead == LOW ||
      lastLeft == LOW ||
      lastRight == LOW
    ) {

      Serial.println(
        "ALL THREE TOUCH"
      );

      excitedEyes();

      panServo.write(
        30
      );

      delay(200);

      panServo.write(
        130
      );

      delay(200);

      panServo.write(
        80
      );

      specialSound();

      delay(300);

      normalEyes();
    }
  }

  // Save states

  lastHead = head;
  lastLeft = left;
  lastRight = right;

  // ===================================================
  // ULTRASONIC
  // ===================================================

  checkDistance();

  delay(80);
}

// =====================================================
// ULTRASONIC
// =====================================================

void checkDistance() {

  digitalWrite(
    TRIG_PIN,
    LOW
  );

  delayMicroseconds(2);

  digitalWrite(
    TRIG_PIN,
    HIGH
  );

  delayMicroseconds(10);

  digitalWrite(
    TRIG_PIN,
    LOW
  );

  long duration =
    pulseIn(
      ECHO_PIN,
      HIGH,
      30000
    );

  if (
    duration > 0
  ) {

    float distance =
      duration * 0.0343 / 2.0;

    Serial.print(
      "Distance: "
    );

    Serial.print(
      distance
    );

    Serial.println(
      " cm"
    );

    if (
      distance < 30
    ) {

      curiousEyes();
    }
  }
}

// =====================================================
// BUZZER SOUNDS
// =====================================================

void headSound() {

  tone(
    BUZZER_LEFT,
    700,
    80
  );

  delay(100);

  tone(
    BUZZER_RIGHT,
    900,
    100
  );

  delay(120);

  noTone(
    BUZZER_LEFT
  );

  noTone(
    BUZZER_RIGHT
  );
}

void leftSound() {

  tone(
    BUZZER_LEFT,
    500,
    100
  );

  delay(130);

  tone(
    BUZZER_LEFT,
    700,
    120
  );

  delay(150);

  noTone(
    BUZZER_LEFT
  );
}

void rightSound() {

  tone(
    BUZZER_RIGHT,
    700,
    100
  );

  delay(130);

  tone(
    BUZZER_RIGHT,
    950,
    120
  );

  delay(150);

  noTone(
    BUZZER_RIGHT
  );
}

void specialSound() {

  tone(
    BUZZER_LEFT,
    600
  );

  tone(
    BUZZER_RIGHT,
    800
  );

  delay(120);

  noTone(
    BUZZER_LEFT
  );

  noTone(
    BUZZER_RIGHT
  );

  delay(100);

  tone(
    BUZZER_LEFT,
    900
  );

  tone(
    BUZZER_RIGHT,
    1200
  );

  delay(150);

  noTone(
    BUZZER_LEFT
  );

  noTone(
    BUZZER_RIGHT
  );
}

// =====================================================
// OLED
// EXACT WORKING NYX CODE
// =====================================================

void drawEye(
  int x,
  int y,
  int w,
  int h
) {

  display.fillRoundRect(
    x,
    y,
    w,
    h,
    12,
    SSD1306_WHITE
  );
}

// =====================================================
// NORMAL
// =====================================================

void normalEyes() {

  display.clearDisplay();

  drawEye(
    15,
    12,
    38,
    40
  );

  drawEye(
    75,
    12,
    38,
    40
  );

  display.display();
}

// =====================================================
// BLINK
// =====================================================

void blinkEyes() {

  display.clearDisplay();

  display.fillRoundRect(
    15,
    30,
    38,
    8,
    4,
    SSD1306_WHITE
  );

  display.fillRoundRect(
    75,
    30,
    38,
    8,
    4,
    SSD1306_WHITE
  );

  display.display();
}

// =====================================================
// LEFT
// =====================================================

void lookLeft() {

  display.clearDisplay();

  drawEye(
    8,
    12,
    38,
    40
  );

  drawEye(
    68,
    12,
    38,
    40
  );

  display.display();
}

// =====================================================
// RIGHT
// =====================================================

void lookRight() {

  display.clearDisplay();

  drawEye(
    22,
    12,
    38,
    40
  );

  drawEye(
    82,
    12,
    38,
    40
  );

  display.display();
}

// =====================================================
// HAPPY
// =====================================================

void happyEyes() {

  display.clearDisplay();

  display.drawLine(
    15,
    35,
    25,
    25,
    SSD1306_WHITE
  );

  display.drawLine(
    25,
    25,
    45,
    35,
    SSD1306_WHITE
  );

  display.drawLine(
    75,
    35,
    85,
    25,
    SSD1306_WHITE
  );

  display.drawLine(
    85,
    25,
    105,
    35,
    SSD1306_WHITE
  );

  display.display();
}

// =====================================================
// EXCITED
// =====================================================

void excitedEyes() {

  display.clearDisplay();

  drawEye(
    7,
    7,
    45,
    48
  );

  drawEye(
    76,
    7,
    45,
    48
  );

  display.display();
}

// =====================================================
// CURIOUS
// =====================================================

void curiousEyes() {

  display.clearDisplay();

  drawEye(
    8,
    8,
    44,
    46
  );

  drawEye(
    76,
    15,
    40,
    40
  );

  display.display();
}