// ==========================================
// 3 TOUCH SENSOR TEST - ARDUINO UNO
// ==========================================

const int TOUCH_1 = 2;
const int TOUCH_2 = 3;
const int TOUCH_3 = 4;

void setup() {
  Serial.begin(9600);

  pinMode(TOUCH_1, INPUT);
  pinMode(TOUCH_2, INPUT);
  pinMode(TOUCH_3, INPUT);

  Serial.println("3 Touch Sensor Test Started");
}

void loop() {

  int touch1 = digitalRead(TOUCH_1);
  int touch2 = digitalRead(TOUCH_2);
  int touch3 = digitalRead(TOUCH_3);

  Serial.print("Touch 1: ");
  Serial.print(touch1 == HIGH ? "TOUCHED" : "NOT TOUCHED");

  Serial.print(" | Touch 2: ");
  Serial.print(touch2 == HIGH ? "TOUCHED" : "NOT TOUCHED");

  Serial.print(" | Touch 3: ");
  Serial.println(touch3 == HIGH ? "TOUCHED" : "NOT TOUCHED");

  delay(200);
}