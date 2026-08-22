#include "esp_camera.h"

// =====================================================
// NYX V4
// ESP32 CAM Vision Controller
// =====================================================

// ESP32 TX -> Nano D6
#define NANO_TX 16

// Nano D7 -> voltage divider -> ESP32 GPIO14
#define NANO_STATE_PIN 14

HardwareSerial nanoSerial(1);

// =====================================================
// AI THINKER ESP32 CAM
// =====================================================

#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1

#define XCLK_GPIO_NUM      0

#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36

#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5

#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// =====================================================
// CAMERA
// =====================================================

const int FRAME_WIDTH = 160;
const int FRAME_HEIGHT = 120;

const int SAMPLE_STEP = 4;

// Pixel difference required
const int PIXEL_THRESHOLD = 32;

// Minimum amount of changed pixels
const int MIN_MOTION = 22;

// Strong movement
const int FAST_MOTION = 85;

// =====================================================
// VISION STABILITY
// =====================================================

String candidateDirection = "";
String stableDirection = "";

int candidateCount = 0;

const int REQUIRED_STABLE_FRAMES = 3;

// =====================================================
// TIMING
// =====================================================

unsigned long lastFrame = 0;

const unsigned long FRAME_INTERVAL = 45;

unsigned long lastCommand = 0;

const unsigned long COMMAND_INTERVAL = 150;

// =====================================================
// ROBOT STATE
// =====================================================

String robotState = "IDLE";

unsigned long lastRobotMessage = 0;

const unsigned long ROBOT_TIMEOUT = 2000;

// =====================================================
// PREVIOUS FRAME
// =====================================================

uint8_t *previousFrame = NULL;

// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  // ESP32 TX -> Nano D6
  nanoSerial.begin(
    9600,
    SERIAL_8N1,
    -1,
    NANO_TX
  );

  pinMode(
    NANO_STATE_PIN,
    INPUT
  );

  Serial.println();
  Serial.println(
    "========================"
  );

  Serial.println(
    "NYX V4 VISION"
  );

  Serial.println(
    "========================"
  );

  // ===================================================
  // CAMERA CONFIG
  // ===================================================

  camera_config_t config;

  config.ledc_channel =
    LEDC_CHANNEL_0;

  config.ledc_timer =
    LEDC_TIMER_0;

  config.pin_d0 =
    Y2_GPIO_NUM;

  config.pin_d1 =
    Y3_GPIO_NUM;

  config.pin_d2 =
    Y4_GPIO_NUM;

  config.pin_d3 =
    Y5_GPIO_NUM;

  config.pin_d4 =
    Y6_GPIO_NUM;

  config.pin_d5 =
    Y7_GPIO_NUM;

  config.pin_d6 =
    Y8_GPIO_NUM;

  config.pin_d7 =
    Y9_GPIO_NUM;

  config.pin_xclk =
    XCLK_GPIO_NUM;

  config.pin_pclk =
    PCLK_GPIO_NUM;

  config.pin_vsync =
    VSYNC_GPIO_NUM;

  config.pin_href =
    HREF_GPIO_NUM;

  config.pin_sscb_sda =
    SIOD_GPIO_NUM;

  config.pin_sscb_scl =
    SIOC_GPIO_NUM;

  config.pin_pwdn =
    PWDN_GPIO_NUM;

  config.pin_reset =
    RESET_GPIO_NUM;

  config.xclk_freq_hz =
    20000000;

  // Grayscale makes frame comparison faster
  config.pixel_format =
    PIXFORMAT_GRAYSCALE;

  config.frame_size =
    FRAMESIZE_QQVGA;

  config.jpeg_quality =
    12;

  config.fb_count =
    1;

  esp_err_t result =
    esp_camera_init(
      &config
    );

  if (
    result != ESP_OK
  ) {

    Serial.print(
      "CAMERA ERROR: "
    );

    Serial.println(
      result,
      HEX
    );

    while (true) {
      delay(1000);
    }
  }

  Serial.println(
    "CAMERA READY"
  );

  Serial.println(
    "VISION READY"
  );
}

// =====================================================
// LOOP
// =====================================================

void loop() {

  readNanoState();

  if (
    robotState != "NEAR"
  ) {

    delay(30);

    return;
  }

  if (
    millis() - lastFrame <
    FRAME_INTERVAL
  ) {

    return;
  }

  lastFrame =
    millis();

  processCamera();
}

// =====================================================
// READ NANO STATE
// =====================================================

void readNanoState() {

  while (
    nanoSerial.available()
  ) {

    String message =
      nanoSerial.readStringUntil(
        '\n'
      );

    message.trim();

    if (
      message == "NEAR"
    ) {

      robotState =
        "NEAR";

      lastRobotMessage =
        millis();

      Serial.println(
        "ROBOT: NEAR"
      );
    }

    else if (
      message == "IDLE"
    ) {

      robotState =
        "IDLE";

      lastRobotMessage =
        millis();

      Serial.println(
        "ROBOT: IDLE"
      );
    }
  }

  // Safety timeout

  if (
    millis() - lastRobotMessage >
    ROBOT_TIMEOUT
  ) {

    robotState =
      "IDLE";
  }
}

// =====================================================
// CAMERA PROCESSING
// =====================================================

void processCamera() {

  camera_fb_t *frame =
    esp_camera_fb_get();

  if (
    frame == NULL
  ) {

    Serial.println(
      "FRAME ERROR"
    );

    return;
  }

  // First frame

  if (
    previousFrame == NULL
  ) {

    previousFrame =
      (uint8_t *)malloc(
        frame->len
      );

    if (
      previousFrame != NULL
    ) {

      memcpy(
        previousFrame,
        frame->buf,
        frame->len
      );
    }

    esp_camera_fb_return(
      frame
    );

    return;
  }

  int leftMotion = 0;

  int centerMotion = 0;

  int rightMotion = 0;

  int totalMotion = 0;

  // ===================================================
  // COMPARE FRAME
  // ===================================================

  for (
    int y = 0;
    y < FRAME_HEIGHT;
    y += SAMPLE_STEP
  ) {

    for (
      int x = 0;
      x < FRAME_WIDTH;
      x += SAMPLE_STEP
    ) {

      int index =
        y * FRAME_WIDTH + x;

      if (
        index >= frame->len
      ) {

        continue;
      }

      int difference =
        abs(
          (int)frame->buf[index] -
          (int)previousFrame[index]
        );

      if (
        difference >
        PIXEL_THRESHOLD
      ) {

        totalMotion++;

        if (
          x < 53
        ) {

          leftMotion++;
        }

        else if (
          x < 107
        ) {

          centerMotion++;
        }

        else {

          rightMotion++;
        }
      }
    }
  }

  // ===================================================
  // SAVE CURRENT FRAME
  // ===================================================

  memcpy(
    previousFrame,
    frame->buf,
    frame->len
  );

  esp_camera_fb_return(
    frame
  );

  // ===================================================
  // FAST MOVEMENT
  // ===================================================

  if (
    totalMotion >
    FAST_MOTION
  ) {

    sendCommand(
      "FAST"
    );

    candidateCount = 0;

    return;
  }

  // ===================================================
  // NOT ENOUGH MOVEMENT
  // ===================================================

  if (
    totalMotion <
    MIN_MOTION
  ) {

    return;
  }

  // ===================================================
  // DETERMINE ZONE
  // ===================================================

  String direction =
    chooseDirection(
      leftMotion,
      centerMotion,
      rightMotion
    );

  stabilizeDirection(
    direction
  );
}

// =====================================================
// CHOOSE DIRECTION
// =====================================================

String chooseDirection(
  int left,
  int center,
  int right
) {

  // Add a small dead zone so tiny
  // differences do not cause jitter.

  if (
    abs(left - center) < 5 &&
    abs(right - center) < 5
  ) {

    return "CENTER";
  }

  if (
    left >= center &&
    left >= right
  ) {

    return "LEFT";
  }

  if (
    right >= left &&
    right >= center
  ) {

    return "RIGHT";
  }

  return "CENTER";
}

// =====================================================
// STABLE DIRECTION
// =====================================================

void stabilizeDirection(
  String direction
) {

  if (
    direction == ""
  ) {

    return;
  }

  if (
    direction ==
    candidateDirection
  ) {

    candidateCount++;

  } else {

    candidateDirection =
      direction;

    candidateCount = 1;
  }

  if (
    candidateCount >=
    REQUIRED_STABLE_FRAMES
  ) {

    if (
      stableDirection !=
      candidateDirection
    ) {

      stableDirection =
        candidateDirection;

      sendCommand(
        stableDirection
      );
    }
  }
}

// =====================================================
// SEND TO NANO
// =====================================================

void sendCommand(
  String command
) {

  if (
    millis() - lastCommand <
    COMMAND_INTERVAL
  ) {

    return;
  }

  lastCommand =
    millis();

  nanoSerial.println(
    command
  );

  Serial.print(
    "VISION -> "
  );

  Serial.println(
    command
  );
}