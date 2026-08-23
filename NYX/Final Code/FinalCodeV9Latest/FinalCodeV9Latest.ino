#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// =============================================================================
// NYX - EMO-Style Desktop Robot (Ultra-Reliable Touch & Easter Eggs Edition)
// Guaranteed Touch Detection, Direct OLED Rendering, Real-Time Serial Debug
// =============================================================================

// ================= MUSICAL NOTES =================
#define NOTE_B3  247
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

// ================= OLED CONFIGURATION =================
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
int currentPan = 80;

// ================= TOUCH SENSORS =================
#define TOUCH_HEAD 4
#define TOUCH_LEFT 11
#define TOUCH_RIGHT 5

// ================= ULTRASONIC SENSOR (SILENT) =================
#define TRIG_PIN 2
#define ECHO_PIN 3

// ================= DUAL BUZZERS =================
#define BUZZER_LEFT A0
#define BUZZER_RIGHT A1

// ================= SYSTEM VARIABLES =================
bool soundEnabled = true;
bool inGameMode = false;
uint8_t gameScore = 0;

int lastHead = LOW;
int lastLeft = LOW;
int lastRight = LOW;

unsigned long headPressStartTime = 0;
bool headHeld = false;
bool muteToggled = false;
uint8_t petStreak = 0;
unsigned long lastPetTime = 0;

// Fast Combo History Buffer
char comboHistory[4] = {0, 0, 0, 0};
unsigned long lastComboTime = 0;

// Timers & Distance
unsigned long lastActionTime = 0;
unsigned long lastIdleTime = 0;
unsigned long lastMusicTime = 0;
unsigned long lastDistanceTime = 0;
float currentDistance = -1.0;
float lastDistance = -1.0;

// Function Declarations
void smoothPan(int targetAngle, int stepDelay = 5);
void drawEye(int x, int y, int w, int h, int r = 12);
void normalEyes();
void blinkEyes();
void lookLeftEyes();
void lookRightEyes();
void happyEyes();
void excitedEyes();
void curiousEyes();
void loveEyes();
void sleepyEyes();
void angryEyes();
void scaredEyes();
void dizzyEyes();
void sadEyes();
void sunglassesEyes();
void matrixEyes();
void rageEyes();
void showMuteScreen(bool isMuted);

void singNote(int freq, int durationMs);
void playSpiderMan();
void playKuddaRap();
void playBailaDance();
void playNyanCat();
void playSoftHum();
void playSoftLullaby();
void chirp();
void purr();
void gameBeep();
void gameWinSound();
void gameLoseSound();

void triggerEasterEggBaila();
void triggerEasterEggMatrix();
void triggerEasterEggRage();
void triggerEasterEggNyan();

void toggleMuteMode();
void checkDistance();
void handleTouch();
void processSerial();
void performIdle();
void runReflexGame();
bool checkComboAndTrigger(char key);

// =============================================================================
// SMOOTH SERVO EASING CONTROLLER
// =============================================================================
void smoothPan(int targetAngle, int stepDelay) {
  targetAngle = constrain(targetAngle, 25, 135);
  while (currentPan != targetAngle) {
    if (currentPan < targetAngle) {
      currentPan++;
    } else {
      currentPan--;
    }
    panServo.write(currentPan);
    delay(stepDelay);
  }
}

// =============================================================================
// SETUP - EXACT WORKING INITIALIZATION
// =============================================================================
void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println(F("======================"));
  Serial.println(F("NYX V14 Online"));
  Serial.println(F("======================"));

  display.begin(
    SSD1306_SWITCHCAPVCC,
    0x3C
  );

  delay(100);

  normalEyes();

  Serial.println(F("OLED READY"));

  panServo.attach(PAN_PIN);
  panServo.write(80);
  currentPan = 80;
  Serial.println(F("PAN SERVO READY"));

  pinMode(TOUCH_HEAD, INPUT);
  pinMode(TOUCH_LEFT, INPUT);
  pinMode(TOUCH_RIGHT, INPUT);
  Serial.println(F("TOUCH SENSORS READY (D4, D5, D11)"));

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);
  Serial.println(F("ULTRASONIC READY (D2, D3)"));

  pinMode(BUZZER_LEFT, OUTPUT);
  pinMode(BUZZER_RIGHT, OUTPUT);
  noTone(BUZZER_LEFT);
  noTone(BUZZER_RIGHT);
  Serial.println(F("BUZZERS READY (A0, A1)"));

  // Friendly soft startup greeting
  excitedEyes();
  singNote(NOTE_C5, 80);
  singNote(NOTE_G5, 140);
  delay(150);
  normalEyes();

  lastActionTime = millis();
  lastIdleTime = millis();
  lastMusicTime = millis();

  Serial.println(F("=================================="));
  Serial.println(F("NYX Ready & Listening for Touches!"));
  Serial.println(F("=================================="));
}

// =============================================================================
// MAIN LOOP
// =============================================================================
void loop() {
  unsigned long now = millis();

  // 1. Process Desktop Serial Commands
  processSerial();

  // 2. Process Touch Sensors & Easter Egg Gestures
  handleTouch();

  // 3. Process Silent Ultrasonic Detection (every 140ms)
  if (now - lastDistanceTime >= 140) {
    lastDistanceTime = now;
    checkDistance();
  }

  // 4. Ambient Idle Behaviors & Rarer Singing
  if (!inGameMode) {
    // Rare Singing in Idle (every 90s)
    if (now - lastMusicTime >= 90000) {
      lastMusicTime = now;
      if (soundEnabled && random(0, 100) < 40) {
        int r = random(0, 3);
        if (r == 0) playSpiderMan();
        else if (r == 1) playKuddaRap();
        else playSoftHum();
        normalEyes();
      }
    }

    // Gentle Idle Look-around (every 4.5s)
    if (now - lastIdleTime >= 4500) {
      lastIdleTime = now;
      performIdle();
    }
  }

  delay(20);
}

// =============================================================================
// SERIAL CLI PROCESSOR
// =============================================================================
void processSerial() {
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    lastActionTime = millis();

    switch (cmd) {
      case 'm': case 'M': toggleMuteMode(); break;
      case 'g': case 'G': runReflexGame(); break;
      case 's': case 'S': playSpiderMan(); normalEyes(); break;
      case 'k': case 'K': playKuddaRap(); normalEyes(); break;
      case 'b': case 'B': triggerEasterEggBaila(); normalEyes(); break;
      case 'x': case 'X': triggerEasterEggMatrix(); normalEyes(); break;
      case 'r': case 'R': triggerEasterEggRage(); normalEyes(); break;
      case 'n': case 'N': triggerEasterEggNyan(); normalEyes(); break;
      case 'h': case 'H': happyEyes(); singNote(NOTE_C5, 80); delay(500); normalEyes(); break;
      case 'z': case 'Z': sleepyEyes(); playSoftLullaby(); smoothPan(80, 12); break;
      case 'w': case 'W': excitedEyes(); singNote(NOTE_G5, 120); delay(300); normalEyes(); break;
    }
  }
}

// =============================================================================
// FAST TOUCH COMBO DETECTOR (NON-BLOCKING)
// =============================================================================
bool checkComboAndTrigger(char key) {
  unsigned long now = millis();

  // Reset combo if idle for more than 2.5s
  if (now - lastComboTime > 2500) {
    comboHistory[0] = 0;
    comboHistory[1] = 0;
    comboHistory[2] = 0;
    comboHistory[3] = 0;
  }
  lastComboTime = now;

  comboHistory[0] = comboHistory[1];
  comboHistory[1] = comboHistory[2];
  comboHistory[2] = comboHistory[3];
  comboHistory[3] = key;

  // 1. Matrix: 3 Head Taps ('H', 'H', 'H')
  if (comboHistory[1] == 'H' && comboHistory[2] == 'H' && comboHistory[3] == 'H') {
    memset(comboHistory, 0, sizeof(comboHistory));
    Serial.println(F("--> TRIGGERED: MATRIX DIGITAL RAIN"));
    triggerEasterEggMatrix();
    normalEyes();
    return true;
  }

  // 2. Baila Dance: ('L', 'L', 'R', 'R')
  if (comboHistory[0] == 'L' && comboHistory[1] == 'L' && comboHistory[2] == 'R' && comboHistory[3] == 'R') {
    memset(comboHistory, 0, sizeof(comboHistory));
    Serial.println(F("--> TRIGGERED: SRI LANKAN BAILA DANCE"));
    triggerEasterEggBaila();
    normalEyes();
    return true;
  }

  // 3. Nyan Cat: 3 Right Taps ('R', 'R', 'R')
  if (comboHistory[1] == 'R' && comboHistory[2] == 'R' && comboHistory[3] == 'R') {
    memset(comboHistory, 0, sizeof(comboHistory));
    Serial.println(F("--> TRIGGERED: NYAN CAT MELODY"));
    triggerEasterEggNyan();
    normalEyes();
    return true;
  }

  // 4. Spider-Man Theme: ('L', 'R', 'L')
  if (comboHistory[1] == 'L' && comboHistory[2] == 'R' && comboHistory[3] == 'L') {
    memset(comboHistory, 0, sizeof(comboHistory));
    Serial.println(F("--> TRIGGERED: SPIDER-MAN THEME"));
    playSpiderMan();
    normalEyes();
    return true;
  }

  return false;
}

// =============================================================================
// TOUCH SENSORS & GESTURE PROCESSING
// =============================================================================
void handleTouch() {
  unsigned long now = millis();
  int head = digitalRead(TOUCH_HEAD);
  int left = digitalRead(TOUCH_LEFT);
  int right = digitalRead(TOUCH_RIGHT);

  // 1. TRIPLE TOUCH (Head + Left + Right together) -> Kudda Sinhala Rap
  if (head == HIGH && left == HIGH && right == HIGH) {
    if (lastHead == LOW || lastLeft == LOW || lastRight == LOW) {
      Serial.println(F("--> TRIGGERED: TRIPLE TOUCH (KUDDA RAP)"));
      lastActionTime = now;
      memset(comboHistory, 0, sizeof(comboHistory));
      playKuddaRap();
      normalEyes();
      lastHead = head;
      lastLeft = left;
      lastRight = right;
      return;
    }
  }

  // 2. DUAL TOUCH (Left + Right together -> Game Mode)
  if (left == HIGH && right == HIGH && head == LOW) {
    if (lastLeft == LOW || lastRight == LOW) {
      Serial.println(F("--> TRIGGERED: LEFT+RIGHT (GAME MODE)"));
      lastActionTime = now;
      memset(comboHistory, 0, sizeof(comboHistory));
      runReflexGame();
      lastHead = head;
      lastLeft = left;
      lastRight = right;
      return;
    }
  }

  // 3. HEAD TOUCH
  if (head == HIGH) {
    lastActionTime = now;
    if (!headHeld) {
      headHeld = true;
      headPressStartTime = now;
      muteToggled = false;
      Serial.println(F("HEAD TOUCH DETECTED"));
    } else {
      // Laser Rage: Hold Head + Ultrasonic < 8cm
      if (!muteToggled && currentDistance > 0 && currentDistance < 8.0 && (now - headPressStartTime > 1200)) {
        muteToggled = true;
        Serial.println(F("--> TRIGGERED: LASER RAGE EASTER EGG"));
        triggerEasterEggRage();
        normalEyes();
      }
      // Hold Head for 2.2s -> Toggle Mute
      else if (!muteToggled && (now - headPressStartTime > 2200)) {
        muteToggled = true;
        toggleMuteMode();
      }
      // Sustained Petting (>500ms)
      else if (!muteToggled && (now - headPressStartTime > 500)) {
        if (now - lastPetTime > 300) {
          lastPetTime = now;
          petStreak++;
          if (petStreak >= 3) {
            loveEyes();
            singNote(NOTE_C4, 80);
            singNote(NOTE_G4, 140);
          } else {
            happyEyes();
            purr();
          }
        }
      }
    }
  } else {
    // Head Released
    if (headHeld) {
      headHeld = false;
      unsigned long duration = now - headPressStartTime;
      if (!muteToggled && duration < 500) {
        if (!checkComboAndTrigger('H')) {
          happyEyes();
          chirp();
          delay(120);
          normalEyes();
        }
      } else if (duration >= 500 && !muteToggled) {
        normalEyes();
      }
    }
  }

  if (now - lastPetTime > 3000) petStreak = 0;

  // 4. LEFT CHEEK TOUCH (Rising edge)
  if (left == HIGH && lastLeft == LOW && head == LOW && right == LOW) {
    lastActionTime = now;
    Serial.println(F("LEFT CHEEK TOUCH DETECTED"));
    if (!checkComboAndTrigger('L')) {
      lookLeftEyes();
      smoothPan(35, 3);
      chirp();
      delay(120);
      smoothPan(80, 4);
      normalEyes();
    }
  }

  // 5. RIGHT CHEEK TOUCH (Rising edge)
  if (right == HIGH && lastRight == LOW && head == LOW && left == LOW) {
    lastActionTime = now;
    Serial.println(F("RIGHT CHEEK TOUCH DETECTED"));
    if (!checkComboAndTrigger('R')) {
      lookRightEyes();
      smoothPan(125, 3);
      chirp();
      delay(120);
      smoothPan(80, 4);
      normalEyes();
    }
  }

  lastHead = head;
  lastLeft = left;
  lastRight = right;
}

// =============================================================================
// ULTRASONIC SENSOR (100% SILENT - DETECTION ONLY)
// =============================================================================
void checkDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 20000);

  if (duration > 0) {
    lastDistance = currentDistance;
    currentDistance = duration * 0.0343 / 2.0;

    // Sudden intrusion (< 10cm): Scared expression and pan away (SILENT)
    if (lastDistance > 25.0 && currentDistance < 10.0) {
      lastActionTime = millis();
      scaredEyes();
      smoothPan(130, 4);
      delay(250);
      smoothPan(80, 6);
      normalEyes();
      return;
    }

    // Friendly approach (12 - 30cm): Curious glance (SILENT)
    if (currentDistance > 12.0 && currentDistance < 30.0) {
      curiousEyes();
      delay(250);
      normalEyes();
    }
  } else {
    currentDistance = -1.0;
  }
}

// =============================================================================
// AMBIENT IDLE BEHAVIOR
// =============================================================================
void performIdle() {
  int r = random(0, 100);

  if (r < 50) {
    blinkEyes();
    normalEyes();
  } else if (r < 75) {
    int angle = (random(0, 2) == 0) ? 40 : 120;
    if (angle < 80) lookLeftEyes();
    else lookRightEyes();
    smoothPan(angle, 12);
    delay(300);
    smoothPan(80, 10);
    normalEyes();
  } else if (r < 90) {
    happyEyes();
    singNote(NOTE_C5, 70);
    delay(300);
    normalEyes();
  } else {
    chirp();
    curiousEyes();
    delay(250);
    normalEyes();
  }
}

// =============================================================================
// MUTE & SOUND TOGGLE
// =============================================================================
void toggleMuteMode() {
  soundEnabled = !soundEnabled;
  showMuteScreen(!soundEnabled);
  Serial.print(F("SOUND MODE: "));
  Serial.println(soundEnabled ? F("ENABLED (ON)") : F("MUTED (OFF)"));
  if (soundEnabled) {
    singNote(NOTE_E5, 90);
    singNote(NOTE_A5, 130);
  }
}

void showMuteScreen(bool isMuted) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(18, 15);
  display.print(isMuted ? F("MUTED") : F("SOUND ON"));

  display.fillRoundRect(50, 40, 10, 14, 2, SSD1306_WHITE);
  display.fillTriangle(60, 40, 72, 32, 72, 60, SSD1306_WHITE);
  if (isMuted) {
    display.drawLine(46, 32, 78, 62, SSD1306_WHITE);
    display.drawLine(46, 33, 78, 63, SSD1306_WHITE);
  } else {
    display.drawCircle(78, 47, 5, SSD1306_WHITE);
  }
  display.display();
  delay(800);
  normalEyes();
}

// =============================================================================
// MINI-GAME: REFLEX DUEL
// =============================================================================
void runReflexGame() {
  inGameMode = true;
  gameScore = 0;
  unsigned long timeLimit = 1400;

  for (int i = 3; i >= 1; i--) {
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(55, 20);
    display.print(i);
    display.display();
    gameBeep();
    delay(500);
  }

  for (int round = 0; round < 5; round++) {
    int target = random(0, 2);

    display.clearDisplay();
    if (target == 0) {
      smoothPan(30, 4);
      drawEye(8, 12, 38, 40, 12);
      drawEye(68, 12, 38, 40, 12);
      display.fillTriangle(6, 32, 18, 20, 18, 44, SSD1306_WHITE);
    } else {
      smoothPan(130, 4);
      drawEye(22, 12, 38, 40, 12);
      drawEye(82, 12, 38, 40, 12);
      display.fillTriangle(122, 32, 110, 20, 110, 44, SSD1306_WHITE);
    }
    display.display();
    if (soundEnabled) tone(BUZZER_LEFT, NOTE_G4, 60);

    unsigned long startWait = millis();
    bool hit = false;

    while (millis() - startWait < timeLimit) {
      int l = digitalRead(TOUCH_LEFT);
      int r = digitalRead(TOUCH_RIGHT);

      if ((target == 0 && l == HIGH) || (target == 1 && r == HIGH)) {
        hit = true;
        gameScore++;
        happyEyes();
        gameWinSound();
        delay(250);
        smoothPan(80, 6);
        if (timeLimit > 450) timeLimit -= 100;
        break;
      } else if ((target == 0 && r == HIGH) || (target == 1 && l == HIGH)) {
        hit = false;
        break;
      }
      delay(10);
    }

    if (!hit) {
      dizzyEyes();
      gameLoseSound();
      delay(1000);
      break;
    }
    delay(250);
  }

  inGameMode = false;
  smoothPan(80, 6);
  normalEyes();
}

// =============================================================================
// PURE SOLID EMO-STYLE EYE GRAPHICS (NO DOTS)
// =============================================================================
void drawEye(int x, int y, int w, int h, int r) {
  display.fillRoundRect(x, y, w, h, r, SSD1306_WHITE);
}

void normalEyes() {
  display.clearDisplay();
  drawEye(15, 12, 38, 40, 12);
  drawEye(75, 12, 38, 40, 12);
  display.display();
}

void blinkEyes() {
  display.clearDisplay();
  display.fillRoundRect(15, 22, 38, 20, 8, SSD1306_WHITE);
  display.fillRoundRect(75, 22, 38, 20, 8, SSD1306_WHITE);
  display.display();
  delay(30);

  display.clearDisplay();
  display.fillRoundRect(15, 30, 38, 8, 4, SSD1306_WHITE);
  display.fillRoundRect(75, 30, 38, 8, 4, SSD1306_WHITE);
  display.display();
  delay(50);
}

void lookLeftEyes() {
  display.clearDisplay();
  drawEye(8, 12, 38, 40, 12);
  drawEye(68, 12, 38, 40, 12);
  display.display();
}

void lookRightEyes() {
  display.clearDisplay();
  drawEye(22, 12, 38, 40, 12);
  drawEye(82, 12, 38, 40, 12);
  display.display();
}

void happyEyes() {
  display.clearDisplay();
  for (int i = 0; i < 5; i++) {
    display.drawLine(15, 38 - i, 25, 24 - i, SSD1306_WHITE);
    display.drawLine(25, 24 - i, 45, 38 - i, SSD1306_WHITE);
    display.drawLine(75, 38 - i, 85, 24 - i, SSD1306_WHITE);
    display.drawLine(85, 24 - i, 105, 38 - i, SSD1306_WHITE);
  }
  display.display();
}

void excitedEyes() {
  display.clearDisplay();
  drawEye(7, 7, 45, 48, 14);
  drawEye(76, 7, 45, 48, 14);
  display.display();
}

void curiousEyes() {
  display.clearDisplay();
  drawEye(8, 8, 44, 46, 14);
  drawEye(76, 15, 40, 40, 10);
  display.display();
}

void sleepyEyes() {
  display.clearDisplay();
  display.fillRoundRect(15, 26, 38, 24, 8, SSD1306_WHITE);
  display.fillRoundRect(75, 26, 38, 24, 8, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(112, 10);
  display.print(F("zZ"));
  display.display();
}

void loveEyes() {
  display.clearDisplay();
  display.fillCircle(25, 24, 11, SSD1306_WHITE);
  display.fillCircle(41, 24, 11, SSD1306_WHITE);
  display.fillTriangle(15, 27, 51, 27, 33, 47, SSD1306_WHITE);

  display.fillCircle(85, 24, 11, SSD1306_WHITE);
  display.fillCircle(101, 24, 11, SSD1306_WHITE);
  display.fillTriangle(75, 27, 111, 27, 93, 47, SSD1306_WHITE);
  display.display();
}

void angryEyes() {
  display.clearDisplay();
  drawEye(15, 16, 38, 36, 10);
  drawEye(75, 16, 38, 36, 10);
  display.fillTriangle(10, 12, 55, 12, 55, 26, SSD1306_BLACK);
  display.fillTriangle(73, 12, 118, 12, 73, 26, SSD1306_BLACK);
  display.display();
}

void scaredEyes() {
  display.clearDisplay();
  display.fillRoundRect(25, 22, 22, 22, 6, SSD1306_WHITE);
  display.fillRoundRect(81, 22, 22, 22, 6, SSD1306_WHITE);
  display.display();
}

void dizzyEyes() {
  display.clearDisplay();
  display.drawLine(15, 16, 45, 46, SSD1306_WHITE);
  display.drawLine(45, 16, 15, 46, SSD1306_WHITE);
  display.drawLine(75, 16, 105, 46, SSD1306_WHITE);
  display.drawLine(105, 16, 75, 46, SSD1306_WHITE);
  display.display();
}

void sadEyes() {
  display.clearDisplay();
  for (int i = 0; i < 4; i++) {
    display.drawLine(15, 20 + i, 34, 38 + i, SSD1306_WHITE);
    display.drawLine(34, 38 + i, 53, 20 + i, SSD1306_WHITE);
    display.drawLine(75, 20 + i, 94, 38 + i, SSD1306_WHITE);
    display.drawLine(94, 38 + i, 113, 20 + i, SSD1306_WHITE);
  }
  display.display();
}

void sunglassesEyes() {
  display.clearDisplay();
  display.fillRoundRect(14, 20, 44, 24, 6, SSD1306_WHITE);
  display.fillRoundRect(70, 20, 44, 24, 6, SSD1306_WHITE);
  display.fillRect(56, 24, 16, 6, SSD1306_WHITE);
  display.drawLine(18, 22, 26, 42, SSD1306_BLACK);
  display.drawLine(74, 22, 82, 42, SSD1306_BLACK);
  display.display();
}

void matrixEyes() {
  display.clearDisplay();
  for (int x = 8; x < 120; x += 12) {
    int h1 = (x * 7) % 50;
    display.drawLine(x, 0, x, h1, SSD1306_WHITE);
    display.drawPixel(x + 2, (h1 + 8) % 64, SSD1306_WHITE);
  }
  display.display();
}

void rageEyes() {
  display.clearDisplay();
  display.fillRect(10, 26, 48, 12, SSD1306_WHITE);
  display.fillRect(70, 26, 48, 12, SSD1306_WHITE);
  display.display();
}

// =============================================================================
// PLEASANT AUDIO ENGINE
// =============================================================================
void singNote(int freq, int durationMs) {
  if (!soundEnabled || freq <= 0) {
    delay(durationMs);
    return;
  }
  tone(BUZZER_LEFT, freq, durationMs - 15);
  delay(durationMs);
  noTone(BUZZER_LEFT);
}

void playSpiderMan() {
  excitedEyes();
  int notes[] = { NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, NOTE_B4, NOTE_A4, NOTE_G4, NOTE_E4 };
  int times[] = { 180,     180,     340,     160,     160,     160,     160,     360     };

  for (int i = 0; i < 8; i++) {
    if (i == 0) smoothPan(65, 5);
    else if (i == 4) smoothPan(95, 5);
    singNote(notes[i], times[i]);
  }
  smoothPan(80, 6);
}

void playKuddaRap() {
  sunglassesEyes();
  int notes[] = { NOTE_D4, NOTE_F4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_D4, NOTE_C4, NOTE_D4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_D4 };
  int times[] = { 130,     130,     180,     130,     130,     140,     120,     130,     130,     180,     180,     160,     280     };

  for (int i = 0; i < 13; i++) {
    if (i == 0) smoothPan(68, 5);
    else if (i == 4) smoothPan(92, 5);
    else if (i == 8) smoothPan(72, 5);
    else if (i == 11) smoothPan(88, 5);
    singNote(notes[i], times[i]);
  }
  smoothPan(80, 6);
}

// =============================================================================
// EASTER EGGS
// =============================================================================
void triggerEasterEggBaila() {
  sunglassesEyes();
  int notes[] = { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_G4, NOTE_A4, NOTE_G4, NOTE_E4, NOTE_C4, NOTE_D4, NOTE_E4, NOTE_D4, NOTE_C4 };
  int times[] = { 140,     140,     180,     140,     140,     140,     140,     140,     140,     140,     140,     300     };

  for (int i = 0; i < 12; i++) {
    if (i == 0) smoothPan(65, 5);
    else if (i == 4) smoothPan(95, 5);
    else if (i == 8) smoothPan(70, 5);
    singNote(notes[i], times[i]);
  }
  smoothPan(80, 6);
  happyEyes();
  delay(400);
}

void triggerEasterEggMatrix() {
  for (int i = 0; i < 8; i++) {
    matrixEyes();
    singNote(NOTE_C5 + (i * 30), 70);
    delay(30);
  }
  singNote(NOTE_C4, 250);
  curiousEyes();
  delay(400);
}

void triggerEasterEggRage() {
  rageEyes();
  for (int f = NOTE_C4; f <= NOTE_A5; f += 40) {
    singNote(f, 35);
    smoothPan(random(0, 2) == 0 ? 76 : 84, 3);
  }
  smoothPan(80, 6);
  excitedEyes();
  delay(600);
}

void triggerEasterEggNyan() {
  excitedEyes();
  int melody[] = { NOTE_FS4, NOTE_GS4, NOTE_D4, NOTE_DS4, NOTE_B3, NOTE_D4, NOTE_CS4, NOTE_B3, NOTE_B3, NOTE_CS4, NOTE_D4, NOTE_D4 };
  int tempo[]  = { 120,      120,      120,     120,      120,     120,     120,      120,     120,     120,      120,     240     };

  for (int i = 0; i < 12; i++) {
    if (i == 0) smoothPan(70, 4);
    else if (i == 6) smoothPan(90, 4);
    singNote(melody[i], tempo[i]);
  }
  smoothPan(80, 6);
  happyEyes();
  delay(400);
}

void playSoftHum() {
  if (!soundEnabled) return;
  int melody[] = { NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5, NOTE_G4, NOTE_C5 };
  int tempo[]  = { 130,     130,     130,     190,     130,     280 };

  for (int i = 0; i < 6; i++) {
    if (i == 0) smoothPan(72, 6);
    else if (i == 3) smoothPan(88, 6);
    singNote(melody[i], tempo[i]);
  }
  smoothPan(80, 6);
}

void playSoftLullaby() {
  if (!soundEnabled) return;
  singNote(NOTE_G4, 250);
  singNote(NOTE_E4, 250);
  singNote(NOTE_C4, 400);
}

void chirp() {
  singNote(NOTE_G4, 45);
  singNote(NOTE_C5, 65);
}

void purr() {
  singNote(NOTE_C4, 80);
  singNote(NOTE_E4, 80);
  singNote(NOTE_G4, 110);
}

void gameBeep() {
  singNote(NOTE_A4, 80);
}

void gameWinSound() {
  singNote(NOTE_G4, 90);
  singNote(NOTE_C5, 90);
  singNote(NOTE_E5, 180);
}

void gameLoseSound() {
  singNote(NOTE_E4, 140);
  singNote(NOTE_C4, 220);
}