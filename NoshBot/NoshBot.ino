#include <Servo.h>

#include "IR_remote.h"
#include "keymap.h"

#define BAUDRATE 9600

#define PIN_IR_REMOTE 3
#define LINE_SENSOR_LEFT A0
#define LINE_SENSOR_RIGHT A1
#define MOTOR_LEFT_FORWARD 2
#define MOTOR_LEFT_PWM 5
#define MOTOR_RIGHT_FORWARD 4
#define MOTOR_RIGHT_PWM 6

IRremote ir(PIN_IR_REMOTE);

// Left_Tra = 7;
// Center_Tra = 8;
// Right_Tra = 9;

void setup()
{
    Serial.begin(BAUDRATE);
    Serial.println("Getting ready...");
    pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
    pinMode(MOTOR_LEFT_PWM, OUTPUT);
    pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
    pinMode(MOTOR_RIGHT_PWM, OUTPUT);
    pinMode(LINE_SENSOR_LEFT, INPUT);
    pinMode(LINE_SENSOR_RIGHT, INPUT);
    Serial.println("Ready.");
}

/* Move Noshbot forward */
void moveForward(int duration, int speed)
{
    digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
    analogWrite(MOTOR_LEFT_PWM, speed);
    digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
    analogWrite(MOTOR_RIGHT_PWM, speed);
    delay(duration);
}

/* Move Noshbot backward */
void moveBackward(int duration, int speed)
{
    digitalWrite(MOTOR_LEFT_FORWARD, LOW);
    analogWrite(MOTOR_LEFT_PWM, speed);
    digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
    analogWrite(MOTOR_RIGHT_PWM, speed);
    delay(duration);
}

/* Turn NoshBot left */
void turnLeft(int duration, int speed)
{
    digitalWrite(MOTOR_LEFT_FORWARD, LOW);
    analogWrite(MOTOR_LEFT_PWM, speed);
    digitalWrite(MOTOR_RIGHT_FORWARD, LOW);
    analogWrite(MOTOR_RIGHT_PWM, speed);
    delay(duration);
}

/* Turn NoshBot right */
void turnRight(int duration, int speed)
{
    digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
    analogWrite(MOTOR_LEFT_PWM, speed);
    digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
    analogWrite(MOTOR_RIGHT_PWM, speed);
    delay(duration);
}

/* Stop NoshBot's motor functions */
void stopMotors()
{
    analogWrite(MOTOR_LEFT_PWM, 0);
    analogWrite(MOTOR_RIGHT_PWM, 0);
}

void loop()
{
    // int Left_Tra = 1;
    // int Center_Tra = 1;
    // int Right_Tra = 1;
    // int Black = 1;

    byte ir_command = ir.getIrKey(ir.getCode(), 1);

    switch (ir_command)
    {
    case IR_KEYCODE_UP: // Go forward.
        moveForward(0, 150);
        break;

    case IR_KEYCODE_DOWN: // Go backward.
        moveBackward(0, 150);
        break;

    case IR_KEYCODE_LEFT: // Turn left.
        turnLeft(200, 50);
        stopMotors();
        break;

    case IR_KEYCODE_RIGHT: // Turn right.
        turnRight(200, 50);
        stopMotors();
        break;

    case IR_KEYCODE_OK: // Stop moving.
        stopMotors();
        break;

    case IR_KEYCODE_1:
        moveForward(2000, 200);  // Move straight for 2 seconds
        turnLeft(500, 100);      // Turn left
        moveForward(1000, 200);  // Move straight for 1 second
        stopMotors();            // Stop
        delay(3000);             // wait for 3 seconds
        moveBackward(1000, 200); // Move backward for 1 second
        turnLeft(600, 100);      // Turn left
        moveForward(2000, 200);  // Move straight for 2 seconds
        break;

    case IR_KEYCODE_2:
        moveForward(4000, 200);  // Move straight for 4 seconds
        turnLeft(500, 100);      // Turn left
        moveForward(1000, 200);  // Move straight for 1 second
        stopMotors();            // Stop
        delay(3000);             // wait for 3 seconds
        moveBackward(1000, 200); // Move backward for 1 second
        turnLeft(600, 100);      // Turn left
        moveForward(4000, 200);  // Move straight for 4 seconds
        break;

    case IR_KEYCODE_3:
        digitalWrite(MOTOR_LEFT_FORWARD, HIGH);
        analogWrite(MOTOR_LEFT_PWM, 130);
        digitalWrite(MOTOR_RIGHT_FORWARD, HIGH);
        analogWrite(MOTOR_RIGHT_PWM, 130);
        break;

    default:
        Serial.println("ERROR: Unknown IR command.");
        break;
    }
}