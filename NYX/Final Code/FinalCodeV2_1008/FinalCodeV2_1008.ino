#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// =====================================================
// NYX V2
// =====================================================

// Ultrasonic
#define TRIG_PIN 2
#define ECHO_PIN 3

// Edge sensors
#define FRONT_IR 4
#define BACK_IR 5

// Servos
#define PAN_SERVO 9
#define TILT_SERVO 10

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =====================================================
// SERVOS
// =====================================================

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

  // IR
  pinMode(FRONT_IR, INPUT);
  pinMode(BACK_IR, INPUT);

  // Servos
  panServo.attach(PAN_SERVO);
  tiltServo.attach(TILT_SERVO);

  panServo.write(CENTER_PAN);
  tiltServo.write(CENTER_TILT);

  // OLED
  if (!display.begin(
        SSD1306_SWITCHCAPVCC,
        0x3C
      )) {

    Serial.println("OLED ERROR");

    while (1);
  }

  display.clearDisplay();
  display.display();

  randomSeed(analogRead(A0));

  normalEyes();

  Serial.println("NYX V2 READY");
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
  // FRONT EDGE
  // ===================================================

  if (frontIR == 1) {

    worriedEyes();

    smoothPan(CENTER_PAN);
    smoothTilt(FRONT_TILT);

    delay(100);

    return;
  }


  // ===================================================
  // BACK EDGE
  // ===================================================

  if (backIR == 1) {

    worriedEyes();

    smoothPan(CENTER_PAN);
    smoothTilt(BACK_TILT);

    delay(100);

    return;
  }


  // ===================================================
  // PERSON / OBJECT DETECTED
  // ===================================================

  if (distance > 0 && distance <= 40) {

    curiousEyes();

    smoothPan(CENTER_PAN);
    smoothTilt(CURIOUS_TILT);

    delay(60);

    return;
  }


  // ===================================================
  // IDLE
  // ===================================================

  smoothTilt(CENTER_TILT);

  idleAnimation();

  delay(30);
}


// =====================================================
// ULTRASONIC
// =====================================================

float getDistance() {

  long duration;

  digitalWrite(TRIG_PIN, LOW);

  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);

  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(
    ECHO_PIN,
    HIGH,
    30000
  );

  if (duration == 0) {
    return -1;
  }

  return duration * 0.0343 / 2;
}


// =====================================================
// SMOOTH PAN
// =====================================================

void smoothPan(int target) {

  if (currentPan == target) {
    return;
  }

  if (currentPan < target) {

    for (
      int pos = currentPan;
      pos <= target;
      pos++
    ) {

      panServo.write(pos);

      delay(25);
    }

  } else {

    for (
      int pos = currentPan;
      pos >= target;
      pos--
    ) {

      panServo.write(pos);

      delay(25);
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

    for (
      int pos = currentTilt;
      pos <= target;
      pos++
    ) {

      tiltServo.write(pos);

      delay(25);
    }

  } else {

    for (
      int pos = currentTilt;
      pos >= target;
      pos--
    ) {

      tiltServo.write(pos);

      delay(25);
    }
  }

  currentTilt = target;
}


// =====================================================
// BASIC EYE
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
// NORMAL CUTE FACE
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

  delay(90);

  normalEyes();
}


// =====================================================
// LOOK LEFT
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
// LOOK RIGHT
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
// HAPPY FACE
// =====================================================

void happyEyes() {

  display.clearDisplay();

  // Left happy eye

  display.drawLine(
    14,
    36,
    24,
    26,
    SSD1306_WHITE
  );

  display.drawLine(
    24,
    26,
    45,
    36,
    SSD1306_WHITE
  );

  // Right happy eye

  display.drawLine(
    74,
    36,
    84,
    26,
    SSD1306_WHITE
  );

  display.drawLine(
    84,
    26,
    105,
    36,
    SSD1306_WHITE
  );

  display.display();
}


// =====================================================
// EXTRA CUTE HAPPY FACE
// =====================================================

void superHappyEyes() {

  display.clearDisplay();

  // Left eye

  display.drawLine(
    12,
    37,
    23,
    24,
    SSD1306_WHITE
  );

  display.drawLine(
    23,
    24,
    47,
    37,
    SSD1306_WHITE
  );

  // Right eye

  display.drawLine(
    73,
    37,
    85,
    24,
    SSD1306_WHITE
  );

  display.drawLine(
    85,
    24,
    109,
    37,
    SSD1306_WHITE
  );

  display.display();
}


// =====================================================
// CURIOUS FACE
// =====================================================

void curiousEyes() {

  display.clearDisplay();

  // Bigger left eye

  display.fillRoundRect(
    10,
    8,
    43,
    46,
    13,
    SSD1306_WHITE
  );

  // Slightly smaller right eye

  display.fillRoundRect(
    75,
    13,
    40,
    41,
    13,
    SSD1306_WHITE
  );

  display.display();
}


// =====================================================
// WORRIED FACE
// =====================================================

void worriedEyes() {

  display.clearDisplay();

  // Left worried eye

  display.drawLine(
    14,
    20,
    25,
    13,
    SSD1306_WHITE
  );

  display.drawLine(
    25,
    13,
    45,
    20,
    SSD1306_WHITE
  );

  // Right worried eye

  display.drawLine(
    74,
    20,
    85,
    13,
    SSD1306_WHITE
  );

  display.drawLine(
    85,
    13,
    106,
    20,
    SSD1306_WHITE
  );

  display.display();
}


// =====================================================
// CUTE IDLE ANIMATION
// =====================================================

void idleAnimation() {

  unsigned long now = millis();


  // ===================================================
  // RANDOM BLINK
  // ===================================================

  if (now - lastBlink >= nextBlink) {

    blinkEyes();

    lastBlink = millis();

    nextBlink = random(
      2500,
      5000
    );
  }


  // ===================================================
  // WAIT BETWEEN LOOKS
  // ===================================================

  if (now - lastIdleMove < 3000) {
    return;
  }

  lastIdleMove = now;


  // ===================================================
  // LOOK LEFT
  // ===================================================

  if (idleState == 0) {

    lookLeft();

    smoothPan(LEFT_PAN);

    idleState = 1;

    return;
  }


  // ===================================================
  // RETURN CENTER
  // ===================================================

  if (idleState == 1) {

    normalEyes();

    smoothPan(CENTER_PAN);

    idleState = 2;

    return;
  }


  // ===================================================
  // LOOK RIGHT
  // ===================================================

  if (idleState == 2) {

    lookRight();

    smoothPan(RIGHT_PAN);

    idleState = 3;

    return;
  }


  // ===================================================
  // RETURN CENTER
  // ===================================================

  if (idleState == 3) {

    normalEyes();

    smoothPan(CENTER_PAN);

    idleState = 4;

    return;
  }


  // ===================================================
  // LITTLE HAPPY MOMENT
  // ===================================================

  if (idleState == 4) {

    superHappyEyes();

    delay(700);

    normalEyes();

    idleState = 0;

    return;
  }
}