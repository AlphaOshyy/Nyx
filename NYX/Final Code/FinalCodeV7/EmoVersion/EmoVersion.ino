#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <EEPROM.h>

// =============================================================================
// NYX V6 - EMO-Style Interactive Desktop Robot Firmware
// Silent Ultrasonic Detection, Melodic Singing Voice, Ultra-Cute Eye Graphics
// =============================================================================

// ================= MUSICAL NOTES (WARM FREQUENCIES) =================
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_C6  1046

// ================= HARDWARE CONFIGURATION =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDR 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pan Servo
Servo panServo;
#define PAN_PIN 9
#define SERVO_CENTER 80
#define SERVO_MIN 25
#define SERVO_MAX 135

// Touch Sensors
#define TOUCH_HEAD 4
#define TOUCH_LEFT
#define TOUCH_RIGHT 5

// Ultrasonic Sensor
#define TRIG_PIN 2
#define ECHO_PIN 3

// Dual Buzzers
#define BUZZER_LEFT A0
#define BUZZER_RIGHT A1

// EEPROM Storage
#define EEPROM_MAGIC 0x4E // 'N'
#define EEPROM_ADDR_MAGIC 0
#define EEPROM_ADDR_AFFECTION 1
#define EEPROM_ADDR_MOOD 2
#define EEPROM_ADDR_INTERACTIONS_L 3
#define EEPROM_ADDR_INTERACTIONS_H 4

// ================= SYSTEM STATES & ENUMS =================
enum RobotState {
  STATE_IDLE,
  STATE_SLEEP,
  STATE_INTERACTING,
  STATE_GAME
};

enum Emotion {
  EMO_NORMAL,
  EMO_HAPPY,
  EMO_SAD,
  EMO_SLEEPY,
  EMO_EXCITED,
  EMO_CURIOUS,
  EMO_BORED,
  EMO_SCARED,
  EMO_LOVE,
  EMO_ANGRY,
  EMO_DIZZY
};

enum LookDirection {
  LOOK_CENTER,
  LOOK_LEFT,
  LOOK_RIGHT
};

// ================= GLOBAL SYSTEM VARIABLES =================
RobotState currentState = STATE_IDLE;
Emotion currentEmotion = EMO_NORMAL;
LookDirection currentLook = LOOK_CENTER;

// Personality & Mood Parameters (0 - 100)
int8_t moodScore = 75;
int8_t affectionScore = 50;
uint16_t totalInteractions = 0;

// Non-blocking Timer Intervals (ms)
#define SENSOR_INTERVAL 30
#define MOOD_DECAY_INTERVAL 20000
#define IDLE_ACTION_INTERVAL 4500
#define IDLE_MUSIC_INTERVAL 22000 // Sings a cute melody periodically
#define SLEEP_TIMEOUT 70000
#define EEPROM_SAVE_INTERVAL 60000

unsigned long lastSensorTime = 0;
unsigned long lastMoodDecayTime = 0;
unsigned long lastIdleActionTime = 0;
unsigned long lastIdleMusicTime = 0;
unsigned long lastInteractionTime = 0;
unsigned long lastEEPROMSaveTime = 0;
unsigned long expressionStartTime = 0;
unsigned long expressionDuration = 0;

// Servo Motion Smoothing
int currentPanPos = SERVO_CENTER;
int targetPanPos = SERVO_CENTER;
unsigned long lastServoStepTime = 0;
uint8_t servoSpeed = 15; // ms per step

// Touch Tracking
int lastHeadState = LOW;
int lastLeftState = LOW;
int lastRightState = LOW;

unsigned long headTouchStartTime = 0;
bool headIsPressed = false;
uint8_t petStreakCount = 0;
unsigned long lastPetTime = 0;

// Tickle Combo (L -> R -> L)
uint8_t tickleStep = 0;
unsigned long lastTickleStepTime = 0;

// Ultrasonic Tracking
float currentDistance = -1.0;
float lastDistance = -1.0;
unsigned long lastUltraReadTime = 0;

// Mini-game Variables (Reflex Duel)
uint8_t gameScore = 0;
int gameTargetSide = 0;
unsigned long gameCueTime = 0;
unsigned long gameTimeLimit = 1400;
bool gameWaitingInput = false;

// ================= FUNCTION DECLARATIONS =================
void loadEEPROM();
void saveEEPROM();
void updateSensors();
void updateMoodAndState();
void updateServo();
void setEmotion(Emotion emo, unsigned long durationMs = 0);
void setPanTarget(int targetAngle, uint8_t speedMs = 15);

// Eye Renderers
void renderEyes();
void drawEye(int x, int y, int w, int h, int r, bool shine = true);
void drawBlush(int x1, int y1, int x2, int y2);
void drawNormalEyes(int offsetX = 0, int offsetY = 0);
void drawBlinkAnimation();
void drawHappyEyes();
void drawExcitedEyes();
void drawCuriousEyes();
void drawSleepyEyes();
void drawLoveEyes();
void drawAngryEyes();
void drawScaredEyes();
void drawDizzyEyes();
void drawSadEyes();

// Melodic Singing Audio Engine
void singNote(int noteFreq, int durationMs);
void singHarmony(int noteLeft, int noteRight, int durationMs);
void playSingingTwinkle();
void playSingingHappy();
void playSingingLullaby();
void playSingingPopTune();
void playIdleSong();

void soundSoftChirp();
void soundSoftPurr();
void soundSoftHappy();
void soundSoftLove();
void soundSoftGasp();
void soundSoftYawn();
void soundSoftGiggle();
void soundGameStart();
void soundGameWin();
void soundGameLose();
void soundGameBeep();

// Actions
void performRandomIdle();
void handleTouchGesture(bool head, bool left, bool right);
void checkProximityEvents();
void startReflexGame();
void updateReflexGame();

// =============================================================================
// SETUP
// =============================================================================
void setup() {
  Serial.begin(9600);
  Serial.println(F("NYX V6 Starting..."));

  loadEEPROM();

  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  setEmotion(EMO_NORMAL);

  panServo.attach(PAN_PIN);
  panServo.write(SERVO_CENTER);
  currentPanPos = SERVO_CENTER;
  targetPanPos = SERVO_CENTER;

  pinMode(TOUCH_HEAD, INPUT);
  pinMode(TOUCH_LEFT, INPUT);
  pinMode(TOUCH_RIGHT, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  pinMode(BUZZER_LEFT, OUTPUT);
  pinMode(BUZZER_RIGHT, OUTPUT);
  noTone(BUZZER_LEFT);
  noTone(BUZZER_RIGHT);

  // Friendly Wake-up Singing
  setEmotion(EMO_EXCITED, 1200);
  soundSoftHappy();
  setEmotion(EMO_NORMAL);

  lastInteractionTime = millis();
  lastIdleMusicTime = millis();
  Serial.println(F("NYX V6 Online"));
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  unsigned long currentMillis = millis();

  // 1. Process Hardware Sensors
  if (currentMillis - lastSensorTime >= SENSOR_INTERVAL) {
    lastSensorTime = currentMillis;
    updateSensors();
  }

  // 2. Smooth Servo Motion
  updateServo();

  // 3. System State Machine
  if (currentState == STATE_GAME) {
    updateReflexGame();
  } else {
    updateMoodAndState();
  }

  // 4. Expression Duration Tracker
  if (expressionDuration > 0 && (currentMillis - expressionStartTime >= expressionDuration)) {
    expressionDuration = 0;
    if (currentState == STATE_SLEEP) {
      setEmotion(EMO_SLEEPY);
    } else {
      setEmotion(EMO_NORMAL);
    }
  }

  // 5. Periodic EEPROM Sync
  if (currentMillis - lastEEPROMSaveTime >= EEPROM_SAVE_INTERVAL) {
    lastEEPROMSaveTime = currentMillis;
    saveEEPROM();
  }
}

// =============================================================================
// SENSOR PROCESSING & TOUCH GESTURES
// =============================================================================
void updateSensors() {
  unsigned long now = millis();
  int head = digitalRead(TOUCH_HEAD);
  int left = digitalRead(TOUCH_LEFT);
  int right = digitalRead(TOUCH_RIGHT);

  // Ultrasonic distance reading (Silent - NO buzzers played)
  if (now - lastUltraReadTime >= 120) {
    lastUltraReadTime = now;
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    long duration = pulseIn(ECHO_PIN, HIGH, 18000);
    if (duration > 0) {
      lastDistance = currentDistance;
      currentDistance = duration * 0.0343 / 2.0;
      checkProximityEvents();
    } else {
      currentDistance = -1.0;
    }
  }

  handleTouchGesture(head == HIGH, left == HIGH, right == HIGH);

  lastHeadState = head;
  lastLeftState = left;
  lastRightState = right;
}

void handleTouchGesture(bool head, bool left, bool right) {
  unsigned long now = millis();

  // --- PRESS BOTH LEFT AND RIGHT -> TOGGLE GAME MODE ---
  if (left && right && (!lastLeftState || !lastRightState)) {
    lastInteractionTime = now;
    totalInteractions++;
    if (currentState == STATE_GAME) {
      currentState = STATE_IDLE;
      setEmotion(EMO_NORMAL, 1000);
      soundSoftChirp();
      setPanTarget(SERVO_CENTER, 15);
    } else {
      startReflexGame();
    }
    return;
  }

  // Mini-game input handling
  if (currentState == STATE_GAME) {
    if (gameWaitingInput) {
      if ((gameTargetSide == 0 && left && !lastLeftState) ||
          (gameTargetSide == 1 && right && !lastRightState)) {
        gameScore++;
        gameWaitingInput = false;
        setEmotion(EMO_HAPPY, 400);
        soundGameWin();
        if (gameTimeLimit > 400) gameTimeLimit -= 70;
        setPanTarget(SERVO_CENTER, 8);
        gameCueTime = now + 500;
      } else if ((gameTargetSide == 0 && right && !lastRightState) ||
                 (gameTargetSide == 1 && left && !lastLeftState) ||
                 (head && !lastHeadState)) {
        gameWaitingInput = false;
        currentState = STATE_IDLE;
        setEmotion(EMO_DIZZY, 2000);
        soundGameLose();
        setPanTarget(SERVO_CENTER, 15);
      }
    }
    return;
  }

  // Wake up if sleeping
  if (currentState == STATE_SLEEP) {
    if ((head && !lastHeadState) || (left && !lastLeftState) || (right && !lastRightState)) {
      lastInteractionTime = now;
      currentState = STATE_IDLE;
      setEmotion(EMO_EXCITED, 1000);
      soundSoftHappy();
      setPanTarget(SERVO_CENTER, 12);
      return;
    }
  }

  // --- TRIPLE TOUCH (Head + Left + Right) -> Cheerful Song & Dance ---
  if (head && left && right) {
    if (!lastHeadState || !lastLeftState || !lastRightState) {
      lastInteractionTime = now;
      totalInteractions++;
      moodScore = min(100, moodScore + 10);
      setEmotion(EMO_EXCITED, 2800);
      playSingingHappy();
      return;
    }
  }

  // --- HEAD TOUCH (Poke vs Sustained Pet vs Pet Streak) ---
  if (head) {
    lastInteractionTime = now;
    if (!headIsPressed) {
      headIsPressed = true;
      headTouchStartTime = now;
    } else {
      if (now - headTouchStartTime > 500) { // Sustained petting
        if (now - lastPetTime > 350) {
          lastPetTime = now;
          petStreakCount++;
          moodScore = min(100, moodScore + 4);
          affectionScore = min(100, affectionScore + 3);
          if (petStreakCount >= 3) {
            setEmotion(EMO_LOVE, 1600);
            soundSoftLove();
          } else {
            setEmotion(EMO_HAPPY, 800);
            soundSoftPurr();
          }
        }
      }
    }
  } else {
    if (headIsPressed) {
      headIsPressed = false;
      unsigned long duration = now - headTouchStartTime;
      if (duration < 500) { // Quick poke
        totalInteractions++;
        moodScore = min(100, moodScore + 2);
        setEmotion(EMO_HAPPY, 1000);
        soundSoftChirp();
      }
    }
  }

  if (now - lastPetTime > 3000) {
    petStreakCount = 0;
  }

  // --- LEFT CHEEK TOUCH ---
  if (left && !lastLeftState) {
    lastInteractionTime = now;
    totalInteractions++;
    setPanTarget(SERVO_MIN + 10, 10);
    setEmotion(EMO_CURIOUS, 1000);
    soundSoftChirp();

    if (tickleStep == 0 || (now - lastTickleStepTime > 1200)) {
      tickleStep = 1;
      lastTickleStepTime = now;
    } else if (tickleStep == 2) {
      tickleStep = 0;
      moodScore = min(100, moodScore + 10);
      setEmotion(EMO_EXCITED, 1800);
      soundSoftGiggle();
      setPanTarget(SERVO_MAX - 10, 8);
    }
  }

  // --- RIGHT CHEEK TOUCH ---
  if (right && !lastRightState) {
    lastInteractionTime = now;
    totalInteractions++;
    setPanTarget(SERVO_MAX - 10, 10);
    setEmotion(EMO_CURIOUS, 1000);
    soundSoftChirp();

    if (tickleStep == 1 && (now - lastTickleStepTime <= 1200)) {
      tickleStep = 2;
      lastTickleStepTime = now;
    } else {
      tickleStep = 0;
    }
  }
}

// =============================================================================
// ULTRASONIC DETECTION (100% SILENT - NO BUZZER SOUNDS)
// =============================================================================
void checkProximityEvents() {
  if (currentDistance <= 0) return;
  unsigned long now = millis();

  // Sudden intrusion (< 10cm): Scared expression and pan away (SILENT)
  if (lastDistance > 25.0 && currentDistance < 10.0 && currentState != STATE_SLEEP) {
    lastInteractionTime = now;
    moodScore = max(0, moodScore - 3);
    setEmotion(EMO_SCARED, 1400);
    setPanTarget(currentPanPos < SERVO_CENTER ? SERVO_MAX : SERVO_MIN, 6);
    return;
  }

  // Friendly approach (12 - 30cm): Curious expression (SILENT)
  if (currentDistance > 12.0 && currentDistance < 30.0 && currentState == STATE_IDLE) {
    lastInteractionTime = now;
    if (expressionDuration == 0) {
      setEmotion(EMO_CURIOUS, 1200);
      setPanTarget(SERVO_CENTER + (random(0, 2) == 0 ? 10 : -10), 12);
    }
  }
}

// =============================================================================
// STATE & MOOD MANAGEMENT WITH IDLE SINGING
// =============================================================================
void updateMoodAndState() {
  unsigned long now = millis();

  // Mood Decay
  if (now - lastMoodDecayTime >= MOOD_DECAY_INTERVAL) {
    lastMoodDecayTime = now;
    if (moodScore > 10) moodScore--;
  }

  // Sleep Inactivity Timer
  if (currentState == STATE_IDLE && (now - lastInteractionTime >= SLEEP_TIMEOUT)) {
    currentState = STATE_SLEEP;
    setEmotion(EMO_SLEEPY);
    playSingingLullaby();
    setPanTarget(SERVO_CENTER, 25);
    return;
  }

  // Periodic Singing in Idle
  if (currentState == STATE_IDLE && expressionDuration == 0) {
    if (now - lastIdleMusicTime >= IDLE_MUSIC_INTERVAL) {
      lastIdleMusicTime = now;
      playIdleSong();
      return;
    }
  }

  // Ambient Idle Animations
  if (currentState == STATE_IDLE && expressionDuration == 0) {
    if (now - lastIdleActionTime >= IDLE_ACTION_INTERVAL) {
      lastIdleActionTime = now;
      performRandomIdle();
    }
  }
}

void performRandomIdle() {
  int r = random(0, 100);

  if (r < 40) {
    // EMO Smooth Blink
    drawBlinkAnimation();
    renderEyes();
  } else if (r < 65) {
    // Look around & Pan
    int target = random(SERVO_MIN, SERVO_MAX);
    setPanTarget(target, 20);
    if (target < SERVO_CENTER - 15) currentLook = LOOK_LEFT;
    else if (target > SERVO_CENTER + 15) currentLook = LOOK_RIGHT;
    else currentLook = LOOK_CENTER;
    renderEyes();
  } else if (r < 80) {
    if (moodScore > 75) {
      setEmotion(EMO_HAPPY, 1200);
      soundSoftHappy();
    } else if (moodScore < 30) {
      setEmotion(EMO_SAD, 1400);
      soundSoftPurr();
    } else {
      setEmotion(EMO_BORED, 1200);
      soundSoftYawn();
    }
  } else if (r < 92) {
    soundSoftChirp();
    setEmotion(EMO_CURIOUS, 1000);
  } else {
    // Mini 2-note hum
    singNote(NOTE_G4, 100);
    singNote(NOTE_C5, 150);
  }
}

// =============================================================================
// NON-BLOCKING SERVO CONTROLLER
// =============================================================================
void setPanTarget(int targetAngle, uint8_t speedMs) {
  targetPanPos = constrain(targetAngle, SERVO_MIN, SERVO_MAX);
  servoSpeed = speedMs;
}

void updateServo() {
  unsigned long now = millis();
  if (currentPanPos != targetPanPos) {
    if (now - lastServoStepTime >= servoSpeed) {
      lastServoStepTime = now;
      if (currentPanPos < targetPanPos) currentPanPos++;
      else if (currentPanPos > targetPanPos) currentPanPos--;
      panServo.write(currentPanPos);
    }
  }
}

// =============================================================================
// MINI-GAME: REFLEX DUEL
// =============================================================================
void startReflexGame() {
  currentState = STATE_GAME;
  gameScore = 0;
  gameTimeLimit = 1400;
  soundGameStart();

  for (int i = 3; i >= 1; i--) {
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(55, 20);
    display.print(i);
    display.display();
    soundGameBeep();
    delay(500);
  }
  gameCueTime = millis() + 350;
}

void updateReflexGame() {
  unsigned long now = millis();

  if (!gameWaitingInput && now >= gameCueTime) {
    gameTargetSide = random(0, 2);
    gameWaitingInput = true;
    gameCueTime = now;

    display.clearDisplay();
    if (gameTargetSide == 0) {
      setPanTarget(SERVO_MIN, 7);
      drawEye(8, 12, 38, 40, 14, true);
      drawEye(68, 12, 38, 40, 14, true);
      display.fillTriangle(6, 32, 18, 20, 18, 44, SSD1306_WHITE);
    } else {
      setPanTarget(SERVO_MAX, 7);
      drawEye(22, 12, 38, 40, 14, true);
      drawEye(82, 12, 38, 40, 14, true);
      display.fillTriangle(122, 32, 110, 20, 110, 44, SSD1306_WHITE);
    }
    display.display();
    tone(BUZZER_LEFT, NOTE_G4, 60);
  }

  if (gameWaitingInput && (now - gameCueTime > gameTimeLimit)) {
    gameWaitingInput = false;
    currentState = STATE_IDLE;
    setEmotion(EMO_DIZZY, 2000);
    soundGameLose();
    setPanTarget(SERVO_CENTER, 15);
  }
}

// =============================================================================
// ULTRA-CUTE EMO EYE GRAPHICS
// =============================================================================
void setEmotion(Emotion emo, unsigned long durationMs) {
  currentEmotion = emo;
  expressionStartTime = millis();
  expressionDuration = durationMs;
  renderEyes();
}

void drawEye(int x, int y, int w, int h, int r, bool shine) {
  display.fillRoundRect(x, y, w, h, r, SSD1306_WHITE);
  if (shine && w >= 32 && h >= 32) {
    // Primary large glossy shine
    display.fillCircle(x + w - 10, y + 10, 4, SSD1306_BLACK);
    // Secondary cute shimmer sparkle
    display.fillCircle(x + w - 6, y + 17, 2, SSD1306_BLACK);
    display.fillCircle(x + 10, y + h - 10, 2, SSD1306_BLACK);
  }
}

void drawBlush(int x1, int y1, int x2, int y2) {
  // Soft rounded blush marks
  display.fillRoundRect(x1, y1, 12, 4, 2, SSD1306_WHITE);
  display.fillRoundRect(x2, y2, 12, 4, 2, SSD1306_WHITE);
}

void renderEyes() {
  display.clearDisplay();
  switch (currentEmotion) {
    case EMO_NORMAL:
      if (currentLook == LOOK_LEFT) drawNormalEyes(-8, 0);
      else if (currentLook == LOOK_RIGHT) drawNormalEyes(8, 0);
      else drawNormalEyes(0, 0);
      break;
    case EMO_HAPPY: drawHappyEyes(); break;
    case EMO_EXCITED: drawExcitedEyes(); break;
    case EMO_CURIOUS: drawCuriousEyes(); break;
    case EMO_SLEEPY: drawSleepyEyes(); break;
    case EMO_LOVE: drawLoveEyes(); break;
    case EMO_ANGRY: drawAngryEyes(); break;
    case EMO_SCARED: drawScaredEyes(); break;
    case EMO_DIZZY: drawDizzyEyes(); break;
    case EMO_SAD: drawSadEyes(); break;
    case EMO_BORED: drawSleepyEyes(); break;
    default: drawNormalEyes(); break;
  }
  display.display();
}

void drawNormalEyes(int offsetX, int offsetY) {
  drawEye(18 + offsetX, 12 + offsetY, 38, 40, 14, true);
  drawEye(72 + offsetX, 12 + offsetY, 38, 40, 14, true);
}

void drawBlinkAnimation() {
  display.clearDisplay();
  display.fillRoundRect(18, 22, 38, 20, 10, SSD1306_WHITE);
  display.fillRoundRect(72, 22, 38, 20, 10, SSD1306_WHITE);
  display.display();
  delay(35);

  display.clearDisplay();
  display.fillRoundRect(18, 30, 38, 6, 3, SSD1306_WHITE);
  display.fillRoundRect(72, 30, 38, 6, 3, SSD1306_WHITE);
  display.display();
  delay(55);

  display.clearDisplay();
  display.fillRoundRect(18, 22, 38, 20, 10, SSD1306_WHITE);
  display.fillRoundRect(72, 22, 38, 20, 10, SSD1306_WHITE);
  display.display();
  delay(35);
}

void drawHappyEyes() {
  // Thick smiling crescents with blush
  for (int i = 0; i < 5; i++) {
    display.drawLine(15, 38 - i, 34, 18 - i, SSD1306_WHITE);
    display.drawLine(34, 18 - i, 53, 38 - i, SSD1306_WHITE);
    display.drawLine(75, 38 - i, 94, 18 - i, SSD1306_WHITE);
    display.drawLine(94, 18 - i, 113, 38 - i, SSD1306_WHITE);
  }
  drawBlush(15, 46, 99, 46);
}

void drawExcitedEyes() {
  drawEye(12, 6, 44, 48, 16, true);
  drawEye(72, 6, 44, 48, 16, true);
  display.fillCircle(24, 20, 4, SSD1306_BLACK);
  display.fillCircle(84, 20, 4, SSD1306_BLACK);
  drawBlush(10, 56, 106, 56);
}

void drawCuriousEyes() {
  drawEye(14, 6, 44, 46, 14, true);
  drawEye(74, 16, 38, 36, 12, true);
}

void drawSleepyEyes() {
  display.fillRoundRect(18, 26, 38, 24, 10, SSD1306_WHITE);
  display.fillRoundRect(72, 26, 38, 24, 10, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(110, 10);
  display.print(F("zZ"));
}

void drawLoveEyes() {
  display.fillCircle(28, 24, 11, SSD1306_WHITE);
  display.fillCircle(44, 24, 11, SSD1306_WHITE);
  display.fillTriangle(18, 27, 54, 27, 36, 47, SSD1306_WHITE);

  display.fillCircle(84, 24, 11, SSD1306_WHITE);
  display.fillCircle(100, 24, 11, SSD1306_WHITE);
  display.fillTriangle(74, 27, 110, 27, 92, 47, SSD1306_WHITE);

  drawBlush(14, 50, 102, 50);
}

void drawAngryEyes() {
  drawEye(18, 16, 38, 36, 10, false);
  drawEye(72, 16, 38, 36, 10, false);
  display.fillTriangle(14, 12, 58, 12, 58, 26, SSD1306_BLACK);
  display.fillTriangle(70, 12, 114, 12, 70, 26, SSD1306_BLACK);
}

void drawScaredEyes() {
  display.drawCircle(36, 32, 20, SSD1306_WHITE);
  display.drawCircle(36, 32, 19, SSD1306_WHITE);
  display.fillCircle(36, 32, 6, SSD1306_WHITE);

  display.drawCircle(92, 32, 20, SSD1306_WHITE);
  display.drawCircle(92, 32, 19, SSD1306_WHITE);
  display.fillCircle(92, 32, 6, SSD1306_WHITE);
}

void drawDizzyEyes() {
  display.drawLine(20, 16, 50, 46, SSD1306_WHITE);
  display.drawLine(50, 16, 20, 46, SSD1306_WHITE);
  display.drawLine(76, 16, 106, 46, SSD1306_WHITE);
  display.drawLine(106, 16, 76, 46, SSD1306_WHITE);
}

void drawSadEyes() {
  for (int i = 0; i < 4; i++) {
    display.drawLine(15, 20 + i, 34, 38 + i, SSD1306_WHITE);
    display.drawLine(34, 38 + i, 53, 20 + i, SSD1306_WHITE);
    display.drawLine(75, 20 + i, 94, 38 + i, SSD1306_WHITE);
    display.drawLine(94, 38 + i, 113, 20 + i, SSD1306_WHITE);
  }
  display.fillCircle(55, 48, 3, SSD1306_WHITE); // Teardrop
}

// =============================================================================
// COMFORTABLE & PLEASANT SINGING MUSIC SYNTHESIZER
// =============================================================================
void singNote(int noteFreq, int durationMs) {
  if (noteFreq <= 0) {
    delay(durationMs);
    return;
  }
  tone(BUZZER_LEFT, noteFreq, durationMs - 15);
  delay(durationMs);
  noTone(BUZZER_LEFT);
}

void singHarmony(int noteLeft, int noteRight, int durationMs) {
  tone(BUZZER_LEFT, noteLeft, durationMs - 15);
  tone(BUZZER_RIGHT, noteRight, durationMs - 15);
  delay(durationMs);
  noTone(BUZZER_LEFT);
  noTone(BUZZER_RIGHT);
}

void playIdleSong() {
  int choice = random(0, 3);
  if (choice == 0) playSingingTwinkle();
  else if (choice == 1) playSingingHappy();
  else playSingingPopTune();
}

void playSingingTwinkle() {
  // Twinkle melody motif with head groove
  int melody[] = { NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_G4 };
  int tempo[]  = { 180,     180,     180,     180,     180,     180,     340 };

  for (int i = 0; i < 7; i++) {
    setPanTarget(SERVO_CENTER + ((i % 2 == 0) ? 8 : -8), 12);
    singNote(melody[i], tempo[i]);
  }
  setPanTarget(SERVO_CENTER, 15);
}

void playSingingHappy() {
  // Cheerful singing melody
  int melody[] = { NOTE_E4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_E4, NOTE_D4, NOTE_C4 };
  int tempo[]  = { 150,     150,     150,     150,     150,     150,     150,     150,     280 };

  for (int i = 0; i < 9; i++) {
    setPanTarget(SERVO_CENTER + ((i % 2 == 0) ? 10 : -10), 10);
    singNote(melody[i], tempo[i]);
  }
  setPanTarget(SERVO_CENTER, 15);
}

void playSingingPopTune() {
  // Melodic harmony tune
  int melody[] = { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_G4, NOTE_C5 };
  int tempo[]  = { 120,     120,     120,     180,     120,     260 };

  for (int i = 0; i < 6; i++) {
    setPanTarget(SERVO_CENTER + ((i % 2 == 0) ? 7 : -7), 10);
    singHarmony(melody[i], melody[i] / 2, tempo[i]);
  }
  setPanTarget(SERVO_CENTER, 15);
}

void playSingingLullaby() {
  // Soft, warm sleep notes
  singNote(NOTE_G4, 250);
  singNote(NOTE_E4, 250);
  singNote(NOTE_C4, 400);
}

void soundSoftChirp() {
  singNote(NOTE_G4, 60);
  singNote(NOTE_C5, 80);
}

void soundSoftPurr() {
  singNote(NOTE_C4, 90);
  singNote(NOTE_E4, 90);
  singNote(NOTE_G4, 120);
}

void soundSoftHappy() {
  singNote(NOTE_C5, 80);
  singNote(NOTE_E5, 80);
  singNote(NOTE_G5, 140);
}

void soundSoftLove() {
  singHarmony(NOTE_C4, NOTE_G4, 120);
  singHarmony(NOTE_E4, NOTE_C5, 140);
  singHarmony(NOTE_G4, NOTE_E5, 220);
}

void soundSoftGiggle() {
  for (int i = 0; i < 3; i++) {
    singNote(NOTE_G4 + (i * 40), 40);
  }
}

void soundSoftYawn() {
  singNote(NOTE_G4, 180);
  singNote(NOTE_E4, 200);
  singNote(NOTE_C4, 280);
}

void soundGameStart() {
  singNote(NOTE_C4, 90);
  singNote(NOTE_E4, 90);
  singNote(NOTE_G4, 140);
}

void soundGameBeep() {
  singNote(NOTE_A4, 80);
}

void soundGameWin() {
  singHarmony(NOTE_G4, NOTE_C5, 100);
  singHarmony(NOTE_C5, NOTE_E5, 180);
}

void soundGameLose() {
  singNote(NOTE_E4, 140);
  singNote(NOTE_C4, 220);
}

// =============================================================================
// EEPROM STORAGE
// =============================================================================
void loadEEPROM() {
  if (EEPROM.read(EEPROM_ADDR_MAGIC) == EEPROM_MAGIC) {
    affectionScore = EEPROM.read(EEPROM_ADDR_AFFECTION);
    moodScore = EEPROM.read(EEPROM_ADDR_MOOD);
    totalInteractions = EEPROM.read(EEPROM_ADDR_INTERACTIONS_L) |
                        (EEPROM.read(EEPROM_ADDR_INTERACTIONS_H) << 8);
  } else {
    affectionScore = 50;
    moodScore = 75;
    totalInteractions = 0;
    saveEEPROM();
  }
}

void saveEEPROM() {
  EEPROM.update(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.update(EEPROM_ADDR_AFFECTION, affectionScore);
  EEPROM.update(EEPROM_ADDR_MOOD, moodScore);
  EEPROM.update(EEPROM_ADDR_INTERACTIONS_L, totalInteractions & 0xFF);
  EEPROM.update(EEPROM_ADDR_INTERACTIONS_H, (totalInteractions >> 8) & 0xFF);
}