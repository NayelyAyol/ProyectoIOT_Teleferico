#include <Servo.h>

Servo servo360;

void setup() {
  servo360.attach(8);
}

void loop() {
  servo360.write(180);
}
