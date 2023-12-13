#include <Wire.h>
#include <RTClib.h>
#include <arduinoFFT.h>
#include <LiquidCrystal.h>

RTC_DS1307 rtc;
arduinoFFT FFT = arduinoFFT();

// Define pins for DC motor using L293D
#define ENABLE 5
#define DIRA 3
#define DIRB 4

#define SOUND_SENSOR_APIN A0
#define BUTTON_PIN 2 // Example button pin, change as needed

#define LCD_D7 13
#define LCD_D6 12
#define LCD_D5 11
#define LCD_D4 10
#define LCD_ENABLE 32
#define LCD_READ 28

// Motor speed levels
#define FULL_SPEED 255
#define THREE_QUARTER_SPEED 192
#define HALF_SPEED 128
#define STOPPED 0

// Sound sensor thresholds
#define SOUND_THRESHOLD 500
#define SOUND_THRESHOLD_LOW 400

// Frequency analysis parameters
#define SAMPLES 256
#define SAMPLING_FREQUENCY 1000
#define TARGET_NOTE_C4 262
#define TARGET_NOTE_A4 440
#define FREQUENCY_ERROR 0.02 // 2%

// Define LCD pins
LiquidCrystal lcd(LCD_READ, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Global variables
volatile bool fanRunning = false;
volatile int fanSpeed = STOPPED;
volatile bool fanClockwise = true; // Initial fan direction

void updateInfoISR();
bool isInRange(double value, double target);

void setup() {
  Serial.begin(9600);

  // Set up DC motor using L293D
  pinMode(ENABLE, OUTPUT);
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);

  // Set up RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {  // Check if RTC lost power
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Set up LCD
  lcd.begin(16, 2);

  // Set up timer interrupt
  attachInterrupt(digitalPinToInterrupt(2), updateInfoISR, RISING);
}

void loop() {
  // Check for serial input to change fan direction or speed
  if (Serial.available() > 0) {
    char input = Serial.read();
    processSerialInput(input);
  }

  // Read the value from the sound sensor analog pin
  int soundSensorValue = analogRead(SOUND_SENSOR_APIN);

  // Use the sound sensor value to adjust fan speed
  if (soundSensorValue > SOUND_THRESHOLD) {
    // Perform basic frequency analysis using analogRead values
    int frequency = map(soundSensorValue, 0, 1023, 20, 2000); // Map analogRead range to frequency range
    // Check if the frequency matches the target notes with the allowed error
    if (isInRange(frequency, TARGET_NOTE_C4)) {
      increaseFanSpeed();
    } else if (isInRange(frequency, TARGET_NOTE_A4)) {
      decreaseFanSpeed();
    }
  }

  // Update fan-related information and time on the LCD
  updateFanInfo();
  updateInfoISR();

  delay(1000); // Update information every second
}

void processSerialInput(char input) {
  switch (input) {
    case 'E':
    case 'e':
      analogWrite(ENABLE, HIGH);
      break;
    case 'C':
    case 'c':
      fanClockwise = true;  // Update fan direction to clockwise
      fanRunning = true;
      lcd.clear();
      updateFanDirection();
      updateInfoISR();
      digitalWrite(DIRA, LOW);
      digitalWrite(DIRB, HIGH);
      delay(1000);
      break;
    case 'R':
    case 'r':
      fanClockwise = false;  // Update fan direction to counterclockwise
      fanRunning = true;
      lcd.clear();
      updateFanDirection();
      updateInfoISR();
      digitalWrite(DIRA, HIGH);
      digitalWrite(DIRB, LOW);
      delay(1000);
      break;
    case '0':
      fanSpeed = STOPPED;
      fanRunning = false;
      lcd.clear();
      updateFanInfo();
      updateInfoISR();
      analogWrite(ENABLE, 0);
      delay(1000);
      break;
    case '1':
      fanSpeed = HALF_SPEED;
      fanRunning = true;
      lcd.clear();
      updateFanInfo();
      updateInfoISR();
      analogWrite(ENABLE, HALF_SPEED);
      delay(1000);
      break;
    case '2':
      fanSpeed = THREE_QUARTER_SPEED;
      fanRunning = true;
      lcd.clear();
      updateFanInfo();
      updateInfoISR();
      analogWrite(ENABLE, THREE_QUARTER_SPEED);
      delay(1000);
      break;
    case '3':
      fanSpeed = FULL_SPEED;
      fanRunning = true;
      lcd.clear();
      updateFanInfo();
      updateInfoISR();
      analogWrite(ENABLE, FULL_SPEED);
      delay(1000);
      break;
  }
}

void updateFanInfo() {
  lcd.setCursor(0, 1);
  lcd.print(getFanSpeedText());
  lcd.print(" ");
  lcd.print(getRotationDirection());
  lcd.print(" ");
  lcd.print(getFrequency());
}

String getFrequency() {
  // Read the value from the sound sensor analog pin
  int soundSensorValue = analogRead(SOUND_SENSOR_APIN);

  // Perform basic frequency analysis using analogRead values
  int frequency = map(soundSensorValue, 0, 1023, 20, 2000); // Map analogRead range to frequency range

  return String(frequency) + " Hz";
}

String getFanSpeedText() {
  if (fanSpeed == 0) {
    return "0";
  } else if (fanSpeed == HALF_SPEED) {
    return "1/2";
  } else if (fanSpeed == THREE_QUARTER_SPEED) {
    return "3/4";
  } else if (fanSpeed == FULL_SPEED) {
    return "FULL";
  } else {
    return "UNKNOWN";
  }
}

String getRotationDirection() {
  if (fanClockwise) {
    return "C";
  } else {
    return "CC";
  }
}

void updateInfoISR() {
  static unsigned long lastUpdateMillis = 0;
  const unsigned long updateInterval = 1000;  // Update interval in milliseconds

  unsigned long currentMillis = millis();

  // Check if the specified update interval has passed
  if (currentMillis - lastUpdateMillis >= updateInterval) {
    DateTime now = rtc.now();

    // Display the time on the LCD
    lcd.setCursor(0, 0);
    lcd.print("Time: ");
    lcd.print(now.hour(), DEC);
    lcd.print(':');
    lcd.print((now.minute() < 10) ? "0" : "");
    lcd.print(now.minute(), DEC);
    lcd.print(':');
    lcd.print((now.second() < 10) ? "0" : "");
    lcd.print(now.second(), DEC);

    // Update the last update time
    lastUpdateMillis = currentMillis;
  }
}

void stopMotor() {
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, LOW);
  analogWrite(ENABLE, 0);
}

void increaseFanSpeed() {
  if (fanSpeed < 3) {
    fanSpeed++;
    updateMotorSpeed();
    updateFanDirection();  // Add this line to update the fan direction
  }
}

void decreaseFanSpeed() {
  if (fanSpeed > 0) {
    fanSpeed--;
    updateMotorSpeed();
    updateFanDirection();  // Add this line to update the fan direction
  }
}

void updateMotorSpeed() {
  switch (fanSpeed) {
    case 0:
      stopMotor();
      break;
    case 1:
      runMotorHalfSpeed();
      break;
    case 2:
      runMotorThreeQuarterSpeed();
      break;
    case 3:
      runMotorFullSpeed();
      break;
  }
  updateFanDirection();  // Add this line to update the fan direction
}

void runMotorHalfSpeed() {
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, LOW);
  analogWrite(ENABLE, HALF_SPEED);
}

void runMotorThreeQuarterSpeed() {
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, HIGH);
  analogWrite(ENABLE, THREE_QUARTER_SPEED);
}

void runMotorFullSpeed() {
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, HIGH);
  analogWrite(ENABLE, FULL_SPEED);
}

void updateFanDirection() {
  if (fanClockwise) {
    digitalWrite(DIRA, HIGH);
    digitalWrite(DIRB, LOW);
    Serial.println("Fan Direction: Clockwise (C)");
  } else {
    digitalWrite(DIRA, LOW);
    digitalWrite(DIRB, HIGH);
    Serial.println("Fan Direction: Counterclockwise (CC)");
  }
}

// Function to check if a value is within a specified range with an allowed error
bool isInRange(double value, double target) {
  double lowerBound = target - (target * FREQUENCY_ERROR);
  double upperBound = target + (target * FREQUENCY_ERROR);
  return (value >= lowerBound && value <= upperBound);
}
