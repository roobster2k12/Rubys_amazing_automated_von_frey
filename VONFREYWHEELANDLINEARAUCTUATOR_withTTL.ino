#include <Servo.h>

#define IN1 8
#define IN2 9
#define IN3 10
#define IN4 11
#define SERVO_PIN 6
#define TTL_PIN 12

const int POSITIONS = 8;
const int STEPS_PER_POSITION = 512;
int currentPosition = 0;
int pos = 0;

Servo myservo;

const int coilSequence[8][4] = {
  {1, 0, 0, 0},
  {1, 1, 0, 0},
  {0, 1, 0, 0},
  {0, 1, 1, 0},
  {0, 0, 1, 0},
  {0, 0, 1, 1},
  {0, 0, 0, 1},
  {1, 0, 0, 1}
};

void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(TTL_PIN, OUTPUT);
  digitalWrite(TTL_PIN, LOW);
  
  myservo.attach(SERVO_PIN);
  myservo.write(0);
  delay(500);
  
  Serial.begin(9600);
  Serial.println("Stepper + Servo + TTL Ready!");
  printHelp();
}

void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    
    // STEPPER COMMANDS
    if (command >= '0' && command <= '7') {
      int targetPosition = command - '0';
      moveToPosition(targetPosition);
    }
    else if (command == 'n') moveToPosition((currentPosition + 1) % POSITIONS);
    else if (command == 'p') moveToPosition((currentPosition - 1 + POSITIONS) % POSITIONS);
    
    // SERVO SIMPLE COMMANDS
    else if (command == 'e') servoExtend();
    else if (command == 'r') servoRetract();
    else if (command == 'm') servoMiddle();
    
    // SERVO SEQUENCE A
    else if (command == 'A' || command == 'a') runSequenceA();
    
    else if (command == 'h') printHelp();
  }
}

// ===== STEPPER MOTOR FUNCTIONS =====
void moveToPosition(int target) {
  int stepsNeeded = (target - currentPosition) * STEPS_PER_POSITION;
  
  if (stepsNeeded > POSITIONS * STEPS_PER_POSITION / 2) {
    stepsNeeded -= POSITIONS * STEPS_PER_POSITION;
  }
  else if (stepsNeeded < -POSITIONS * STEPS_PER_POSITION / 2) {
    stepsNeeded += POSITIONS * STEPS_PER_POSITION;
  }
  
  int direction = (stepsNeeded > 0) ? 1 : -1;
  
  for (int i = 0; i < abs(stepsNeeded); i++) {
    stepMotor(direction);
    delayMicroseconds(2500);
  }
  
  currentPosition = target;
  disableMotor();
  Serial.print("Stepper Position: ");
  Serial.println(target);
}

void stepMotor(int direction) {
  static int stepIndex = 0;
  
  stepIndex += direction;
  if (stepIndex < 0) stepIndex = 7;
  if (stepIndex > 7) stepIndex = 0;
  
  digitalWrite(IN1, coilSequence[stepIndex][0] * 255);
  digitalWrite(IN2, coilSequence[stepIndex][1] * 255);
  digitalWrite(IN3, coilSequence[stepIndex][2] * 255);
  digitalWrite(IN4, coilSequence[stepIndex][3] * 255);
}

void disableMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ===== SERVO FUNCTIONS =====
void servoExtend() {
  myservo.write(180);
  Serial.println("Servo: Extended");
  delay(500);
}

void servoRetract() {
  myservo.write(0);
  Serial.println("Servo: Retracted");
  delay(500);
}

void servoMiddle() {
  myservo.write(90);
  Serial.println("Servo: Middle");
  delay(500);
}

// ===== SEQUENCE A =====
void runSequenceA() {
  Serial.println("Running Sequence A");
  
  // TTL GOES HIGH
  digitalWrite(TTL_PIN, HIGH);
  Serial.println("TTL: HIGH");
  
  // ROTATE FORWARD (0° to 180°)
  for (pos = 0; pos <= 180; pos += 1) { 
    myservo.write(pos);
    delay(2);
  }
  
  // DELAY 2 SECONDS (TTL still HIGH)
  delay(2000);
  
  // ROTATE BACKWARD (180° to 0°)
  for (pos = 180; pos >= 0; pos -= 1) { 
    myservo.write(pos);
    delay(2);
  }
  
  // TTL GOES LOW AFTER RETRACT COMPLETE
  digitalWrite(TTL_PIN, LOW);
  Serial.println("TTL: LOW");
  
  // STOP
  myservo.write(0);
  Serial.println("Sequence A complete\n");
}

void printHelp() {
  Serial.println("\n=== STEPPER COMMANDS ===");
  Serial.println("0-7: Go to position");
  Serial.println("n: Next position");
  Serial.println("p: Previous position");
  
  Serial.println("\n=== SERVO COMMANDS ===");
  Serial.println("e: Extend");
  Serial.println("r: Retract");
  Serial.println("m: Middle");
  Serial.println("A: Sequence A (TTL HIGH until retract complete)");
  
  Serial.println("\nh: Help\n");
} 