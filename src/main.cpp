#include <Arduino.h>
void setup() {
  Serial.begin(115200);
  pinMode(18, INPUT_PULLUP);
}

void loop() {
  Serial.println(digitalRead(18));
  delay(200);
}