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
  digitalWrite(motorRightForward, LOW);
  analogWrite(motorLeftPWM, speed);
  analogWrite(motorRightPWM, speed);
  delay(duration);
}

void moveBackward(int duration, int speed) {
  digitalWrite(motorLeftForward, LOW);
  digitalWrite(motorRightForward, HIGH);
  analogWrite(motorLeftPWM, speed);
  analogWrite(motorRightPWM, speed);
  delay(duration);
}

void turnLeft(int duration, int speed) {
  digitalWrite(motorLeftForward, LOW);
  digitalWrite(motorRightForward, LOW);
  analogWrite(motorLeftPWM, speed);
  analogWrite(motorRightPWM, speed);
  delay(duration);
}

void stopMotors() {
  analogWrite(motorLeftPWM, 0);
  analogWrite(motorRightPWM, 0);
}


void Ultrasonic_Avoidance() {
  Funtion_FLag = true;
  while (Funtion_FLag) {
    int Front_Distance = 0;
    int Left_Distance = 0;
    int Right_Distance = 0;
    int Right_IR_Value = 1;
    int Left_IR_Value = 1;
    Left_IR_Value = digitalRead(A1);
    Right_IR_Value = digitalRead(A2);
    Front_Distance = checkdistance();
    Serial.println(Front_Distance);
    if (Left_IR_Value == 0 && Right_IR_Value == 1) {
      digitalWrite(2,HIGH);
      analogWrite(5,255);
      digitalWrite(4,LOW);
      analogWrite(6,12);

    } else if (Left_IR_Value == 1 && Right_IR_Value == 0) {
      digitalWrite(2,HIGH);
      analogWrite(5,12);
      digitalWrite(4,LOW);
      analogWrite(6,255);
    } else {
      digitalWrite(2,HIGH);
      analogWrite(5,(4 * 22.5));
      digitalWrite(4,LOW);
      analogWrite(6,(4 * 22.5));

    }
    if (Front_Distance <= D_mid) {
      digitalWrite(2,LOW);
      analogWrite(5,0);
      digitalWrite(4,HIGH);
      analogWrite(6,0);
      if (Front_Distance <= D_mix || Left_IR_Value == 0 && Right_IR_Value == 0) {
        digitalWrite(2,LOW);
        analogWrite(5,(4.5 * 22.5));
        digitalWrite(4,HIGH);
        analogWrite(6,(4.5 * 22.5));
        delay(300);
        digitalWrite(2,LOW);
        analogWrite(5,0);
        digitalWrite(4,HIGH);
        analogWrite(6,0);

      }
      myservo.write(165);
      delay(500);
      Serial.println(Left_Distance);
      delay(100);
      Left_Distance = checkdistance();
      myservo.write(15);
      delay(500);
      Serial.println(Right_Distance);
      delay(100);
      Right_Distance = checkdistance();
      myservo.write(90);
      if ((D_mix < Left_Distance && Left_Distance < D_max) && (D_mix < Right_Distance && Right_Distance < D_max)) {
        if (Right_Distance > Left_Distance) {
          digitalWrite(2,HIGH);
          analogWrite(5,(9 * 22.5));
          digitalWrite(4,HIGH);
          analogWrite(6,(9 * 22.5));
          delay(250);

        } else {
          digitalWrite(2,LOW);
          analogWrite(5,(9 * 22.5));
          digitalWrite(4,LOW);
          analogWrite(6,(9 * 22.5));
          delay(250);

        }

      } else if (D_mix < Left_Distance && Left_Distance < D_max || D_mix < Right_Distance && Right_Distance < D_max) {
        if (D_mix < Left_Distance && Left_Distance < D_max) {
          digitalWrite(2,LOW);
          analogWrite(5,(7 * 22.5));
          digitalWrite(4,LOW);
          analogWrite(6,(7 * 22.5));
          delay(250);

        } else if (D_mix < Right_Distance && Right_Distance < D_max) {
          digitalWrite(2,HIGH);
          analogWrite(5,(7 * 22.5));
          digitalWrite(4,HIGH);
          analogWrite(6,(7 * 22.5));
          delay(250);
        }
      } else if (Right_Distance < D_mix && Left_Distance < D_mix) {
        digitalWrite(2,HIGH);
        analogWrite(5,0);
        digitalWrite(4,LOW);
        analogWrite(6,(9 * 22.5));
        delay(510);
        digitalWrite(2,LOW);
        analogWrite(5,0);
        digitalWrite(4,HIGH);
        analogWrite(6,0);
      }
      digitalWrite(2,LOW);
      analogWrite(5,0);
      digitalWrite(4,HIGH);
      analogWrite(6,0);

    }
    BLE_value = "";
    while (Serial.available() > 0) {
      BLE_value = BLE_value + ((char)(Serial.read()));
      delay(2);
    }
    if ('%' == String(BLE_value).charAt(0) && 'Q' == String(BLE_value).charAt(1)) {
      Funtion_FLag = false;
      digitalWrite(2,LOW);
      analogWrite(5,0);
      digitalWrite(4,HIGH);
      analogWrite(6,0);

    }
  }
}

void Ultrasonic_Follow() {
  Funtion_FLag = true;
  while (Funtion_FLag) {
    int Infrared_Trigger_Flag = 0;
    int Front_Distance = 0;
    int Left_Distance = 0;
    int Right_Distance = 0;
    int Right_IR_Value = 1;
    int Left_IR_Value = 1;
    Left_IR_Value = digitalRead(A1);
    Right_IR_Value = digitalRead(A2);
    Front_Distance = checkdistance();
    if (Front_Distance < 5 && (Left_IR_Value != Infrared_Trigger_Flag && Right_IR_Value != Infrared_Trigger_Flag)) {
      digitalWrite(2,LOW);
      analogWrite(5,(3 * 25.5));
      digitalWrite(4,HIGH);
      analogWrite(6,(3 * 25.5));

    } else if (Front_Distance < 5 && (Left_IR_Value == Infrared_Trigger_Flag && Right_IR_Value != Infrared_Trigger_Flag)) {
      digitalWrite(2,LOW);
      analogWrite(5,(4 * 25.5));
      digitalWrite(4,HIGH);
      analogWrite(6,(0.056 * (4 * 255)));
    } else if (Front_Distance < 5 && (Left_IR_Value != Infrared_Trigger_Flag && Right_IR_Value == Infrared_Trigger_Flag)) {
      digitalWrite(2,LOW);
      analogWrite(5,(0.056 * (4 * 255)));
      digitalWrite(4,HIGH);
      analogWrite(6,(4 * 25.5));
    } else if (Front_Distance < 5 && (Left_IR_Value == Infrared_Trigger_Flag && Right_IR_Value == Infrared_Trigger_Flag)) {
      digitalWrite(2,LOW);
      analogWrite(5,(3 * 25.5));
      digitalWrite(4,HIGH);
      analogWrite(6,(3 * 25.5));
    } else if (Front_Distance > 10 && (Left_IR_Value != Infrared_Trigger_Flag && Right_IR_Value != Infrared_Trigger_Flag)) {
      digitalWrite(2,HIGH);
      analogWrite(5,(4 * 25.5));
      digitalWrite(4,LOW);
      analogWrite(6,(4 * 25.5));
    } else if (Front_Distance > 10 && (Left_IR_Value == Infrared_Trigger_Flag && Right_IR_Value != Infrared_Trigger_Flag)) {
      digitalWrite(2,LOW);
      analogWrite(5,(4 * 25.5));
      digitalWrite(4,LOW);
      analogWrite(6,(4 * 25.5));
    } else if (Front_Distance > 10 && (Left_IR_Value != Infrared_Trigger_Flag && Right_IR_Value == Infrared_Trigger_Flag)) {
      digitalWrite(2,HIGH);
      analogWrite(5,(4 * 25.5));
      digitalWrite(4,HIGH);
      analogWrite(6,(4 * 25.5));
    } else if ((5 <= Front_Distance && Front_Distance <= 10) && (Left_IR_Value != Infrared_Trigger_Flag && Right_IR_Value == Infrared_Trigger_Flag)) {
      digitalWrite(2,HIGH);
      analogWrite(5,(4 * 25.5));
      digitalWrite(4,LOW);
      analogWrite(6,(0.056 * (4 * 25.5)));
    } else if ((5 <= Front_Distance && Front_Distance <= 10) && (Left_IR_Value == Infrared_Trigger_Flag && Right_IR_Value != Infrared_Trigger_Flag)) {
      digitalWrite(2,HIGH);
      analogWrite(5,(0.056 * (4 * 25.5)));
      digitalWrite(4,LOW);
      analogWrite(6,(4 * 25.5));
    } else if ((5 <= Front_Distance && Front_Distance <= 10) && (Left_IR_Value != Infrared_Trigger_Flag && Right_IR_Value != Infrared_Trigger_Flag)) {
      digitalWrite(2,LOW);
      analogWrite(5,0);
      digitalWrite(4,HIGH);
      analogWrite(6,0);
    }
    BLE_value = "";
    while (Serial.available() > 0) {
      BLE_value = BLE_value + ((char)(Serial.read()));
      delay(2);
    }
    if ('%' == String(BLE_value).charAt(0) && 'Q' == String(BLE_value).charAt(1)) {
      Funtion_FLag = false;
      digitalWrite(2,LOW);
      analogWrite(5,0);
      digitalWrite(4,HIGH);
      analogWrite(6,0);

    }
  }
}

void Infrared_Tracing() {
  Funtion_FLag = true;
  int Left_Tra_Value = 1;
  int Center_Tra_Value = 1;
  int Right_Tra_Value = 1;
  int Black = 1;
  int white = 0;
  while (Funtion_FLag) {
    Left_Tra_Value = digitalRead(7);
    Center_Tra_Value = digitalRead(8);
    Right_Tra_Value = digitalRead(9);
    if (Left_Tra_Value != Black && (Center_Tra_Value == Black && Right_Tra_Value != Black)) {
      digitalWrite(2,HIGH);
      analogWrite(5,120);
      digitalWrite(4,LOW);
      analogWrite(6,120);

    } else if (Left_Tra_Value == Black && (Center_Tra_Value == Black && Right_Tra_Value != Black)) {
      digitalWrite(2,LOW);
      analogWrite(5,120);
      digitalWrite(4,LOW);
      analogWrite(6,120);
    } else if (Left_Tra_Value == Black && (Center_Tra_Value != Black && Right_Tra_Value != Black)) {
      digitalWrite(2,LOW);
      analogWrite(5,80);
      digitalWrite(4,LOW);
      analogWrite(6,80);
    } else if (Left_Tra_Value != Black && (Center_Tra_Value != Black && Right_Tra_Value == Black)) {
      digitalWrite(2,HIGH);
      analogWrite(5,80);
      digitalWrite(4,HIGH);
      analogWrite(6,80);
    } else if (Left_Tra_Value != Black && (Center_Tra_Value == Black && Right_Tra_Value == Black)) {
      digitalWrite(2,HIGH);
      analogWrite(5,120);
      digitalWrite(4,HIGH);
      analogWrite(6,120);
    } else if (Left_Tra_Value == Black && (Center_Tra_Value == Black && Right_Tra_Value == Black)) {
      digitalWrite(2,LOW);
      analogWrite(5,0);
      digitalWrite(4,HIGH);
      analogWrite(6,0);
    } else if (false) {
    }
    BLE_value = "";
    while (Serial.available() > 0) {
      BLE_value = BLE_value + ((char)(Serial.read()));
      delay(2);
    }
    if ('%' == String(BLE_value).charAt(0) && 'Q' == String(BLE_value).charAt(1)) {
      Funtion_FLag = false;
      digitalWrite(2,LOW);
      analogWrite(5,0);
      digitalWrite(4,HIGH);
      analogWrite(6,0);

    }
  }
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