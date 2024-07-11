#include <Servo.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 4);

#include "IR_remote.h"
#include "keymap.h"


#define BAUDRATE 9600
#define OVERRIDE_OBSTACLE_DETECTION true

// PIN assignments
#define PIN_IR_REMOTE 3
#define IR_SENSOR_LEFT A1
#define IR_SENSOR_RIGHT A2
#define MOTOR_LEFT_FORWARD 2
#define MOTOR_LEFT_PWM 5
#define MOTOR_RIGHT_FORWARD 4
#define MOTOR_RIGHT_PWM 6
#define LEFT_TRA 7
#define CENTER_TRA 8
#define RIGHT_TRA 9
#define BUTTON 13

#define MOVEMENT_SPEED 80
#define TURN_SPEED 50

bool on_left;
int target_row;
int current_row = 0;

// bool at_home = true;
bool destination_reached = false;
IRremote ir(PIN_IR_REMOTE);

void LCD_Display(){
    lcd.init();
    lcd.backlight();
}

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
    pinMode(BUTTON, INPUT);
    Serial.println("Ready.");

    LCD_Display();
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

/* Move NoshBot to the chosen Table */
void goToTable(int target_row, bool on_left)
{
    int current_row = 0;

    Serial.println("goToTable started...");

    bool turn_phase_1 = false;
    bool turn_phase_2 = false;
    bool intersection_already_registered = false;

    while (!destination_reached)
    {
        int line_detected_left = digitalRead(LEFT_TRA);
        int line_detected_center = digitalRead(CENTER_TRA);
        int line_detected_right = digitalRead(RIGHT_TRA);

        // for some reason, it's 1 when it's clear?
        int obstacle_detected_left = !digitalRead(IR_SENSOR_LEFT);
        int obstacle_detected_right = !digitalRead(IR_SENSOR_RIGHT);

        byte ir_command = ir.getIrKey(ir.getCode(), 1);

        if (ir_command == IR_KEYCODE_OK)
        {
            Serial.println("Force stopped.");
            stopMotors();
            break;
        }

        // stop at all costs to prevent collision
        if ((obstacle_detected_left || obstacle_detected_right) && !OVERRIDE_OBSTACLE_DETECTION)
        {
            Serial.println("Obstacle detected.");
            stopMotors();
        }

        // Phase 1: keep turning left/right until center and opposite sensor declares false
        if (turn_phase_1)
        {
            if (on_left)
            {
                Serial.println("left (phase 1)");
                turnLeft(50, TURN_SPEED);
                if (!line_detected_center && !line_detected_right)
                {
                    turn_phase_1 = false;
                    turn_phase_2 = true;
                }
            }
            else
            {
                Serial.println("right (phase 1)");
                turnRight(50, TURN_SPEED);
                if (!line_detected_center && !line_detected_left)
                {
                    turn_phase_1 = false;
                    turn_phase_2 = true;
                }
            }
            continue;
        }

        // Phase 2: keep turning left/right until center declares true
        else if (turn_phase_2)
        {
            if (on_left)
            {
                Serial.println("left (phase 2)");
                turnLeft(0, TURN_SPEED);
                if (line_detected_center && line_detected_right)
                    turn_phase_2 = false;
            }
            else
            {
                Serial.println("right (phase 2)");
                turnRight(0, TURN_SPEED);
                if (line_detected_center && line_detected_left)
                    turn_phase_2 = false;
            }
            continue;
        }

        // We have passed the intersection, we can now return this to false.
        if (!(line_detected_left && line_detected_center && line_detected_right))
            intersection_already_registered = false;

        // Automated driving
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
        else if (line_detected_left && !line_detected_center && line_detected_right)
        {
            Serial.println("no center");
        }
        else if (line_detected_left && line_detected_center && !line_detected_right)
        {
            Serial.println("left");
            turnLeft(0, TURN_SPEED);
        }
        else if (line_detected_left && !line_detected_center && !line_detected_right)
        {
            Serial.println("left (no center)");
            turnLeft(0, TURN_SPEED);
        }
        else if (line_detected_left && line_detected_center && line_detected_right)
        {
            Serial.println(String(current_row) + " == " + String(target_row));
            if (intersection_already_registered)
                continue;
            intersection_already_registered = true; // Prevent increments from the same intersection

            if (++current_row == target_row)
            {
                turn_phase_1 = true;
                moveForward(400, MOVEMENT_SPEED); // put the intersection under the bot
                if (on_left)
                {
                    Serial.println("left (intersection)");
                    turnLeft(500, TURN_SPEED);
                }
                else
                {
                    Serial.println("right (intersection)");
                    turnRight(500, TURN_SPEED);
                }
            }
        }
        else
        {
            Serial.println("ERROR: Unknown state!");
        }
    }
    Serial.println("Destination reached!");
}

void perform_u_turn()
{
    bool line_detected = false;
    Serial.println("Performing U-Turn...");
    while (true)
    {
        byte ir_command = ir.getIrKey(ir.getCode(), 1);

        if (ir_command == IR_KEYCODE_OK)
        {
            Serial.println("Force stopped.");
            stopMotors();
            break;
        }

        turnRight(0, TURN_SPEED);
        if (digitalRead(CENTER_TRA) && digitalRead(LEFT_TRA))
            break;
    }
    stopMotors();
    Serial.println("U-Turn Done.");
}

/* Make NoshBot return to the starting point */
void returnHome()
{
    bool turn_phase_1 = false;
    bool turn_phase_2 = false;
    target_row = current_row;
    bool intersection_already_registered = false;

    Serial.println("NoshBot Going Home!");

    perform_u_turn();
    while (destination_reached)
    {
        int line_detected_left = digitalRead(LEFT_TRA);
        int line_detected_center = digitalRead(CENTER_TRA);
        int line_detected_right = digitalRead(RIGHT_TRA);

        int obstacle_detected_left = !digitalRead(IR_SENSOR_LEFT);
        int obstacle_detected_right = !digitalRead(IR_SENSOR_RIGHT);

        byte ir_command = ir.getIrKey(ir.getCode(), 1);

        if (ir_command == IR_KEYCODE_OK)
        {
            Serial.println("Force stopped.");
            stopMotors();
            break;
        }

        // stop at all costs to prevent collision
        if ((obstacle_detected_left || obstacle_detected_right) && !OVERRIDE_OBSTACLE_DETECTION)
        {
            Serial.println("Obstacle detected.");
            stopMotors();
        }

        // Phase 1: keep turning left/right until center and opposite sensor declares false
        if (turn_phase_1)
        {
            if (on_left)
            {
                Serial.println("left (phase 1)");
                turnRight(50, TURN_SPEED);
                if (!line_detected_center && !line_detected_right)
                {
                    turn_phase_1 = false;
                    turn_phase_2 = true;
                }
            }
            else
            {
                Serial.println("right (phase 1)");
                turnLeft(50, TURN_SPEED);
                if (!line_detected_center && !line_detected_left)
                {
                    turn_phase_1 = false;
                    turn_phase_2 = true;
                }
            }
            continue;
        }
        // Phase 2: keep turning left/right until center declares true
        else if (turn_phase_2)
        {
            if (on_left)
            {
                Serial.println("left (phase 2)");
                turnRight(0, TURN_SPEED);
                if (line_detected_center && line_detected_right)
                    turn_phase_2 = false;
            }
            else
            {
                Serial.println("right (phase 2)");
                turnLeft(0, TURN_SPEED);
                if (line_detected_center && line_detected_left)
                    turn_phase_2 = false;
            }
            continue;
        }

        // We have passed the intersection, we can now return this to false.
        if (!(line_detected_left && line_detected_center && line_detected_right))
            intersection_already_registered = false;

        // Automated driving
        if (!line_detected_left && !line_detected_center && !line_detected_right)
        {
            Serial.println("stopped");
            // Stop if the line no longer exists.
            stopMotors();
            destination_reached = true;
            break;
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
        else if (line_detected_left && !line_detected_center && line_detected_right)
        {
            Serial.println("no center");
        }
        else if (line_detected_left && line_detected_center && !line_detected_right)
        {
            Serial.println("left");
            turnLeft(0, TURN_SPEED);
        }
        else if (line_detected_left && !line_detected_center && !line_detected_right)
        {
            Serial.println("left (no center)");
            turnLeft(0, TURN_SPEED);
        }
        else if (line_detected_left && line_detected_center && line_detected_right)
        {
            Serial.println(String(current_row) + " == " + String(target_row));
            if (intersection_already_registered)
                continue;
            intersection_already_registered = true; // Prevent increments from the same intersection

            if (current_row-- == target_row)
            {
                turn_phase_1 = true;
                moveForward(400, MOVEMENT_SPEED); // put the intersection under the bot
                if (on_left)
                {
                    Serial.println("left (intersection)");
                    turnRight(500, TURN_SPEED);
                }
                else
                {
                    Serial.println("right (intersection)");
                    turnLeft(500, TURN_SPEED);
                }
            }
        }
        else
        {
            Serial.println("ERROR: Unknown state!");
        }
    }

    Serial.println("Destination reached!");
    destination_reached = false;
    Serial.println("Turning back...");
    moveForward(500, MOVEMENT_SPEED); // put the intersection under the bot
    perform_u_turn();
    Serial.println("Ready na ulit.");
 
    delay(5000);

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Nosh");
}

void loop()
{
    byte ir_command = ir.getIrKey(ir.getCode(), 1);

    switch (ir_command)
    {
    case IR_KEYCODE_UP: // Go forward.
        moveForward(0, 150);
        stopMotors();
        break;

    case IR_KEYCODE_DOWN: // Go backward.
        moveBackward(0, 150);
        stopMotors();
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

    case IR_KEYCODE_9:
        returnHome();
        break;

    default:
        // Serial.println("ERROR: Unknown IR command.");
        break;
    }
}