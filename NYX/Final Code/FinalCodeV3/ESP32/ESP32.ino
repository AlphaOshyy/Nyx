#include "esp_camera.h"

// ========================================
// NYX V4
// Real Camera Motion Detection
// ESP32 CAM AI Thinker
// ========================================

// ESP32 CAM -> Nano
#define NANO_TX 16

HardwareSerial nanoSerial(1);

// ========================================
// AI Thinker ESP32 CAM Pins
// ========================================

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

// ========================================
// MOTION SETTINGS
// ========================================

const int SAMPLE_STEP = 40;
const int MOTION_THRESHOLD = 25;
const int CHANGED_PIXELS = 120;

uint8_t *previousFrame = NULL;
size_t previousLength = 0;

unsigned long lastDetection = 0;
const unsigned long COOLDOWN = 5000;

// ========================================
// SETUP
// ========================================

void setup() {

  Serial.begin(115200);

  nanoSerial.begin(
    9600,
    SERIAL_8N1,
    -1,
    NANO_TX
  );

  delay(1000);

  Serial.println();
  Serial.println("NYX CAMERA STARTING");

  camera_config_t config;

  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;

  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;

  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;

  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;

  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;

  config.xclk_freq_hz = 20000000;

  config.pixel_format = PIXFORMAT_GRAYSCALE;

  config.frame_size = FRAMESIZE_QQVGA;

  config.jpeg_quality = 12;

  config.fb_count = 1;

  esp_err_t err = esp_camera_init(&config);

  if (err != ESP_OK) {

    Serial.println("CAMERA FAILED");

    while (true) {
      delay(1000);
    }
  }

  Serial.println("CAMERA READY");

  nanoSerial.println("WAKE");
}

// ========================================
// MAIN LOOP
// ========================================

void loop() {

  camera_fb_t *frame = esp_camera_fb_get();

  if (!frame) {

    Serial.println("FRAME ERROR");

    delay(100);

    return;
  }

  bool motionDetected = false;

  // First frame
  if (previousFrame == NULL) {

    previousLength = frame->len;

    previousFrame =
      (uint8_t *) malloc(previousLength);

    if (previousFrame != NULL) {

      memcpy(
        previousFrame,
        frame->buf,
        previousLength
      );
    }

    esp_camera_fb_return(frame);

    delay(200);

    return;
  }

  int changedPixels = 0;

  size_t length =
    min(frame->len, previousLength);

  // Compare sampled pixels
  for (
    size_t i = 0;
    i < length;
    i += SAMPLE_STEP
  ) {

    int difference =
      abs(
        (int)frame->buf[i] -
        (int)previousFrame[i]
      );

    if (
      difference >
      MOTION_THRESHOLD
    ) {

      changedPixels++;

      if (
        changedPixels >
        CHANGED_PIXELS
      ) {

        motionDetected = true;

        break;
      }
    }
  }

  // Update previous frame
  free(previousFrame);

  previousLength = frame->len;

  previousFrame =
    (uint8_t *) malloc(previousLength);

  if (previousFrame != NULL) {

    memcpy(
      previousFrame,
      frame->buf,
      previousLength
    );
  }

  esp_camera_fb_return(frame);

  // Real motion event
  if (
    motionDetected &&
    millis() - lastDetection >
    COOLDOWN
  ) {

    Serial.println("REAL MOTION DETECTED");

    nanoSerial.println("CURIOUS");

    lastDetection = millis();
  }

  delay(150);
}