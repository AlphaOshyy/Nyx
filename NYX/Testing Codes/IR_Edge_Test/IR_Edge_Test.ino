#define LEFT_IR 4
#define RIGHT_IR 5

void setup() {

  Serial.begin(9600);

  pinMode(LEFT_IR, INPUT);
  pinMode(RIGHT_IR, INPUT);

  Serial.println("NYX Dual IR Test");

}


void loop() {

  int leftState = digitalRead(LEFT_IR);
  int rightState = digitalRead(RIGHT_IR);


  Serial.print("Left: ");
  Serial.print(leftState);

  Serial.print("  Right: ");
  Serial.println(rightState);


  delay(200);

}