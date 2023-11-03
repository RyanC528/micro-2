#include <Wire.h>

#define X_pin A11
#define Y_pin A10

// Joystick pins
const int SW_pin = 2;   // Digital pin connected to the joystick switch output
const int buttonPin = 13;
const int buzzerPin = 9;

bool appleEaten = false;

// MPU-6050 variables
const int MPU_addr = 0x68;  // I2C address of the MPU-6050
int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;
int joystickX = 0;
int joystickY = 0;
int button = 0;
int control = 0;

void setup() {
  Wire.begin();  // Initialize I2C communication
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // Set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize built-in LED pin
  digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED on (HIGH is the voltage level)
  pinMode(buzzerPin, OUTPUT);
  pinMode(SW_pin, INPUT);
  pinMode(buttonPin, OUTPUT);
}

void loop() {
  // Read from the joystick:
  joystickX = analogRead(X_pin);
  joystickY = analogRead(Y_pin);
  int joystickSwitch = digitalRead(SW_pin);

  button = digitalRead(buttonPin);

  if(button == HIGH){
    control = 1 - control;
    delay(100);
  }

  // Read from the MPU-6050 sensor:
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // Starting with register 0x43 (GYRO_XOUT_H)
  Wire.requestFrom(MPU_addr, 14, true);  // Request 2 registers
  AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  Wire.endTransmission(false);
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)

  // Use joystick or MPU-6050 to control the Snake's movement
  if (control == 0) {  // Joystick switch is pressed
    // Handle joystick input to control the Snake's direction
    if (joystickX < 200) {
      // Handle left movement
      Serial.write("a");
    } if (joystickX > 800) {
      // Handle right movement
      Serial.write("d");
    } if (joystickY < 200) {
      // Handle up movement
      Serial.write("w");
    } if (joystickY > 800) {
      // Handle down movement
      Serial.write("s");

    }

  } else {
    // Handle MPU-6050 input to control the Snake's direction
    if (AcY > 1000 || AcX > 1000){
      if (GyX > 1000) {
        Serial.write("a");
      } 
      if (GyX < -1000) {
        Serial.write("d");
      }
      if (GyY < 1000) {
        Serial.write("w");
      }
      if (GyY < -1000) {
        Serial.write("s");
      }
    delay(100);
 }
}

  delay(100);

  if (Serial.available()) {
    // Read the incoming character
    char incomingChar = Serial.read();

    // Check if the incoming character is 'E'
    if (incomingChar == 'E') {
      // Set the flag to indicate an apple is eaten
      appleEaten = true;
    }
  }
  // Check if the snake ate an apple
  if (appleEaten) {
    digitalWrite(buzzerPin, HIGH);
    delay(100);
    digitalWrite(buzzerPin, LOW);
  }
  
  appleEaten = false;
}
