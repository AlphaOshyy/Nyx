#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <EEPROM.h>

// =============================================================================
// NYX V7 - Desktop Interactive Companion
// Spider-Man & Kudda Melodies, Mute Toggle, Serial CLI, Silent Radar
// =============================================================================

// ================= MUSICAL NOTES =================
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_A5  880

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
#define TOUCH_LEFT 11
#define TOUCH_RIGHT 5

// Ultrasonic Sensor (Silent Detection Only)
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
#define EEPROM_ADDR_MUTE 5

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
  EMO_DIZZY,
  EMO_MUSIC
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

// Personality & Mood Parameters
int8_t moodScore = 80;
int8_t affectionScore = 50;
uint16_t totalInteractions = 0;
bool soundEnabled = true; // Mute toggle for quiet desktop use

// Non-blocking Timer Intervals (ms)
#define SENSOR_INTERVAL 30
#define MOOD_DECAY_INTERVAL 30000
#define IDLE_ACTION_INTERVAL 5000
#define IDLE_MUSIC_INTERVAL 90000 // Rare singing in idle (every 1.5 min)
#define SLEEP_TIMEOUT 90000       // Auto-sleep after 1.5 min of silence
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

// Touch Debouncing & Tracking
int lastHeadState = LOW;
int lastLeftState = LOW;
int lastRightState = LOW;

unsigned long headTouchStartTime = 0;
bool headIsPressed = false;
bool muteToggledThisPress = false;
uint8_t petStreakCount = 0;
unsigned long lastPetTime = 0;

// Tickle Combo (L -> R -> L)
uint8_t tickleStep = 0;
unsigned long lastTickleStepTime = 0;

// Ultrasonic Tracking (Silent)
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
void processSerialCLI();
void setEmotion(Emotion emo, unsigned long durationMs = 0);
void setPanTarget(int targetAngle, uint8_t speedMs = 15);
void toggleMuteMode();

// Display Graphics
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
void drawMusicEyes();
void showMuteNotification(bool isMuted);

// Melodic Singing Audio Engine
void singNote(int noteFreq, int durationMs);
void playSpiderManTheme();
void playKuddaRapMelody();
void playSoftHum();
void playSoftLullaby();

void soundSoftChirp();
void soundSoftPurr();
void soundSoftHappy();
void soundSoftLove();
void soundSoftYawn();
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
  Serial.println(F("NYX V7 Starting (Desktop Edition)..."));

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

  // Soft startup greeting
  setEmotion(EMO_EXCITED, 1000);
  soundSoftHappy();
  setEmotion(EMO_NORMAL);

  lastInteractionTime = millis();
  lastIdleMusicTime = millis();

  Serial.println(F("=================================="));
  Serial.println(F("NYX Desktop CLI Commands:"));
  Serial.println(F("  m : Toggle Mute / Sound"));
  Serial.println(F("  g : Toggle Game Mode"));
  Serial.println(F("  s : Play Spider-Man Tune"));
  Serial.println(F("  k : Play Kudda Rap Melody"));
  Serial.println(F("  h : Happy Emotion"));
  Serial.println(F("  z : Sleep Mode"));
  Serial.println(F("  w : Wake Up"));
  Serial.println(F("=================================="));
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  unsigned long currentMillis = millis();

  // 1. Process Desktop Serial Commands
  processSerialCLI();

  // 2. Process Hardware Sensors
  if (currentMillis - lastSensorTime >= SENSOR_INTERVAL) {
    lastSensorTime = currentMillis;
    updateSensors();
  }

  // 3. Smooth Servo Motion
  updateServo();

  // 4. System State Machine
  if (currentState == STATE_GAME) {
    updateReflexGame();
  } else {
    updateMoodAndState();
  }

  // 5. Timed Expression Expiration
  if (expressionDuration > 0 && (currentMillis - expressionStartTime >= expressionDuration)) {
    expressionDuration = 0;
    if (currentState == STATE_SLEEP) {
      setEmotion(EMO_SLEEPY);
    } else {
      setEmotion(EMO_NORMAL);
    }
  }

  // 6. Periodic EEPROM Save
  if (currentMillis - lastEEPROMSaveTime >= EEPROM_SAVE_INTERVAL) {
    lastEEPROMSaveTime = currentMillis;
    saveEEPROM();
  }
}

// =============================================================================
// DESKTOP SERIAL CLI CONTROLLER
// =============================================================================
void processSerialCLI() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    lastInteractionTime = millis();

    switch (cmd) {
      case 'm':
      case 'M':
        toggleMuteMode();
        break;
      case 'g':
      case 'G':
        if (currentState == STATE_GAME) {
          currentState = STATE_IDLE;
          setEmotion(EMO_NORMAL, 1000);
          setPanTarget(SERVO_CENTER, 15);
        } else {
          startReflexGame();
        }
        break;
      case 's':
      case 'S':
        setEmotion(EMO_MUSIC, 3500);
        playSpiderManTheme();
        setEmotion(EMO_HAPPY, 1000);
        break;
      case 'k':
      case 'K':
        setEmotion(EMO_MUSIC, 3500);
        playKuddaRapMelody();
        setEmotion(EMO_HAPPY, 1000);
        break;
      case 'h':
      case 'H':
        setEmotion(EMO_HAPPY, 1500);
        soundSoftHappy();
        break;
      case 'z':
      case 'Z':
        currentState = STATE_SLEEP;
        setEmotion(EMO_SLEEPY);
        setPanTarget(SERVO_CENTER, 25);
        break;
      case 'w':
      case 'W':
        currentState = STATE_IDLE;
        setEmotion(EMO_EXCITED, 1200);
        soundSoftHappy();
        setPanTarget(SERVO_CENTER, 12);
        break;
    }
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

  // Ultrasonic distance reading (Silent - NO buzzers)
  if (now - lastUltraReadTime >= 140) {
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

  // Mini-game input processor
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

  // --- TRIPLE TOUCH (Head + Left + Right) -> Play Kudda Rap Melody! ---
  if (head && left && right) {
    if (!lastHeadState || !lastLeftState || !lastRightState) {
      lastInteractionTime = now;
      totalInteractions++;
      moodScore = min(100, moodScore + 10);
      setEmotion(EMO_MUSIC, 3500);
      playKuddaRapMelody();
      return;
    }
  }

  // --- HEAD TOUCH (Poke vs Sustained Pet vs Hold to Mute) ---
  if (head) {
    lastInteractionTime = now;
    if (!headIsPressed) {
      headIsPressed = true;
      headTouchStartTime = now;
      muteToggledThisPress = false;
    } else {
      // Hold Head for 2.2 seconds to Toggle Mute / Sound Mode
      if (!muteToggledThisPress && (now - headTouchStartTime > 2200)) {
        muteToggledThisPress = true;
        toggleMuteMode();
      } else if (!muteToggledThisPress && (now - headTouchStartTime > 500)) {
        // Sustained petting
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
      if (!muteToggledThisPress && duration < 500) {
        // Quick poke
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

    // Tickle combo (L -> R -> L -> Plays Spider-Man!)
    if (tickleStep == 0 || (now - lastTickleStepTime > 1200)) {
      tickleStep = 1;
      lastTickleStepTime = now;
    } else if (tickleStep == 2) {
      tickleStep = 0;
      moodScore = min(100, moodScore + 10);
      setEmotion(EMO_MUSIC, 3500);
      playSpiderManTheme();
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
// ULTRASONIC DETECTION (100% SILENT - ZERO BUZZER SOUNDS)
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
// MUTE & SOUND CONTROLLER
// =============================================================================
void toggleMuteMode() {
  soundEnabled = !soundEnabled;
  showMuteNotification(soundEnabled);
  Serial.print(F("Sound Mode: "));
  Serial.println(soundEnabled ? F("ENABLED (ON)") : F("MUTED (OFF)"));
  if (soundEnabled) {
    singNote(NOTE_E5, 100);
    singNote(NOTE_A5, 140);
  }
}

void showMuteNotification(bool enabled) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 15);
  display.print(enabled ? F("SOUND ON") : F("MUTED"));

  // Speaker icon
  display.fillRoundRect(50, 40, 10, 14, 2, SSD1306_WHITE);
  display.fillTriangle(60, 40, 72, 32, 72, 60, SSD1306_WHITE);
  if (!enabled) {
    display.drawLine(46, 32, 78, 62, SSD1306_WHITE);
    display.drawLine(46, 33, 78, 63, SSD1306_WHITE);
  } else {
    display.drawCircle(78, 47, 5, SSD1306_WHITE);
  }
  display.display();
  delay(700);
  renderEyes();
}

// =============================================================================
// STATE & MOOD MANAGEMENT
// =============================================================================
void updateMoodAndState() {
  unsigned long now = millis();

  // Mood Decay
  if (now - lastMoodDecayTime >= MOOD_DECAY_INTERVAL) {
    lastMoodDecayTime = now;
    if (moodScore > 10) moodScore--;
  }

  // Auto-Sleep Timer
  if (currentState == STATE_IDLE && (now - lastInteractionTime >= SLEEP_TIMEOUT)) {
    currentState = STATE_SLEEP;
    setEmotion(EMO_SLEEPY);
    if (soundEnabled) playSoftLullaby();
    setPanTarget(SERVO_CENTER, 25);
    return;
  }

  // Rare Idle Music (Only every 90s, gentle & pleasant)
  if (currentState == STATE_IDLE && expressionDuration == 0) {
    if (now - lastIdleMusicTime >= IDLE_MUSIC_INTERVAL) {
      lastIdleMusicTime = now;
      if (soundEnabled && random(0, 100) < 50) {
        int r = random(0, 3);
        if (r == 0) playSpiderManTheme();
        else if (r == 1) playKuddaRapMelody();
        else playSoftHum();
        return;
      }
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

  if (r < 45) {
    // EMO Smooth Blink
    drawBlinkAnimation();
    renderEyes();
  } else if (r < 75) {
    // Look around & Pan
    int target = random(SERVO_MIN, SERVO_MAX);
    setPanTarget(target, 20);
    if (target < SERVO_CENTER - 15) currentLook = LOOK_LEFT;
    else if (target > SERVO_CENTER + 15) currentLook = LOOK_RIGHT;
    else currentLook = LOOK_CENTER;
    renderEyes();
  } else if (r < 90) {
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
  } else {
    // Subtle quiet chirp
    soundSoftChirp();
    setEmotion(EMO_CURIOUS, 1000);
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
    if (soundEnabled) tone(BUZZER_LEFT, NOTE_G4, 60);
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
// CUTE EMO EYE GRAPHICS
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
    display.fillCircle(x + w - 10, y + 10, 4, SSD1306_BLACK);
    display.fillCircle(x + w - 6, y + 17, 2, SSD1306_BLACK);
    display.fillCircle(x + 10, y + h - 10, 2, SSD1306_BLACK);
  }
}

void drawBlush(int x1, int y1, int x2, int y2) {
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
    case EMO_MUSIC: drawMusicEyes(); break;
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
  display.fillCircle(55, 48, 3, SSD1306_WHITE);
}

void drawMusicEyes() {
  drawEye(18, 14, 38, 38, 12, false);
  drawEye(72, 14, 38, 38, 12, false);
  display.fillCircle(30, 36, 4, SSD1306_BLACK);
  display.fillRect(32, 20, 3, 16, SSD1306_BLACK);
  display.fillRect(32, 20, 10, 4, SSD1306_BLACK);

  display.fillCircle(84, 36, 4, SSD1306_BLACK);
  display.fillRect(86, 20, 3, 16, SSD1306_BLACK);
  display.fillRect(86, 20, 10, 4, SSD1306_BLACK);
  drawBlush(15, 46, 99, 46);
}

// =============================================================================
// MELODIC SINGING ENGINE (SPIDER-MAN & KUDDA MELODIES)
// =============================================================================
void singNote(int noteFreq, int durationMs) {
  if (!soundEnabled) {
    delay(durationMs);
    return;
  }
  if (noteFreq <= 0) {
    delay(durationMs);
    return;
  }
  tone(BUZZER_LEFT, noteFreq, durationMs - 15);
  delay(durationMs);
  noTone(BUZZER_LEFT);
}

// 1. Classic Spider-Man Theme Riff (~3.5 seconds)
void playSpiderManTheme() {
  if (!soundEnabled) return;
  int notes[] = { NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4 };
  int times[] = { 180,     180,     340,     160,     160,     160,     160,     360     };

  for (int i = 0; i < 8; i++) {
    setPanTarget(SERVO_CENTER + ((i % 2 == 0) ? 14 : -14), 10);
    singNote(notes[i], times[i]);
  }
  setPanTarget(SERVO_CENTER, 15);
}

// 2. Sri Lankan Rap "Kudda" Hook Melody (~3.5 seconds)
void playKuddaRapMelody() {
  if (!soundEnabled) return;
  int notes[] = { NOTE_D4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_C4, NOTE_D4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_D4 };
  int times[] = { 130,     130,     180,     130,     130,     140,     120,     130,     130,     180,     180,     160,     280     };

  for (int i = 0; i < 13; i++) {
    setPanTarget(SERVO_CENTER + ((i % 2 == 0) ? 10 : -10), 8);
    singNote(notes[i], times[i]);
  }
  setPanTarget(SERVO_CENTER, 15);
}

void playSoftHum() {
  if (!soundEnabled) return;
  int melody[] = { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_G4, NOTE_C5 };
  int tempo[]  = { 130,     130,     130,     190,     130,     280 };

  for (int i = 0; i < 6; i++) {
    setPanTarget(SERVO_CENTER + ((i % 2 == 0) ? 8 : -8), 10);
    singNote(melody[i], tempo[i]);
  }
  setPanTarget(SERVO_CENTER, 15);
}

void playSoftLullaby() {
  if (!soundEnabled) return;
  singNote(NOTE_G4, 250);
  singNote(NOTE_E4, 250);
  singNote(NOTE_C4, 400);
}

void soundSoftChirp() {
  if (!soundEnabled) return;
  singNote(NOTE_G4, 50);
  singNote(NOTE_C5, 70);
}

void soundSoftPurr() {
  if (!soundEnabled) return;
  singNote(NOTE_C4, 80);
  singNote(NOTE_E4, 80);
  singNote(NOTE_G4, 110);
}

void soundSoftHappy() {
  if (!soundEnabled) return;
  singNote(NOTE_C5, 70);
  singNote(NOTE_E5, 70);
  singNote(NOTE_G5, 130);
}

void soundSoftLove() {
  if (!soundEnabled) return;
  singNote(NOTE_C4, 100);
  singNote(NOTE_E4, 110);
  singNote(NOTE_G4, 180);
}

void soundSoftYawn() {
  if (!soundEnabled) return;
  singNote(NOTE_G4, 160);
  singNote(NOTE_E4, 180);
  singNote(NOTE_C4, 260);
}

void soundGameStart() {
  if (!soundEnabled) return;
  singNote(NOTE_C4, 90);
  singNote(NOTE_E4, 90);
  singNote(NOTE_G4, 140);
}

void soundGameBeep() {
  if (!soundEnabled) return;
  singNote(NOTE_A4, 80);
}

void soundGameWin() {
  if (!soundEnabled) return;
  singNote(NOTE_G4, 90);
  singNote(NOTE_C5, 90);
  singNote(NOTE_E5, 180);
}

void soundGameLose() {
  if (!soundEnabled) return;
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
    soundEnabled = (EEPROM.read(EEPROM_ADDR_MUTE) != 0);
  } else {
    affectionScore = 50;
    moodScore = 80;
    totalInteractions = 0;
    soundEnabled = true;
    saveEEPROM();
  }
}

void saveEEPROM() {
  EEPROM.update(EEPROM_ADDR_MAGIC, EEPROM_MAGIC);
  EEPROM.update(EEPROM_ADDR_AFFECTION, affectionScore);
  EEPROM.update(EEPROM_ADDR_MOOD, moodScore);
  EEPROM.update(EEPROM_ADDR_INTERACTIONS_L, totalInteractions & 0xFF);
  EEPROM.update(EEPROM_ADDR_INTERACTIONS_H, (totalInteractions >> 8) & 0xFF);
  EEPROM.update(EEPROM_ADDR_MUTE, soundEnabled ? 1 : 0);
}