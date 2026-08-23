#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <EEPROM.h>

// =====================================================
// NYX V6
// Arduino Nano Desktop Pet
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

// ================= PINS =================

#define PAN_PIN 9

#define TOUCH_HEAD 4
#define TOUCH_LEFT 11
#define TOUCH_RIGHT 5

#define TRIG_PIN 2
#define ECHO_PIN 3

#define BUZZER_LEFT A0
#define BUZZER_RIGHT A1

// ================= EEPROM =================

#define EEPROM_MOOD 0
#define EEPROM_AFFECTION 1
#define EEPROM_INTERACTIONS 4

// =====================================================
// EMOTIONS
// =====================================================

enum Emotion {
  HAPPY,
  SAD,
  SLEEPY,
  EXCITED,
  CURIOUS,
  BORED,
  SCARED,
  LOVE,
  ANGRY
};

Emotion currentEmotion = HAPPY;

// =====================================================
// STATES
// =====================================================

enum NYXState {
  IDLE,
  SLEEPING,
  INTERACTING,
  GAME_MODE
};

NYXState currentState = IDLE;

// =====================================================
// MODES
// =====================================================

enum GameType {
  NO_GAME,
  REACTION_GAME,
  SIMON_GAME,
  RHYTHM_GAME,
  HOT_COLD_GAME,
  REFLEX_DUEL
};

GameType currentGame = NO_GAME;

// =====================================================
// SOUND TYPES
// =====================================================

enum SoundType {
  SOUND_NONE,
  SOUND_HAPPY,
  SOUND_SLEEPY,
  SOUND_EXCITED,
  SOUND_CURIOUS,
  SOUND_SCARED,
  SOUND_LOVE,
  SOUND_ANGRY,
  SOUND_SUCCESS,
  SOUND_FAIL,
  SOUND_COUNTDOWN,
  SOUND_SPECIAL
};

// =====================================================
// CORE VARIABLES
// =====================================================

Servo panServo;

int mood = 70;
int affection = 30;

unsigned long interactionCount = 0;

int panPosition = 80;

// =====================================================
// TOUCH VARIABLES
// =====================================================

int lastHead = LOW;
int lastLeft = LOW;
int lastRight = LOW;

bool modeSwitchLocked = false;

unsigned long headTouchStart = 0;

int pettingStreak = 0;
unsigned long lastPetTime = 0;

int touchSequence[3] = {0, 0, 0};
int sequenceIndex = 0;

unsigned long lastSequenceTime = 0;

// =====================================================
// TIMERS
// =====================================================

unsigned long lastInteraction = 0;
unsigned long lastMoodUpdate = 0;
unsigned long lastBlink = 0;
unsigned long nextBlink = 3000;
unsigned long blinkStart = 0;
unsigned long lastIdleMove = 0;
unsigned long nextIdleMove = 5000;
unsigned long lastDistanceCheck = 0;
unsigned long lastEEPROMWrite = 0;

bool blinking = false;

// =====================================================
// DISTANCE
// =====================================================

float currentDistance = 999;
float lastDistance = 999;

bool greeted = false;

// =====================================================
// SOUND ENGINE
// =====================================================

SoundType currentSound = SOUND_NONE;

bool soundPlaying = false;

int soundStep = 0;

unsigned long lastSoundStep = 0;

// =====================================================
// GAME VARIABLES
// =====================================================

bool gameStarted = false;
bool gameWaiting = false;

unsigned long gameTimer = 0;

int reactionTarget = 0;

int simonSequence[8];
int simonLength = 0;
int simonInputIndex = 0;

float hotColdTarget = 30;

int rhythmScore = 0;

// =====================================================
// FUNCTION DECLARATIONS
// =====================================================

void normalEyes();
void happyEyes();
void sadEyes();
void sleepyEyes();
void excitedEyes();
void curiousEyes();
void boredEyes();
void scaredEyes();
void loveEyes();
void angryEyes();
void blinkEyes();
void lookLeft();
void lookRight();

void setEmotion(Emotion emotion);
void updateMood(unsigned long now);
void updateTouch(unsigned long now);
void updateDistance(unsigned long now);
void updateBlink(unsigned long now);
void updateIdle(unsigned long now);
void updateSleep(unsigned long now);
void updateSound(unsigned long now);
void updateGame(unsigned long now);

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(9600);

  display.begin(
    SSD1306_SWITCHCAPVCC,
    0x3C
  );

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

  pinMode(
    TRIG_PIN,
    OUTPUT
  );

  pinMode(
    ECHO_PIN,
    INPUT
  );

  pinMode(
    BUZZER_LEFT,
    OUTPUT
  );

  pinMode(
    BUZZER_RIGHT,
    OUTPUT
  );

  panServo.attach(
    PAN_PIN
  );

  panServo.write(80);

  loadMemory();

  mood = constrain(
    mood,
    0,
    100
  );

  affection = constrain(
    affection,
    0,
    100
  );

  randomSeed(
    analogRead(A2)
  );

  normalEyes();

  lastInteraction = millis();

  Serial.println("NYX V6 READY");

  startSound(
    SOUND_HAPPY
  );
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  unsigned long now = millis();

  updateTouch(now);

  updateMood(now);

  updateDistance(now);

  updateSleep(now);

  updateBlink(now);

  updateIdle(now);

  updateSound(now);

  updateGame(now);

  saveMemory(now);
}

// =====================================================
// EEPROM
// =====================================================

void loadMemory() {

  byte savedMood =
    EEPROM.read(
      EEPROM_MOOD
    );

  byte savedAffection =
    EEPROM.read(
      EEPROM_AFFECTION
    );

  EEPROM.get(
    EEPROM_INTERACTIONS,
    interactionCount
  );

  if (
    savedMood <= 100
  ) {
    mood = savedMood;
  }

  if (
    savedAffection <= 100
  ) {
    affection = savedAffection;
  }
}

void saveMemory(
  unsigned long now
) {

  if (
    now - lastEEPROMWrite < 30000
  ) {
    return;
  }

  EEPROM.update(
    EEPROM_MOOD,
    mood
  );

  EEPROM.update(
    EEPROM_AFFECTION,
    affection
  );

  EEPROM.put(
    EEPROM_INTERACTIONS,
    interactionCount
  );

  lastEEPROMWrite = now;
}

// =====================================================
// MOOD ENGINE
// =====================================================

void updateMood(
  unsigned long now
) {

  if (
    now - lastMoodUpdate < 60000
  ) {
    return;
  }

  lastMoodUpdate = now;

  if (mood > 65) {
    mood--;
  }

  if (mood < 55) {
    mood++;
  }

  if (
    now - lastInteraction > 300000
  ) {
    mood -= 2;
  }

  mood = constrain(
    mood,
    0,
    100
  );

  if (
    currentState == IDLE
  ) {
    updateEmotionFromMood();
  }
}

void updateEmotionFromMood() {

  if (mood >= 85) {

    setEmotion(EXCITED);

  } else if (mood >= 70) {

    setEmotion(HAPPY);

  } else if (mood >= 50) {

    setEmotion(CURIOUS);

  } else if (mood >= 30) {

    setEmotion(BORED);

  } else {

    setEmotion(SAD);
  }
}

// =====================================================
// EMOTION
// =====================================================

void setEmotion(
  Emotion emotion
) {

  currentEmotion = emotion;

  switch (emotion) {

    case HAPPY:
      happyEyes();
      break;

    case SAD:
      sadEyes();
      break;

    case SLEEPY:
      sleepyEyes();
      break;

    case EXCITED:
      excitedEyes();
      break;

    case CURIOUS:
      curiousEyes();
      break;

    case BORED:
      boredEyes();
      break;

    case SCARED:
      scaredEyes();
      break;

    case LOVE:
      loveEyes();
      break;

    case ANGRY:
      angryEyes();
      break;
  }
}

// =====================================================
// TOUCH SYSTEM
// =====================================================

void updateTouch(
  unsigned long now
) {

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

  // BOTH LEFT AND RIGHT
  // MODE SWITCH

  if (
    left == HIGH &&
    right == HIGH
  ) {

    if (
      !modeSwitchLocked
    ) {

      modeSwitchLocked = true;

      changeMode();

      lastInteraction = now;
    }

    lastHead = head;
    lastLeft = left;
    lastRight = right;

    return;
  }

  if (
    left == LOW &&
    right == LOW
  ) {

    modeSwitchLocked = false;
  }

  // ALL THREE SPECIAL

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

      specialReaction();

      lastHead = head;
      lastLeft = left;
      lastRight = right;

      return;
    }
  }

  // HEAD PRESS

  if (
    head == HIGH &&
    lastHead == LOW
  ) {

    headTouchStart = now;

    interactionCount++;

    lastInteraction = now;

    wakeIfSleeping();

    handleGameInput(0);
  }

  // HEAD RELEASE

  if (
    head == LOW &&
    lastHead == HIGH
  ) {

    unsigned long heldTime =
      now - headTouchStart;

    if (
      currentGame == NO_GAME
    ) {

      if (
        heldTime >= 800
      ) {

        petReaction();

      } else {

        headReaction();
      }
    }
  }

  // LEFT

  if (
    left == HIGH &&
    lastLeft == LOW
  ) {

    interactionCount++;

    lastInteraction = now;

    wakeIfSleeping();

    handleGameInput(1);

    if (
      currentGame == NO_GAME
    ) {

      leftReaction();
    }
  }

  // RIGHT

  if (
    right == HIGH &&
    lastRight == LOW
  ) {

    interactionCount++;

    lastInteraction = now;

    wakeIfSleeping();

    handleGameInput(2);

    if (
      currentGame == NO_GAME
    ) {

      rightReaction();
    }
  }

  lastHead = head;
  lastLeft = left;
  lastRight = right;
}

// =====================================================
// TOUCH REACTIONS
// =====================================================

void headReaction() {

  mood += 5;

  affection += 2;

  mood = constrain(
    mood,
    0,
    100
  );

  affection = constrain(
    affection,
    0,
    100
  );

  setEmotion(HAPPY);

  movePan(80);

  startSound(
    SOUND_HAPPY
  );
}

void petReaction() {

  unsigned long now = millis();

  if (
    now - lastPetTime < 5000
  ) {

    pettingStreak++;

  } else {

    pettingStreak = 1;
  }

  lastPetTime = now;

  mood += 8;

  affection += 5;

  if (
    pettingStreak >= 3
  ) {

    mood += 10;

    affection += 10;

    setEmotion(LOVE);

    startSound(
      SOUND_LOVE
    );

  } else {

    setEmotion(HAPPY);

    startSound(
      SOUND_HAPPY
    );
  }

  mood = constrain(
    mood,
    0,
    100
  );

  affection = constrain(
    affection,
    0,
    100
  );
}

void leftReaction() {

  lookLeft();

  movePan(35);

  mood += 2;

  mood = constrain(
    mood,
    0,
    100
  );

  addTouchToSequence(1);

  startSound(
    SOUND_CURIOUS
  );
}

void rightReaction() {

  lookRight();

  movePan(125);

  mood += 2;

  mood = constrain(
    mood,
    0,
    100
  );

  addTouchToSequence(2);

  startSound(
    SOUND_CURIOUS
  );
}

// =====================================================
// TICKLE COMBO
// LEFT RIGHT LEFT
// =====================================================

void addTouchToSequence(
  int touch
) {

  unsigned long now = millis();

  if (
    now - lastSequenceTime > 1200
  ) {

    sequenceIndex = 0;
  }

  lastSequenceTime = now;

  if (
    sequenceIndex < 3
  ) {

    touchSequence[sequenceIndex] =
      touch;

    sequenceIndex++;
  }

  if (
    sequenceIndex >= 3
  ) {

    if (
      touchSequence[0] == 1 &&
      touchSequence[1] == 2 &&
      touchSequence[2] == 1
    ) {

      tickleReaction();
    }

    sequenceIndex = 0;
  }
}

void tickleReaction() {

  mood += 10;

  affection += 3;

  mood = constrain(
    mood,
    0,
    100
  );

  affection = constrain(
    affection,
    0,
    100
  );

  setEmotion(EXCITED);

  startSound(
    SOUND_EXCITED
  );
}

void specialReaction() {

  mood += 15;

  affection += 10;

  mood = constrain(
    mood,
    0,
    100
  );

  affection = constrain(
    affection,
    0,
    100
  );

  setEmotion(EXCITED);

  startSound(
    SOUND_SPECIAL
  );
}

// =====================================================
// MODE SWITCH
// =====================================================

void changeMode() {

  stopGame();

  switch (currentGame) {

    case NO_GAME:
      currentGame = REACTION_GAME;
      break;

    case REACTION_GAME:
      currentGame = SIMON_GAME;
      break;

    case SIMON_GAME:
      currentGame = RHYTHM_GAME;
      break;

    case RHYTHM_GAME:
      currentGame = HOT_COLD_GAME;
      break;

    case HOT_COLD_GAME:
      currentGame = REFLEX_DUEL;
      break;

    default:
      currentGame = NO_GAME;
      break;
  }

  if (
    currentGame == NO_GAME
  ) {

    currentState = IDLE;

  } else {

    currentState = GAME_MODE;
  }

  showMode();

  startSound(
    SOUND_COUNTDOWN
  );

  Serial.print("MODE: ");
  Serial.println(currentGame);
}

void showMode() {

  switch (currentGame) {

    case NO_GAME:
      normalEyes();
      break;

    case REACTION_GAME:
      curiousEyes();
      break;

    case SIMON_GAME:
      excitedEyes();
      break;

    case RHYTHM_GAME:
      happyEyes();
      break;

    case HOT_COLD_GAME:
      scaredEyes();
      break;

    case REFLEX_DUEL:
      angryEyes();
      break;
  }
}

// =====================================================
// SLEEP
// =====================================================

void updateSleep(
  unsigned long now
) {

  if (
    currentState == GAME_MODE ||
    currentState == SLEEPING
  ) {
    return;
  }

  if (
    now - lastInteraction > 180000
  ) {

    currentState = SLEEPING;

    setEmotion(SLEEPY);

    startSound(
      SOUND_SLEEPY
    );
  }
}

void wakeIfSleeping() {

  if (
    currentState != SLEEPING
  ) {
    return;
  }

  currentState = IDLE;

  mood += 5;

  mood = constrain(
    mood,
    0,
    100
  );

  setEmotion(HAPPY);

  startSound(
    SOUND_HAPPY
  );

  lastInteraction = millis();
}

// =====================================================
// ULTRASONIC
// =====================================================

void updateDistance(
  unsigned long now
) {

  if (
    now - lastDistanceCheck < 250
  ) {
    return;
  }

  lastDistanceCheck = now;

  float distance =
    readDistance();

  if (
    distance <= 0 ||
    distance > 200
  ) {
    return;
  }

  currentDistance = distance;

  if (
    lastDistance > 30 &&
    distance < 12 &&
    currentState != GAME_MODE
  ) {

    scaredReaction();
  }

  if (
    distance >= 15 &&
    distance <= 45 &&
    !greeted &&
    currentState == IDLE
  ) {

    setEmotion(CURIOUS);

    startSound(
      SOUND_CURIOUS
    );

    greeted = true;

    lastInteraction = now;
  }

  if (
    distance > 70
  ) {

    greeted = false;
  }

  lastDistance = distance;
}

float readDistance() {

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
      25000
    );

  if (
    duration == 0
  ) {

    return 0;
  }

  return
    duration * 0.0343 / 2.0;
}

void scaredReaction() {

  mood -= 5;

  mood = constrain(
    mood,
    0,
    100
  );

  setEmotion(SCARED);

  movePan(
    random(
      45,
      120
    )
  );

  startSound(
    SOUND_SCARED
  );
}

// =====================================================
// BLINK
// =====================================================

void updateBlink(
  unsigned long now
) {

  if (
    currentState == SLEEPING
  ) {
    return;
  }

  if (
    !blinking &&
    now - lastBlink > nextBlink
  ) {

    blinkEyes();

    blinking = true;

    blinkStart = now;

    lastBlink = now;
  }

  if (
    blinking &&
    now - blinkStart > 120
  ) {

    blinking = false;

    setEmotion(
      currentEmotion
    );

    nextBlink =
      random(
        2000,
        7000
      );
  }
}

// =====================================================
// IDLE
// =====================================================

void updateIdle(
  unsigned long now
) {

  if (
    currentState != IDLE
  ) {
    return;
  }

  if (
    now - lastIdleMove >
    nextIdleMove
  ) {

    int target =
      random(
        45,
        116
      );

    movePan(target);

    setEmotion(CURIOUS);

    lastIdleMove = now;

    nextIdleMove =
      random(
        4000,
        10000
      );
  }

  if (
    now - lastInteraction > 90000
  ) {

    setEmotion(BORED);
  }
}

// =====================================================
// SERVO
// =====================================================

void movePan(
  int position
) {

  position =
    constrain(
      position,
      20,
      150
    );

  panPosition = position;

  panServo.write(
    panPosition
  );
}

// =====================================================
// SOUND ENGINE
// =====================================================

void startSound(
  SoundType sound
) {

  currentSound = sound;

  soundStep = 0;

  soundPlaying = true;

  lastSoundStep = millis();
}

void updateSound(
  unsigned long now
) {

  if (
    !soundPlaying
  ) {
    return;
  }

  if (
    now - lastSoundStep < 110
  ) {
    return;
  }

  lastSoundStep = now;

  switch (currentSound) {

    case SOUND_HAPPY:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 700, 70);
      } else if (soundStep == 1) {
        tone(BUZZER_RIGHT, 950, 90);
      } else {
        stopSound();
      }

      break;

    case SOUND_SLEEPY:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 350, 180);
      } else {
        stopSound();
      }

      break;

    case SOUND_EXCITED:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 700, 70);
      } else if (soundStep == 1) {
        tone(BUZZER_RIGHT, 1000, 70);
      } else if (soundStep == 2) {
        tone(BUZZER_LEFT, 1300, 100);
      } else {
        stopSound();
      }

      break;

    case SOUND_CURIOUS:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 500, 80);
      } else if (soundStep == 1) {
        tone(BUZZER_RIGHT, 750, 100);
      } else {
        stopSound();
      }

      break;

    case SOUND_SCARED:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 1100, 80);
      } else if (soundStep == 1) {
        tone(BUZZER_RIGHT, 1400, 100);
      } else {
        stopSound();
      }

      break;

    case SOUND_LOVE:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 600, 100);
      } else if (soundStep == 1) {
        tone(BUZZER_RIGHT, 800, 100);
      } else if (soundStep == 2) {
        tone(BUZZER_LEFT, 1000, 100);
      } else {
        stopSound();
      }

      break;

    case SOUND_ANGRY:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 900, 100);
      } else if (soundStep == 1) {
        tone(BUZZER_RIGHT, 600, 120);
      } else {
        stopSound();
      }

      break;

    case SOUND_SPECIAL:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 600, 100);
      } else if (soundStep == 1) {
        tone(BUZZER_RIGHT, 900, 100);
      } else if (soundStep == 2) {
        tone(BUZZER_LEFT, 1200, 120);
      } else {
        stopSound();
      }

      break;

    case SOUND_COUNTDOWN:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 500, 80);
      } else if (soundStep == 1) {
        tone(BUZZER_RIGHT, 500, 80);
      } else if (soundStep == 2) {
        tone(BUZZER_LEFT, 900, 120);
      } else {
        stopSound();
      }

      break;

    case SOUND_SUCCESS:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 800, 80);
      } else if (soundStep == 1) {
        tone(BUZZER_RIGHT, 1100, 120);
      } else {
        stopSound();
      }

      break;

    case SOUND_FAIL:

      if (soundStep == 0) {
        tone(BUZZER_LEFT, 500, 120);
      } else if (soundStep == 1) {
        tone(BUZZER_RIGHT, 300, 180);
      } else {
        stopSound();
      }

      break;

    default:
      stopSound();
      break;
  }

  soundStep++;
}

void stopSound() {

  noTone(
    BUZZER_LEFT
  );

  noTone(
    BUZZER_RIGHT
  );

  soundPlaying = false;

  currentSound = SOUND_NONE;
}

// =====================================================
// GAME SYSTEM
// =====================================================

void stopGame() {

  gameStarted = false;

  gameWaiting = false;

  gameTimer = millis();

  simonInputIndex = 0;

  rhythmScore = 0;
}

void updateGame(
  unsigned long now
) {

  if (
    currentGame == NO_GAME
  ) {
    return;
  }

  if (
    !gameStarted
  ) {

    gameStarted = true;

    gameWaiting = true;

    prepareGame();

    return;
  }

  switch (currentGame) {

    case REACTION_GAME:
      updateReactionGame(now);
      break;

    case SIMON_GAME:
      updateSimonGame(now);
      break;

    case RHYTHM_GAME:
      updateRhythmGame(now);
      break;

    case HOT_COLD_GAME:
      updateHotColdGame(now);
      break;

    case REFLEX_DUEL:
      updateReflexDuel(now);
      break;

    default:
      break;
  }
}

void prepareGame() {

  unsigned long now = millis();

  if (
    currentGame == REACTION_GAME
  ) {

    reactionTarget =
      random(
        1,
        3
      );

    gameTimer =
      now +
      random(
        1000,
        3000
      );
  }

  if (
    currentGame == SIMON_GAME
  ) {

    simonLength = 3;

    simonInputIndex = 0;

    for (
      int i = 0;
      i < simonLength;
      i++
    ) {

      simonSequence[i] =
        random(
          0,
          3
        );
    }

    gameTimer = now;
  }

  if (
    currentGame == RHYTHM_GAME
  ) {

    rhythmScore = 0;

    gameTimer = now;
  }

  if (
    currentGame == HOT_COLD_GAME
  ) {

    hotColdTarget =
      random(
        15,
        50
      );

    gameTimer = now;
  }

  if (
    currentGame == REFLEX_DUEL
  ) {

    gameTimer =
      now +
      random(
        2000,
        5000
      );
  }
}

// =====================================================
// GAME INPUT
// =====================================================

void handleGameInput(
  int input
) {

  if (
    currentGame == NO_GAME
  ) {
    return;
  }

  if (
    currentGame == REACTION_GAME &&
    !gameWaiting
  ) {

    if (
      input == reactionTarget
    ) {

      endGame(true);

    } else {

      endGame(false);
    }

    return;
  }

  if (
    currentGame == SIMON_GAME &&
    !gameWaiting
  ) {

    if (
      input ==
      simonSequence[simonInputIndex]
    ) {

      simonInputIndex++;

      if (
        simonInputIndex >=
        simonLength
      ) {

        endGame(true);
      }

    } else {

      endGame(false);
    }

    return;
  }

  if (
    currentGame == RHYTHM_GAME
  ) {

    rhythmScore++;
  }

  if (
    currentGame == REFLEX_DUEL &&
    !gameWaiting
  ) {

    endGame(true);
  }
}

// =====================================================
// REACTION GAME
// =====================================================

void updateReactionGame(
  unsigned long now
) {

  if (
    gameWaiting &&
    now >= gameTimer
  ) {

    gameWaiting = false;

    gameTimer = now;

    if (
      reactionTarget == 1
    ) {

      lookLeft();

    } else {

      lookRight();
    }

    startSound(
      SOUND_COUNTDOWN
    );
  }

  if (
    !gameWaiting &&
    now - gameTimer > 3000
  ) {

    endGame(false);
  }
}

// =====================================================
// SIMON GAME
// =====================================================

void updateSimonGame(
  unsigned long now
) {

  if (
    gameWaiting &&
    now - gameTimer > 1500
  ) {

    gameWaiting = false;

    simonInputIndex = 0;

    int step =
      simonSequence[0];

    if (step == 0) {

      happyEyes();

    } else if (step == 1) {

      lookLeft();

    } else {

      lookRight();
    }
  }
}

// =====================================================
// RHYTHM GAME
// =====================================================

void updateRhythmGame(
  unsigned long now
) {

  if (
    now - gameTimer > 6000
  ) {

    if (
      rhythmScore >= 3
    ) {

      endGame(true);

    } else {

      endGame(false);
    }
  }
}

// =====================================================
// HOT AND COLD
// =====================================================

void updateHotColdGame(
  unsigned long now
) {

  float difference =
    abs(
      currentDistance -
      hotColdTarget
    );

  if (
    difference < 5
  ) {

    endGame(true);

    return;
  }

  if (
    difference < 12
  ) {

    setEmotion(EXCITED);

  } else if (
    difference < 25
  ) {

    setEmotion(CURIOUS);

  } else {

    setEmotion(BORED);
  }

  if (
    now - gameTimer > 15000
  ) {

    endGame(false);
  }
}

// =====================================================
// REFLEX DUEL
// =====================================================

void updateReflexDuel(
  unsigned long now
) {

  if (
    gameWaiting &&
    now >= gameTimer
  ) {

    gameWaiting = false;

    gameTimer = now;

    setEmotion(EXCITED);

    startSound(
      SOUND_EXCITED
    );
  }

  if (
    !gameWaiting &&
    now - gameTimer > 2500
  ) {

    endGame(false);
  }
}

// =====================================================
// END GAME
// =====================================================

void endGame(
  bool success
) {

  if (success) {

    mood += 10;

    affection += 3;

    setEmotion(EXCITED);

    startSound(
      SOUND_SUCCESS
    );

  } else {

    mood -= 3;

    setEmotion(SAD);

    startSound(
      SOUND_FAIL
    );
  }

  mood = constrain(
    mood,
    0,
    100
  );

  affection = constrain(
    affection,
    0,
    100
  );

  currentGame = NO_GAME;

  currentState = IDLE;

  stopGame();

  lastInteraction =
    millis();
}

// =====================================================
// OLED DRAWING
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

void sleepyEyes() {

  display.clearDisplay();

  display.fillRoundRect(
    15,
    30,
    38,
    16,
    8,
    SSD1306_WHITE
  );

  display.fillRoundRect(
    75,
    30,
    38,
    16,
    8,
    SSD1306_WHITE
  );

  display.display();
}

void sadEyes() {

  display.clearDisplay();

  display.drawLine(
    15,
    28,
    30,
    38,
    SSD1306_WHITE
  );

  display.drawLine(
    30,
    38,
    50,
    28,
    SSD1306_WHITE
  );

  display.drawLine(
    75,
    28,
    90,
    38,
    SSD1306_WHITE
  );

  display.drawLine(
    90,
    38,
    110,
    28,
    SSD1306_WHITE
  );

  display.display();
}

void boredEyes() {

  display.clearDisplay();

  display.fillRoundRect(
    15,
    25,
    38,
    20,
    10,
    SSD1306_WHITE
  );

  display.fillRoundRect(
    75,
    32,
    38,
    13,
    8,
    SSD1306_WHITE
  );

  display.display();
}

void scaredEyes() {

  display.clearDisplay();

  drawEye(
    10,
    6,
    45,
    52
  );

  drawEye(
    73,
    6,
    45,
    52
  );

  display.fillCircle(
    32,
    32,
    6,
    SSD1306_BLACK
  );

  display.fillCircle(
    95,
    32,
    6,
    SSD1306_BLACK
  );

  display.display();
}

void loveEyes() {

  display.clearDisplay();

  drawHeart(
    33,
    28
  );

  drawHeart(
    94,
    28
  );

  display.display();
}

void drawHeart(
  int x,
  int y
) {

  display.fillCircle(
    x - 7,
    y - 5,
    8,
    SSD1306_WHITE
  );

  display.fillCircle(
    x + 7,
    y - 5,
    8,
    SSD1306_WHITE
  );

  display.fillTriangle(
    x - 15,
    y - 2,
    x + 15,
    y - 2,
    x,
    y + 20,
    SSD1306_WHITE
  );
}

void angryEyes() {

  display.clearDisplay();

  display.fillTriangle(
    15,
    15,
    53,
    30,
    15,
    45,
    SSD1306_WHITE
  );

  display.fillTriangle(
    113,
    15,
    75,
    30,
    113,
    45,
    SSD1306_WHITE
  );

  display.display();
}