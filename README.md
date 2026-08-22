<div align="center">

# NYX

### An Interactive AI Desktop Pet

A compact robotics project combining embedded control, computer vision, sensing, animation, and personality.

<br>

<img src="https://img.shields.io/badge/Project-HND%20Robotics-7C3AED?style=for-the-badge" alt="HND Robotics">
<img src="https://img.shields.io/badge/Controller-Arduino%20Nano-00979D?style=for-the-badge&logo=arduino&logoColor=white" alt="Arduino Nano">
<img src="https://img.shields.io/badge/Vision-ESP32--CAM-E7352C?style=for-the-badge" alt="ESP32 CAM">
<img src="https://img.shields.io/badge/Language-C%2B%2B%20%7C%20Python-3776AB?style=for-the-badge" alt="C++ Python">

</div>

<br>

## ◈ About NYX

NYX is an interactive desktop pet developed as an HND Software Engineering robotics project.

The system combines an Arduino Nano and ESP32 based vision system with an OLED display, servo controlled head movement, ultrasonic sensing, audio hardware, and computer vision software.

The goal is simple. Build a small robot that feels interactive, expressive, and responsive on a desktop.

## ◇ What NYX Does

| System | Function |
| :--- | :--- |
| 👁️ OLED | Animated eyes and expressions |
| 🧠 ESP32 | Vision and connected processing |
| 📷 ESP32-CAM | Camera based interaction |
| 🦾 SG90 Servos | Pan and tilt head movement |
| 📡 HC-SR04 | Distance sensing |
| 🎙️ Sound Sensor | Detects environmental sound |
| 🔊 Audio | Sound and response output |
| 🧩 Arduino Nano | Main hardware controller |

## ◇ Personality

NYX uses expressive states to make its behaviour feel more alive.

```text
       ┌─────────────┐
       │     NYX     │
       └──────┬──────┘
              │
      ┌───────┼───────┐
      ▼       ▼       ▼
   HAPPY   CURIOUS  SLEEPY
              │
              ▼
           ANGRY
```

The OLED provides the visual feedback while the servos and sensors drive physical interaction.

## ◇ Technology Stack

```text
Hardware
├── Arduino Nano V3
├── ESP32-CAM
├── OLED SSD1306
├── HC-SR04
├── SG90 Servo ×2
├── Pan/Tilt Bracket
├── Sound Sensor / Microphone
├── DFPlayer Mini
├── MAX98357 I2S Amplifier
└── Speaker

Software
├── Arduino C++
├── Python
├── OpenCV
├── MediaPipe
└── TensorFlow Lite
```

## ◇ Repository Structure

The repository contains the project development history, testing code, final code, and project documentation.

```text
NYX/
│
├── NYX/
│   ├── Final Code/
│   │   ├── FinalCodeV0/
│   │   ├── FinalCodeV1_0408/
│   │   ├── FinalCodeV2_1008/
│   │   ├── FinalCodeV3/
│   │   └── FinalCodeV5/
│   │       ├── ArduinoNano/
│   │       └── ESP32/
│   │
│   └── Testing Codes/
│
├── COHNDSE261F039ROBOTICREPORT.docx
└── README.md
```

## ◇ Development

NYX has evolved through multiple hardware and software revisions.

The repository keeps the different final code versions so the development process remains traceable.

```text
V0  →  V1  →  V2  →  V3  →  V5
 │      │      │      │      │
 └──────┴──────┴──────┴──────┴── Development
```

## ◇ Project Focus

• Embedded systems
• Robotics
• Computer vision
• Human interaction
• Sensor integration
• Servo control
• OLED animation
• AI assisted vision
• Python and Arduino communication

## ◇ Current Direction

NYX is being developed as a desktop companion rather than a mobile robot.

The current design focuses on expressive OLED animation, head movement, environmental sensing, ESP32-CAM vision, and interactive behaviour.

## ◇ Academic Project

NYX was developed as part of an HND Software Engineering robotics project.

Project documentation is included in the repository.

## ◇ Developer

<div align="center">

### Oshan Tanusha

Software Engineering Student

<br>

**NYX • Robotics • AI • Embedded Systems**

</div>

<br>

<div align="center">

Made with Arduino, ESP32, Python, and a lot of testing.

</div>
