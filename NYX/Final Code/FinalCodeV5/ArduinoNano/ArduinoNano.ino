#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// =====================================================
// NYX V5
// Arduino Nano Desktop Pet
// =====================================================

// ---------------- PIN CONFIG ----------------

#define TRIG_PIN 2
#define ECHO_PIN 3

#define TOUCH_HEAD 4
#define TOUCH_LEFT 5

#define ESP_RX 6
#define ESP_TX 7

#define PAN_PIN 9
#define TILT_PIN 10

#define TOUCH_RIGHT 11

#define BUZZER_1 A0
#define BUZZER_2 A1

// =====================================================
// OLED
// =====================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// =====================================================
// SERIAL
// =====================================================

SoftwareSerial espSerial(
  ESP_RX,
  ESP_TX
);

// =====================================================
// SERVOS
// =====================================================

Servo panServo;
Servo tiltServo;

int panCurrent = 80;
int panTarget = 80;

int tiltCurrent = 90;
int tiltTarget = 90;

const int PAN_LEFT = 30;
const int PAN_CENTER = 80;
const int PAN_RIGHT = 130;

const int TILT_UP = 50;
const int TILT_CENTER = 90;
const int TILT_DOWN = 140;

// =====================================================
// TOUCH
// =====================================================

bool lastHead = LOW;
bool lastLeft = LOW;
bool lastRight = LOW;

unsigned long lastTouch = 0;

int touchCount = 0;

unsigned long touchWindow = 0;

// =====================================================
// ULTRASONIC
// =====================================================

float distanceCM = -1;

bool nearby = false;

unsigned long lastDistanceCheck = 0;

const unsigned long DISTANCE_INTERVAL = 250;

// =====================================================
// PET STATES
// =====================================================

enum Emotion {

  IDLE,

  HELLO,

  HAPPY,

  CURIOUS,

  EXCITED,

  ALERT,

  SHY,

  SEARCHING,

  SLEEPY,

  CONFUSED
};

Emotion emotion = IDLE;

unsigned long emotionUntil = 0;

unsigned long lastActivity = 0;

unsigned long lastVision = 0;

const unsigned long SLEEP_TIME = 45000;

const unsigned long TOUCH_REACTION = 3000;

// =====================================================
// VISION
// =====================================================

String visionDirection = "CENTER";

unsigned long lastVisionEvent = 0;

const unsigned long VISION_TIMEOUT = 3000;

// =====================================================
// IDLE
// =====================================================

unsigned long lastIdleMove = 0;

unsigned long nextIdleMove = 5000;

// =====================================================
// BLINK
// =====================================================

unsigned long nextBlink = 3000;

// =====================================================
// SOUND
// =====================================================

unsigned long lastSound = 0;

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  espSerial.begin(9600);

  // Ultrasonic

  pinMode(
    TRIG_PIN,
    OUTPUT
  );

  pinMode(
    ECHO_PIN,
    INPUT
  );

  // Touch

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

  // Buzzers

  pinMode(
    BUZZER_1,
    OUTPUT
  );

  pinMode(
    BUZZER_2,
    OUTPUT
  );

  // Servos

  panServo.attach(
    PAN_PIN
  );

  tiltServo.attach(
    TILT_PIN
  );

  panServo.write(
    PAN_CENTER
  );

  tiltServo.write(
    TILT_CENTER
  );

  panCurrent =
    PAN_CENTER;

  panTarget =
    PAN_CENTER;

  tiltCurrent =
    TILT_CENTER;

  tiltTarget =
    TILT_CENTER;

  // OLED

  Wire.begin();

  if (
    !display.begin(
      SSD1306_SWITCHCAPVCC,
      0x3C
    )
  ) {

    Serial.println(
      "OLED FAILED"
    );

    while (true) {
      delay(100);
    }
  }

  randomSeed(
    analogRead(A2)
  );

  Serial.println();
  Serial.println(
    "======================"
  );
  Serial.println(
    "NYX V5 STARTED"
  );
  Serial.println(
    "======================"
  );

  startup();

  lastActivity =
    millis();

  lastVision =
    millis();
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  updateServos();

  checkTouch();

  checkUltrasonic();

  readESP32();

  updatePet();

  updateIdle();

  updateBlink();

  delay(5);
}

// =====================================================
// STARTUP
// =====================================================

void startup() {

  sleepyEyes();

  delay(500);

  normalEyes();

  beep(
    700,
    80
  );

  beep(
    900,
    80
  );

  beep(
    1100,
    120
  );

  panTarget =
    PAN_LEFT;

  delay(250);

  panTarget =
    PAN_RIGHT;

  delay(250);

  panTarget =
    PAN_CENTER;

  delay(300);

  tiltTarget =
    TILT_UP;

  delay(250);

  tiltTarget =
    TILT_CENTER;

  setEmotion(
    HAPPY,
    1500
  );
}

// =====================================================
// SERVO ENGINE
// =====================================================

void updateServos() {

  static unsigned long lastServo = 0;

  if (
    millis() - lastServo < 15
  ) {
    return;
  }

  lastServo =
    millis();

  if (
    abs(
      panTarget - panCurrent
    ) > 1
  ) {

    if (
      panCurrent < panTarget
    ) {

      panCurrent++;

    } else {

      panCurrent--;
    }

    panServo.write(
      panCurrent
    );
  }

  if (
    abs(
      tiltTarget - tiltCurrent
    ) > 1
  ) {

    if (
      tiltCurrent < tiltTarget
    ) {

      tiltCurrent++;

    } else {

      tiltCurrent--;
    }

    tiltServo.write(
      tiltCurrent
    );
  }
}

// =====================================================
// TOUCH
// =====================================================

void checkTouch() {

  bool head =
    digitalRead(
      TOUCH_HEAD
    );

  bool left =
    digitalRead(
      TOUCH_LEFT
    );

  bool right =
    digitalRead(
      TOUCH_RIGHT
    );

  // Head

  if (
    head == HIGH &&
    lastHead == LOW
  ) {

    touchEvent(
      "HEAD"
    );
  }

  // Left

  if (
    left == HIGH &&
    lastLeft == LOW
  ) {

    touchEvent(
      "LEFT"
    );
  }

  // Right

  if (
    right == HIGH &&
    lastRight == LOW
  ) {

    touchEvent(
      "RIGHT"
    );
  }

  // Three sensors together

  if (
    head == HIGH &&
    left == HIGH &&
    right == HIGH
  ) {

    if (
      millis() - lastTouch >
      1000
    ) {

      lastTouch =
        millis();

      Serial.println(
        "SPECIAL TOUCH"
      );

      setEmotion(
        EXCITED,
        4000
      );

      specialSound();

      wiggle();
    }
  }

  lastHead =
    head;

  lastLeft =
    left;

  lastRight =
    right;
}

// =====================================================
// TOUCH EVENT
// =====================================================

void touchEvent(
  String type
) {

  lastTouch =
    millis();

  lastActivity =
    millis();

  // Touch counter

  if (
    millis() - touchWindow >
    8000
  ) {

    touchWindow =
      millis();

    touchCount =
      0;
  }

  touchCount++;

  Serial.print(
    "TOUCH: "
  );

  Serial.println(
    type
  );

  // Head

  if (
    type == "HEAD"
  ) {

    panTarget =
      PAN_CENTER;

    tiltTarget =
      TILT_UP;

    setEmotion(
      HAPPY,
      TOUCH_REACTION
    );

    happySound();

    delay(150);

    tiltTarget =
      TILT_CENTER;
  }

  // Left

  else if (
    type == "LEFT"
  ) {

    panTarget =
      PAN_LEFT;

    setEmotion(
      CURIOUS,
      TOUCH_REACTION
    );

    curiousSound();
  }

  // Right

  else if (
    type == "RIGHT"
  ) {

    panTarget =
      PAN_RIGHT;

    setEmotion(
      EXCITED,
      TOUCH_REACTION
    );

    excitedSound();
  }

  // Many touches

  if (
    touchCount >= 5
  ) {

    Serial.println(
      "NYX LOVES THE PETTING"
    );

    setEmotion(
      HAPPY,
      5000
    );

    specialSound();

    wiggle();

    touchCount =
      0;
  }
}

// =====================================================
// ULTRASONIC
// =====================================================

void checkUltrasonic() {

  if (
    millis() - lastDistanceCheck <
    DISTANCE_INTERVAL
  ) {

    return;
  }

  lastDistanceCheck =
    millis();

  distanceCM =
    getDistance();

  bool detected =
    distanceCM > 0 &&
    distanceCM < 45;

  // Object entered range

  if (
    detected &&
    !nearby
  ) {

    nearby =
      true;

    lastActivity =
      millis();

    Serial.print(
      "OBJECT ENTERED: "
    );

    Serial.print(
      distanceCM
    );

    Serial.println(
      " cm"
    );

    setEmotion(
      HELLO,
      2000
    );

    helloSound();

    panTarget =
      PAN_CENTER;

    tiltTarget =
      TILT_CENTER;

    // Tell ESP32 to inspect

    espSerial.println(
      "INSPECT"
    );
  }

  // Very close

  if (
    detected &&
    distanceCM < 10
  ) {

    setEmotion(
      ALERT,
      1200
    );

    alertSound();

    panTarget =
      PAN_CENTER;
  }

  // Object leaves

  if (
    !detected &&
    nearby
  ) {

    nearby =
      false;

    Serial.println(
      "OBJECT LEFT"
    );

    panTarget =
      PAN_CENTER;

    tiltTarget =
      TILT_CENTER;

    setEmotion(
      IDLE,
      1000
    );
  }
}

// =====================================================
// DISTANCE
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
      15000
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
// ESP32
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

    if (
      command.length() == 0
    ) {

      return;
    }

    Serial.print(
      "ESP32: "
    );

    Serial.println(
      command
    );

    lastActivity =
      millis();

    lastVision =
      millis();

    // Current ESP32 code

    if (
      command == "CURIOUS"
    ) {

      if (
        nearby
      ) {

        setEmotion(
          CURIOUS,
          2500
        );

        curiousSound();
      }
    }

    // Future LEFT

    else if (
      command == "LEFT"
    ) {

      visionDirection =
        "LEFT";

      panTarget =
        PAN_LEFT;

      setEmotion(
        CURIOUS,
        2500
      );

      curiousSound();
    }

    // Future RIGHT

    else if (
      command == "RIGHT"
    ) {

      visionDirection =
        "RIGHT";

      panTarget =
        PAN_RIGHT;

      setEmotion(
        CURIOUS,
        2500
      );

      curiousSound();
    }

    // Future CENTER

    else if (
      command == "CENTER"
    ) {

      visionDirection =
        "CENTER";

      panTarget =
        PAN_CENTER;

      setEmotion(
        HAPPY,
        2000
      );

      happySound();
    }

    // Fast movement

    else if (
      command == "FAST"
    ) {

      setEmotion(
        EXCITED,
        3000
      );

      excitedSound();
    }

    // Lost target

    else if (
      command == "LOST"
    ) {

      searchBehavior();
    }

    // ESP32 wake

    else if (
      command == "WAKE"
    ) {

      lastActivity =
        millis();

      setEmotion(
        HAPPY,
        1500
      );
    }
  }
}

// =====================================================
// PET STATE
// =====================================================

void updatePet() {

  if (
    millis() < emotionUntil
  ) {

    return;
  }

  if (
    millis() - lastActivity >
    SLEEP_TIME
  ) {

    if (
      emotion != SLEEPY
    ) {

      setEmotion(
        SLEEPY,
        0
      );

      sleepySound();
    }

    return;
  }

  if (
    emotion == SLEEPY
  ) {

    setEmotion(
      IDLE,
      1000
    );

    return;
  }

  if (
    emotion == HELLO ||
    emotion == HAPPY ||
    emotion == CURIOUS ||
    emotion == EXCITED ||
    emotion == ALERT
  ) {

    setEmotion(
      IDLE,
      0
    );
  }
}

// =====================================================
// IDLE MOVEMENT
// =====================================================

void updateIdle() {

  if (
    emotion != IDLE
  ) {

    return;
  }

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

  int movement =
    random(
      0,
      5
    );

  if (
    movement == 0
  ) {

    panTarget =
      PAN_LEFT;
  }

  else if (
    movement == 1
  ) {

    panTarget =
      PAN_RIGHT;
  }

  else if (
    movement == 2
  ) {

    tiltTarget =
      TILT_UP;
  }

  else if (
    movement == 3
  ) {

    panTarget =
      PAN_CENTER;

    tiltTarget =
      TILT_DOWN;
  }

  else {

    panTarget =
      PAN_CENTER;

    tiltTarget =
      TILT_CENTER;
  }

  nextIdleMove =
    random(
      4000,
      8000
    );
}

// =====================================================
// BLINK
// =====================================================

void updateBlink() {

  if (
    millis() <
    nextBlink
  ) {

    return;
  }

  if (
    emotion == SLEEPY
  ) {

    nextBlink =
      millis() + 3000;

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
// EMOTION
// =====================================================

void setEmotion(
  Emotion newEmotion,
  unsigned long duration
) {

  emotion =
    newEmotion;

  emotionUntil =
    millis() + duration;

  switch (
    emotion
  ) {

    case IDLE:
      normalEyes();
      break;

    case HELLO:
      helloEyes();
      break;

    case HAPPY:
      happyEyes();
      break;

    case CURIOUS:
      curiousEyes();
      break;

    case EXCITED:
      excitedEyes();
      break;

    case ALERT:
      alertEyes();
      break;

    case SHY:
      shyEyes();
      break;

    case SEARCHING:
      searchingEyes();
      break;

    case SLEEPY:
      sleepyEyes();
      break;

    case CONFUSED:
      confusedEyes();
      break;
  }
}

// =====================================================
// SEARCH
// =====================================================

void searchBehavior() {

  setEmotion(
    SEARCHING,
    3500
  );

  Serial.println(
    "NYX SEARCHING"
  );

  panTarget =
    PAN_LEFT;

  delay(500);

  panTarget =
    PAN_CENTER;

  delay(500);

  panTarget =
    PAN_RIGHT;

  delay(500);

  panTarget =
    PAN_CENTER;
}

// =====================================================
// WIGGLE
// =====================================================

void wiggle() {

  panTarget =
    PAN_LEFT;

  delay(180);

  panTarget =
    PAN_RIGHT;

  delay(180);

  panTarget =
    PAN_LEFT;

  delay(180);

  panTarget =
    PAN_CENTER;
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
    10,
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
    12,
    10,
    42,
    44
  );

  drawEye(
    74,
    10,
    42,
    44
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
    8,
    7,
    45,
    48
  );

  drawEye(
    77,
    14,
    39,
    40
  );

  display.display();
}

void excitedEyes() {

  display.clearDisplay();

  drawEye(
    5,
    5,
    49,
    51
  );

  drawEye(
    74,
    5,
    49,
    51
  );

  display.display();
}

void alertEyes() {

  display.clearDisplay();

  drawEye(
    12,
    8,
    42,
    47
  );

  drawEye(
    74,
    8,
    42,
    47
  );

  display.display();
}

void shyEyes() {

  display.clearDisplay();

  display.fillRoundRect(
    18,
    29,
    32,
    12,
    5,
    SSD1306_WHITE
  );

  display.fillRoundRect(
    78,
    29,
    32,
    12,
    5,
    SSD1306_WHITE
  );

  display.display();
}

void searchingEyes() {

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
    12,
    SSD1306_WHITE
  );

  display.display();
}

void blinkEyes() {

  display.clearDisplay();

  display.fillRoundRect(
    15,
    31,
    38,
    5,
    3,
    SSD1306_WHITE
  );

  display.fillRoundRect(
    75,
    31,
    38,
    5,
    3,
    SSD1306_WHITE
  );

  display.display();

  delay(80);

  switch (
    emotion
  ) {

    case HAPPY:
      happyEyes();
      break;

    case CURIOUS:
      curiousEyes();
      break;

    case EXCITED:
      excitedEyes();
      break;

    case ALERT:
      alertEyes();
      break;

    case SLEEPY:
      sleepyEyes();
      break;

    default:
      normalEyes();
      break;
  }
}

// =====================================================
// BUZZER FUNCTIONS
// =====================================================

void beep(
  int frequency,
  int duration
) {

  tone(
    BUZZER_1,
    frequency,
    duration
  );

  delay(
    duration
  );

  noTone(
    BUZZER_1
  );

  delay(20);
}

void happySound() {

  beep(
    600,
    60
  );

  beep(
    800,
    60
  );

  beep(
    1000,
    100
  );
}

void curiousSound() {

  beep(
    500,
    70
  );

  beep(
    700,
    100
  );

  beep(
    900,
    130
  );
}

void excitedSound() {

  beep(
    700,
    45
  );

  beep(
    900,
    45
  );

  beep(
    1100,
    45
  );

  beep(
    1400,
    100
  );
}

void helloSound() {

  beep(
    650,
    70
  );

  beep(
    850,
    100
  );
}

void alertSound() {

  beep(
    1300,
    70
  );

  beep(
    1300,
    70
  );

  beep(
    1600,
    100
  );
}

void sleepySound() {

  beep(
    350,
    150
  );

  beep(
    280,
    220
  );
}

void specialSound() {

  beep(
    600,
    50
  );

  beep(
    750,
    50
  );

  beep(
    900,
    50
  );

  beep(
    1100,
    70
  );

  beep(
    1400,
    120
  );
}