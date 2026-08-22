#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// =====================================================
// NYX V4
// Arduino Nano
// =====================================================

// ---------------- PIN CONFIG ----------------

#define TRIG_PIN 2
#define ECHO_PIN 3

#define ESP_RX 6
#define ESP_TX 7

#define PAN_PIN 9

#define BUZZER_1 A0
#define BUZZER_2 A1

// ---------------- OLED ----------------

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// ---------------- SERIAL ----------------

SoftwareSerial espSerial(
  ESP_RX,
  ESP_TX
);

// ---------------- SERVO ----------------

Servo panServo;

int currentPan = 90;
int targetPan = 90;

const int PAN_LEFT = 58;
const int PAN_CENTER = 90;
const int PAN_RIGHT = 122;

// ---------------- DISTANCE ----------------

bool nearby = false;

float currentDistance = -1;

unsigned long lastDistanceCheck = 0;

const unsigned long DISTANCE_INTERVAL = 100;

// ---------------- VISION ----------------

String lastVision = "CENTER";

unsigned long lastVisionTime = 0;

int leftHits = 0;
int rightHits = 0;

const unsigned long LOST_TARGET_TIME = 2800;

// ---------------- ACTIVITY ----------------

unsigned long lastActivity = 0;

const unsigned long SLEEP_TIME = 45000;

// ---------------- BLINK ----------------

unsigned long nextBlink = 3500;

// ---------------- IDLE MOVEMENT ----------------

unsigned long lastIdleMove = 0;

unsigned long nextIdleMove = 6000;

// ---------------- SOUND ----------------

unsigned long lastSound = 0;

// =====================================================
// EMOTIONS
// =====================================================

enum Emotion {

  IDLE,

  HELLO,

  CURIOUS,

  HAPPY,

  EXCITED,

  SHY,

  TRACKING,

  CONFUSED,

  SEARCHING,

  SLEEPY,

  ALERT,

  CALM
};

Emotion emotion = IDLE;

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  espSerial.begin(9600);

  pinMode(
    TRIG_PIN,
    OUTPUT
  );

  pinMode(
    ECHO_PIN,
    INPUT
  );

  pinMode(
    BUZZER_1,
    OUTPUT
  );

  pinMode(
    BUZZER_2,
    OUTPUT
  );

  panServo.attach(
    PAN_PIN
  );

  panServo.write(
    PAN_CENTER
  );

  Wire.begin();

  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      0x3C
    )
  ) {

    while (true) {
    }
  }

  randomSeed(
    analogRead(A2)
  );

  normalEyes();

  delay(300);

  helloSound();

  lastActivity =
    millis();

  lastVisionTime =
    millis();
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop() {

  updateServo();

  checkDistance();

  readESP32();

  updateEmotion();

  updateBlink();

  updateIdleMovement();

  sendRobotState();
}

// =====================================================
// SMOOTH SERVO
// =====================================================

void updateServo() {

  static unsigned long lastServo =
    0;

  if (
    millis() - lastServo <
    10
  ) {
    return;
  }

  lastServo =
    millis();

  float difference =
    targetPan - currentPan;

  if (
    abs(difference) < 0.4
  ) {

    currentPan =
      targetPan;

  } else {

    currentPan +=
      difference * 0.15;
  }

  panServo.write(
    (int)currentPan
  );
}

// =====================================================
// ULTRASONIC
// =====================================================

void checkDistance() {

  if (
    millis() - lastDistanceCheck <
    DISTANCE_INTERVAL
  ) {
    return;
  }

  lastDistanceCheck =
    millis();

  currentDistance =
    getDistance();

  bool detected =
    currentDistance > 0 &&
    currentDistance < 45;

  // ---------------- NEW OBJECT ----------------

  if (
    detected &&
    !nearby
  ) {

    nearby = true;

    lastActivity =
      millis();

    lastVisionTime =
      millis();

    setEmotion(
      HELLO
    );
  }

  // ---------------- VERY CLOSE ----------------

  if (
    detected &&
    currentDistance < 8
  ) {

    if (
      emotion != ALERT
    ) {

      setEmotion(
        ALERT
      );
    }
  }

  else if (
    detected &&
    currentDistance < 15
  ) {

    if (
      emotion != SHY &&
      emotion != ALERT
    ) {

      setEmotion(
        SHY
      );
    }
  }

  // ---------------- OBJECT LEFT ----------------

  if (
    !detected &&
    nearby
  ) {

    nearby = false;

    targetPan =
      PAN_CENTER;

    setEmotion(
      CALM
    );

    lastActivity =
      millis();
  }
}

// =====================================================
// GET DISTANCE
// =====================================================

float getDistance() {

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

  unsigned long duration =
    pulseIn(
      ECHO_PIN,
      HIGH,
      18000
    );

  if (
    duration == 0
  ) {

    return -1;
  }

  return
    duration *
    0.0343 /
    2.0;
}

// =====================================================
// ESP32 CAMERA INPUT
// =====================================================

void readESP32() {

  while (
    espSerial.available()
  ) {

    String command =
      espSerial.readStringUntil(
        '\n'
      );

    command.trim();

    // ---------------- LEFT ----------------

    if (
      command == "LEFT"
    ) {

      if (!nearby) {
        continue;
      }

      lastVision =
        "LEFT";

      lastVisionTime =
        millis();

      lastActivity =
        millis();

      targetPan =
        PAN_LEFT;

      leftHits++;

      rightHits = 0;

      if (
        leftHits >= 3
      ) {

        setEmotion(
          TRACKING
        );
      }
    }

    // ---------------- RIGHT ----------------

    else if (
      command == "RIGHT"
    ) {

      if (!nearby) {
        continue;
      }

      lastVision =
        "RIGHT";

      lastVisionTime =
        millis();

      lastActivity =
        millis();

      targetPan =
        PAN_RIGHT;

      rightHits++;

      leftHits = 0;

      if (
        rightHits >= 3
      ) {

        setEmotion(
          TRACKING
        );
      }
    }

    // ---------------- CENTER ----------------

    else if (
      command == "CENTER"
    ) {

      if (!nearby) {
        continue;
      }

      lastVision =
        "CENTER";

      lastVisionTime =
        millis();

      lastActivity =
        millis();

      targetPan =
        PAN_CENTER;

      leftHits = 0;
      rightHits = 0;

      setEmotion(
        HAPPY
      );
    }

    // ---------------- FAST MOVEMENT ----------------

    else if (
      command == "FAST"
    ) {

      if (!nearby) {
        continue;
      }

      lastActivity =
        millis();

      setEmotion(
        EXCITED
      );
    }
  }
}

// =====================================================
// EMOTION ENGINE
// =====================================================

void updateEmotion() {

  // Object exists

  if (nearby) {

    // Lost visual target

    if (
      millis() - lastVisionTime >
      LOST_TARGET_TIME
    ) {

      if (
        emotion != SHY &&
        emotion != ALERT &&
        emotion != SEARCHING
      ) {

        setEmotion(
          SEARCHING
        );
      }
    }

    // Side switching

    if (
      leftHits >= 3 &&
      rightHits >= 3
    ) {

      setEmotion(
        CONFUSED
      );

      leftHits = 0;
      rightHits = 0;
    }

    return;
  }

  // No object

  if (
    millis() - lastActivity >
    SLEEP_TIME
  ) {

    setEmotion(
      SLEEPY
    );
  }
}

// =====================================================
// EMOTION CHANGE
// =====================================================

void setEmotion(
  Emotion newEmotion
) {

  if (
    emotion == newEmotion
  ) {

    return;
  }

  emotion =
    newEmotion;

  switch (
    emotion
  ) {

    case IDLE:

      normalEyes();

      break;

    case HELLO:

      targetPan =
        PAN_CENTER;

      helloEyes();

      helloSound();

      break;

    case CURIOUS:

      curiousEyes();

      curiousSound();

      break;

    case HAPPY:

      happyEyes();

      happySound();

      break;

    case EXCITED:

      excitedEyes();

      excitedSound();

      break;

    case SHY:

      targetPan =
        PAN_CENTER;

      shyEyes();

      shySound();

      break;

    case TRACKING:

      trackingEyes();

      trackingSound();

      break;

    case CONFUSED:

      confusedEyes();

      confusedSound();

      break;

    case SEARCHING:

      searchingEyes();

      searchingSound();

      break;

    case SLEEPY:

      targetPan =
        PAN_CENTER;

      sleepyEyes();

      sleepySound();

      break;

    case ALERT:

      alertEyes();

      alertSound();

      break;

    case CALM:

      targetPan =
        PAN_CENTER;

      calmEyes();

      calmSound();

      break;
  }
}

// =====================================================
// BLINK
// =====================================================

void updateBlink() {

  if (
    millis() < nextBlink
  ) {

    return;
  }

  if (
    emotion == ALERT
  ) {

    nextBlink =
      millis() + 2000;

    return;
  }

  blinkEyes();

  nextBlink =
    millis() +
    random(
      2500,
      6500
    );
}

// =====================================================
// IDLE MOVEMENT
// =====================================================

void updateIdleMovement() {

  if (
    nearby
  ) {

    return;
  }

  if (
    millis() - lastIdleMove <
    nextIdleMove
  ) {

    return;
  }

  lastIdleMove =
    millis();

  int action =
    random(
      0,
      4
    );

  if (
    action == 0
  ) {

    targetPan =
      80;

    lookLeft();
  }

  else if (
    action == 1
  ) {

    targetPan =
      100;

    lookRight();
  }

  else {

    targetPan =
      PAN_CENTER;

    normalEyes();
  }

  nextIdleMove =
    random(
      4000,
      8000
    );
}

// =====================================================
// SEND STATUS TO ESP32
// =====================================================

void sendRobotState() {

  static unsigned long lastSend =
    0;

  if (
    millis() - lastSend <
    500
  ) {

    return;
  }

  lastSend =
    millis();

  if (
    nearby
  ) {

    espSerial.println(
      "NEAR"
    );

  } else {

    espSerial.println(
      "IDLE"
    );
  }
}

// =====================================================
// OLED EYES
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

void helloEyes() {

  display.clearDisplay();

  drawEye(
    13,
    14,
    40,
    38
  );

  drawEye(
    75,
    14,
    40,
    38
  );

  display.display();
}

void happyEyes() {

  display.clearDisplay();

  display.drawLine(
    14,
    38,
    25,
    25,
    SSD1306_WHITE
  );

  display.drawLine(
    25,
    25,
    45,
    38,
    SSD1306_WHITE
  );

  display.drawLine(
    76,
    38,
    87,
    25,
    SSD1306_WHITE
  );

  display.drawLine(
    87,
    25,
    107,
    38,
    SSD1306_WHITE
  );

  display.display();
}

void curiousEyes() {

  display.clearDisplay();

  drawEye(
    9,
    8,
    44,
    47
  );

  drawEye(
    75,
    13,
    42,
    42
  );

  display.display();
}

void excitedEyes() {

  display.clearDisplay();

  drawEye(
    7,
    7,
    47,
    48
  );

  drawEye(
    74,
    7,
    47,
    48
  );

  display.display();
}

void shyEyes() {

  display.clearDisplay();

  display.fillRoundRect(
    18,
    25,
    30,
    20,
    8,
    SSD1306_WHITE
  );

  display.fillRoundRect(
    80,
    25,
    30,
    20,
    8,
    SSD1306_WHITE
  );

  display.display();
}

void trackingEyes() {

  if (
    lastVision ==
    "LEFT"
  ) {

    lookLeft();

  }

  else if (
    lastVision ==
    "RIGHT"
  ) {

    lookRight();

  }

  else {

    normalEyes();
  }
}

void confusedEyes() {

  display.clearDisplay();

  display.drawCircle(
    32,
    32,
    18,
    SSD1306_WHITE
  );

  display.drawCircle(
    96,
    32,
    18,
    SSD1306_WHITE
  );

  display.display();
}

void searchingEyes() {

  display.clearDisplay();

  display.drawCircle(
    32,
    32,
    17,
    SSD1306_WHITE
  );

  display.drawCircle(
    96,
    32,
    17,
    SSD1306_WHITE
  );

  display.display();
}

void sleepyEyes() {

  display.clearDisplay();

  display.fillRoundRect(
    15,
    32,
    38,
    6,
    3,
    SSD1306_WHITE
  );

  display.fillRoundRect(
    75,
    32,
    38,
    6,
    3,
    SSD1306_WHITE
  );

  display.display();
}

void alertEyes() {

  display.clearDisplay();

  drawEye(
    8,
    7,
    45,
    48
  );

  drawEye(
    75,
    7,
    45,
    48
  );

  display.display();
}

void calmEyes() {

  normalEyes();
}

void lookLeft() {

  display.clearDisplay();

  drawEye(
    7,
    12,
    38,
    40
  );

  drawEye(
    67,
    12,
    38,
    40
  );

  display.display();
}

void lookRight() {

  display.clearDisplay();

  drawEye(
    23,
    12,
    38,
    40
  );

  drawEye(
    83,
    12,
    38,
    40
  );

  display.display();
}

void blinkEyes() {

  display.clearDisplay();

  display.fillRoundRect(
    15,
    31,
    38,
    6,
    3,
    SSD1306_WHITE
  );

  display.fillRoundRect(
    75,
    31,
    38,
    6,
    3,
    SSD1306_WHITE
  );

  display.display();

  delay(65);

  trackingEyes();
}

// =====================================================
// BUZZER VOICE
// =====================================================

void note(
  int buzzer,
  int frequency,
  int duration
) {

  tone(
    buzzer,
    frequency,
    duration
  );

  delay(
    duration
  );

  noTone(
    buzzer
  );

  delay(10);
}

// =====================================================
// HI
// =====================================================

void helloSound() {

  note(
    BUZZER_1,
    659,
    70
  );

  note(
    BUZZER_2,
    784,
    100
  );
}

// =====================================================
// CURIOUS
// =====================================================

void curiousSound() {

  note(
    BUZZER_1,
    523,
    60
  );

  note(
    BUZZER_2,
    659,
    60
  );

  note(
    BUZZER_1,
    880,
    90
  );
}

// =====================================================
// HAPPY
// =====================================================

void happySound() {

  note(
    BUZZER_1,
    523,
    55
  );

  note(
    BUZZER_2,
    659,
    55
  );

  note(
    BUZZER_1,
    784,
    55
  );

  note(
    BUZZER_2,
    1047,
    120
  );
}

// =====================================================
// EXCITED
// =====================================================

void excitedSound() {

  note(
    BUZZER_1,
    784,
    35
  );

  note(
    BUZZER_2,
    988,
    35
  );

  note(
    BUZZER_1,
    1175,
    35
  );

  note(
    BUZZER_2,
    1397,
    80
  );
}

// =====================================================
// SHY
// =====================================================

void shySound() {

  note(
    BUZZER_2,
    392,
    120
  );

  note(
    BUZZER_1,
    330,
    150
  );
}

// =====================================================
// TRACKING
// =====================================================

void trackingSound() {

  if (
    millis() - lastSound <
    600
  ) {

    return;
  }

  lastSound =
    millis();

  if (
    lastVision ==
    "LEFT"
  ) {

    note(
      BUZZER_1,
      440,
      35
    );

  } else {

    note(
      BUZZER_2,
      660,
      35
    );
  }
}

// =====================================================
// CONFUSED
// =====================================================

void confusedSound() {

  note(
    BUZZER_1,
    700,
    50
  );

  note(
    BUZZER_2,
    500,
    50
  );

  note(
    BUZZER_1,
    700,
    50
  );

  note(
    BUZZER_2,
    500,
    70
  );
}

// =====================================================
// SEARCHING
// =====================================================

void searchingSound() {

  note(
    BUZZER_1,
    330,
    70
  );

  note(
    BUZZER_2,
    440,
    70
  );

  note(
    BUZZER_1,
    550,
    100
  );
}

// =====================================================
// SLEEPY
// =====================================================

void sleepySound() {

  note(
    BUZZER_1,
    330,
    180
  );

  note(
    BUZZER_2,
    262,
    250
  );
}

// =====================================================
// ALERT
// =====================================================

void alertSound() {

  note(
    BUZZER_1,
    1200,
    50
  );

  note(
    BUZZER_2,
    1200,
    50
  );

  note(
    BUZZER_1,
    1500,
    80
  );
}

// =====================================================
// CALM
// =====================================================

void calmSound() {

  note(
    BUZZER_1,
    659,
    60
  );

  note(
    BUZZER_2,
    523,
    100
  );
}