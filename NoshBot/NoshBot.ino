#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include "IR_remote.h"
#include "keymap.h"

// Constants and PIN assignments

#define BAUDRATE 9600
#define OVERRIDE_OBSTACLE_DETECTION false

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
#define TRIGPIN 12 
#define ECHOPIN 13
#define SERVO 10

#define MOVEMENT_SPEED 80
#define TURN_SPEED 50

LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo MYSERVO;
IRremote ir(PIN_IR_REMOTE);

bool on_left;
int target_row;
int current_row = 0;

// bool at_home = true;
bool destination_reached = false;

bool obstacle_detected = false;

unsigned long previousMillis = 0;        // will store last time servo was updated
const long interval = 15;                // interval for servo movement (milliseconds)
int angle = 0;                           // initial angle for servo
bool increasing = true;                  // direction flag for servo


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
    pinMode(TRIGPIN, OUTPUT);
    pinMode(ECHOPIN, INPUT);
    
    MYSERVO.attach(SERVO);
    Serial.println("Ready.");
    LCD_Display();
}

/* checks obstacles */
bool checkObstacle() {
  long duration, distance;
  
  digitalWrite(TRIGPIN, LOW); 
  delayMicroseconds(2); 
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10); 
  digitalWrite(TRIGPIN, LOW);
  
  duration = pulseIn(ECHOPIN, HIGH);
  distance = (duration / 2) / 29.1; // Calculate distance in cm
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  if (distance < 20 && distance > 0) {
    return true; // Obstacle detected within 20cm
  } else {
    return false; // No obstacle detected or out of range
  }
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
                moveForward(300, MOVEMENT_SPEED); // put the intersection under the bot
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
        while ((obstacle_detected_left || obstacle_detected_right) && !OVERRIDE_OBSTACLE_DETECTION)
        {
            Serial.println("Obstacle detected.");
            stopMotors();
            delay(100);

            obstacle_detected_left = !digitalRead(IR_SENSOR_LEFT);
            obstacle_detected_left = !digitalRead(IR_SENSOR_RIGHT);

            continue;
        }

        while (checkObstacle() && !OVERRIDE_OBSTACLE_DETECTION)
        {
            Serial.println("Obstacle detected. Waiting...");
            stopMotors();
            delay(100);

            continue;
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
                moveForward(300, MOVEMENT_SPEED); // put the intersection under the bot
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
 
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("NoshBot is Home!");
    
    delay(2000);
}    



void loop()
    {

        unsigned long currentMillis = millis();

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
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Serving Table 1!");
            break;

        case IR_KEYCODE_2:
            goToTable(1, true);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Serving Table 2!");
            break;

        case IR_KEYCODE_3:
            goToTable(2, false);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Serving Table 3!");
            break;

        case IR_KEYCODE_4:
            goToTable(2, true);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Serving Table 4!");
            break;

        case IR_KEYCODE_5:
            goToTable(3, false);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Serving Table 5!");
            break;

        case IR_KEYCODE_6:
            goToTable(3, true);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Serving Table 6!");
            break;

        case IR_KEYCODE_9:
            returnHome();
            break;

        default:
            // Serial.println("ERROR: Unknown IR command.");
            break;
        }

        // Non-blocking ultrasonic sensor scanning
        if (currentMillis - previousMillis >= interval) {
            previousMillis = currentMillis;

            if (increasing) {
                angle++;
                if (angle >= 180) {
                    increasing = false;
                }
            } else {
                angle--;
                if (angle <= 0) {
                    increasing = true;
                }
            }

            MYSERVO.write(angle);

            // Check for obstacles
            if (checkObstacle()) {
                // If obstacle detected, stop motors
                stopMotors();

                // Wait for obstacle to clear
                while (checkObstacle()) {
                    delay(100);
                    // Check for remote commands while waiting
                    ir_command = ir.getIrKey(ir.getCode(), 1);
                    if (ir_command == IR_KEYCODE_OK) {
                        stopMotors();
                        break;
                    }
                }
                // Resume normal scanning after obstacle is cleared
                MYSERVO.write(angle);
            }
        }

    }


/* option for ultrasonic sensor */
/* void loop() {
  long duration, distance;
  
  // Rotate the servo from left to right continuously
  for (int angle = 0; angle <= 180; angle++) {
    MYSERVO.write(angle);
    delay(15); // Adjust speed of servo rotation here
    // Check for obstacles
    if (checkObstacle()) {
      // If obstacle detected, focus on it
      while (checkObstacle()) {
        delay(100); // Wait for obstacle to clear
      }
      break; // Exit the for loop to resume normal scanning
    }
  }
  
  for (int angle = 180; angle >= 0; angle--) {
    MYSERVO.write(angle);
    delay(15); // Adjust speed of servo rotation here
    // Check for obstacles
    if (checkObstacle()) {
      // If obstacle detected, focus on it
      while (checkObstacle()) {
        delay(100); // Wait for obstacle to clear
      }
      break; // Exit the for loop to resume normal scanning
    }
  }
}

// Function to check for obstacle using ultrasonic sensor
bool checkObstacle() {
  long duration, distance;
  
  digitalWrite(TRIGPIN, LOW); 
  delayMicroseconds(2); 
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10); 
  digitalWrite(TRIGPIN, LOW);
  
  duration = pulseIn(echoPin, HIGH);
  distance = (duration / 2) / 29.1; // Calculate distance in cm
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  
  if (distance < 20 && distance > 0) {
    return true; // Obstacle detected within 20cm
  } else {
    return false; // No obstacle detected or out of range
  }
} */