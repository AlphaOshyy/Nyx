#include <SPI.h>
#include <U8g2lib.h>

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(
  U8G2_R0,
  10,   // CS
  9,    // DC
  8     // RESET
);

void setup() {
  u8g2.begin();
}

void loop() {
  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_ncenB14_tr);
  u8g2.drawStr(10,30,"Hello!");

  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(10,50,"SPI OLED Test");

  u8g2.sendBuffer();

  delay(1000);
}
