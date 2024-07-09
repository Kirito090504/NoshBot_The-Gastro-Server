#include "IR_remote.h"
#include "keymap.h"

IRremote ir(3);

// Pin definitions
const int motorLeftForward = 2;
const int motorLeftPWM = 5;
const int motorRightForward = 4;
const int motorRightPWM = 6;
const int lineSensorLeft = A0;
const int lineSensorRight = A1;

void setup() {
  pinMode(motorLeftForward, OUTPUT);
  pinMode(motorLeftPWM, OUTPUT);
  pinMode(motorRightForward, OUTPUT);
  pinMode(motorRightPWM, OUTPUT);
  pinMode(lineSensorLeft, INPUT);
  pinMode(lineSensorRight, INPUT);
}

void moveForward(int duration, int speed) {
  digitalWrite(motorLeftForward, HIGH);
  analogWrite(motorLeftPWM, speed);
  digitalWrite(motorRightForward, LOW);
  analogWrite(motorRightPWM, speed);
  delay(duration);
}

void moveBackward(int duration, int speed) {
  digitalWrite(motorLeftForward, LOW);
  analogWrite(motorLeftPWM, speed);
  digitalWrite(motorRightForward, HIGH);
  analogWrite(motorRightPWM, speed);
  delay(duration);
}

void turnLeft(int duration, int speed) {
  digitalWrite(motorLeftForward, LOW);
  analogWrite(motorLeftPWM, speed);
  digitalWrite(motorRightForward, LOW);
  analogWrite(motorRightPWM, speed);
  delay(duration);
}

void stopMotors() {
  analogWrite(motorLeftPWM, 0);
  analogWrite(motorRightPWM, 0);
}

void loop() {
  if (ir.getIrKey(ir.getCode(),1) == IR_KEYCODE_UP) {
    moveForward(0, 150);
  } else if (ir.getIrKey(ir.getCode(),1) == IR_KEYCODE_DOWN) {
    moveBackward(0, 150);
  } else if (ir.getIrKey(ir.getCode(),1) == IR_KEYCODE_LEFT) {
    turnLeft(200, 50);
    stopMotors();
  } else if (ir.getIrKey(ir.getCode(),1) == IR_KEYCODE_RIGHT) {
    digitalWrite(motorLeftForward, HIGH);
    analogWrite(motorLeftPWM, 50);
    digitalWrite(motorRightForward, HIGH);
    analogWrite(motorRightPWM, 50);
    delay(200);
    stopMotors();
  } else if (ir.getIrKey(ir.getCode(),1) == IR_KEYCODE_OK) {
    stopMotors();
  } else if (ir.getIrKey(ir.getCode(),1) == IR_KEYCODE_1) {
    // Move straight for 3 seconds
    moveForward(2000, 200);

    // Turn left
    turnLeft(500, 100);

    // Move straight for 1 second
    moveForward(1000, 200);

    // Stop for 30 seconds
    stopMotors();
    delay(3000); // wait for 30 seconds

    // Move backward for 1 second
    moveBackward(1000, 200);

    // Turn left
    turnLeft(600, 100);

    // Move straight for 3 seconds
    moveForward(2000, 200);
  }else if (ir.getIrKey(ir.getCode(),1) == IR_KEYCODE_2) {
    // Move straight for 3 seconds
    moveForward(4000, 200);

    // Turn left
    turnLeft(500, 100);

    // Move straight for 1 second
    moveForward(1000, 200);

    // Stop for 30 seconds
    stopMotors();
    delay(3000); // wait for 30 seconds

    // Move backward for 1 second
    moveBackward(1000, 200);

    // Turn left
    turnLeft(600, 100);

    // Move straight for 4 seconds
    moveForward(4000, 200);
  } else if (ir.getIrKey(ir.getCode(),1) == IR_KEYCODE_3) {
    digitalWrite(motorLeftForward, HIGH);
    analogWrite(motorLeftPWM, 130);
    digitalWrite(motorRightForward, HIGH);
    analogWrite(motorRightPWM, 130);
  }

  // Line-following logic
  // int leftSensorValue = digitalRead(lineSensorLeft);
  // int rightSensorValue = digitalRead(lineSensorRight);

  // if (leftSensorValue == HIGH && rightSensorValue == LOW) {
  //   // Turn right
  //   digitalWrite(motorLeftForward, HIGH);
  //   analogWrite(motorLeftPWM, 150);
  //   digitalWrite(motorRightForward, HIGH);
  //   analogWrite(motorRightPWM, 0);
  // } else if (leftSensorValue == LOW && rightSensorValue == HIGH) {
  //   // Turn left
  //   digitalWrite(motorLeftForward, HIGH);
  //   analogWrite(motorLeftPWM, 0);
  //   digitalWrite(motorRightForward, HIGH);
  //   analogWrite(motorRightPWM, 150);
  // } else if (leftSensorValue == LOW && rightSensorValue == LOW) {
  //   // Move forward
  //   digitalWrite(motorLeftForward, HIGH);
  //   analogWrite(motorLeftPWM, 150);
  //   digitalWrite(motorRightForward, LOW);
  //   analogWrite(motorRightPWM, 150);
  // } else {
  //   // Stop
  //   stopMotors();
  // }
}
