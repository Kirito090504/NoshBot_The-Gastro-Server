#include <Servo.h>

#include "IR_remote.h"
#include "keymap.h"

#define BAUDRATE 9600

// PIN assignments
#define PIN_IR_REMOTE 3
#define IR_SENSOR_LEFT A0
#define IR_SENSOR_RIGHT A1
#define MOTOR_LEFT_FORWARD 2
#define MOTOR_LEFT_PWM 5
#define MOTOR_RIGHT_FORWARD 4
#define MOTOR_RIGHT_PWM 6
#define LEFT_TRA 7
#define CENTER_TRA 8
#define RIGHT_TRA 9

#define MOVEMENT_SPEED 80
#define TURN_SPEED 50

int current_row = 0;
// bool at_home = true;
bool destination_reached = false;
IRremote ir(PIN_IR_REMOTE);

void setup()
{
    Serial.begin(BAUDRATE);
    Serial.println("Getting ready...");
    pinMode(MOTOR_LEFT_FORWARD, OUTPUT);
    pinMode(MOTOR_LEFT_PWM, OUTPUT);
    pinMode(MOTOR_RIGHT_FORWARD, OUTPUT);
    pinMode(MOTOR_RIGHT_PWM, OUTPUT);
    pinMode(IR_SENSOR_LEFT, INPUT);
    pinMode(IR_SENSOR_RIGHT, INPUT);
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

void goToTable(int target_row, bool on_left)
{

    while (!destination_reached)
    {
        int line_detected_left = digitalRead(LEFT_TRA);
        int line_detected_center = digitalRead(CENTER_TRA);
        int line_detected_right = digitalRead(RIGHT_TRA);
        if (!line_detected_left && !line_detected_center && !line_detected_right)
        {
            Serial.println("stopped");
            // Stop if the line no longer exists.
            stopMotors();
            destination_reached = true;
        }
        else if (!line_detected_left && !line_detected_center && line_detected_right)
        {
            Serial.println("right (no center)");
            // The line is to the right
            turnRight(0, TURN_SPEED);
        }
        else if (!line_detected_left && line_detected_center && !line_detected_right)
        {
            Serial.println("forward");
            // The line is at the center
            moveForward(0, MOVEMENT_SPEED);
        }
        else if (!line_detected_left && line_detected_center && line_detected_right)
        {
            Serial.println("right");
            turnRight(0, TURN_SPEED);
        }
        else if (line_detected_left && !line_detected_center && !line_detected_right)
        {
            Serial.println("left (no center)");
            turnLeft(0, TURN_SPEED);
        }
        else if (line_detected_left && !line_detected_center && line_detected_right)
        {
            Serial.println("no center");
        }
        else if (line_detected_left && line_detected_center && !line_detected_right)
        {
            Serial.println("left");
            turnLeft(0, TURN_SPEED);
        }
        else if (line_detected_left && line_detected_center && line_detected_right)
        {
            Serial.println("forward (WIP)"); // TODO
            moveForward(0, MOVEMENT_SPEED);
        }
        else
        {
            Serial.println("ERROR: Unknown state!");
        }
    }

    Serial.println("Destination reached!");
}

void loop()
{
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
        goToTable(1, false);
        break;

    case IR_KEYCODE_2:
        goToTable(1, true);
        break;

    case IR_KEYCODE_3:
        goToTable(2, false);
        break;

    case IR_KEYCODE_4:
        goToTable(2, true);
        break;

    case IR_KEYCODE_5:
        goToTable(3, false);
        break;

    case IR_KEYCODE_6:
        goToTable(3, true);
        break;

    default:
        // Serial.println("ERROR: Unknown IR command.");
        break;
    }
}