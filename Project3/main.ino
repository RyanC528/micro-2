#include <Wire.h>
#include <RTClib.h>
#include <arduinoFFT.h>
#include <LiquidCrystal.h>

RTC_DS3231 rtc;
arduinoFFT FFT = arduinoFFT();

// Define LCD pins
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

// Define pins for DC motor using L293D
#define MOTOR_ENABLE_PIN 5
#define MOTOR_PIN1 4
#define MOTOR_PIN2 3

#define SOUND_SENSOR_PIN A0
#define BUTTON_PIN 2 // Example button pin, change as needed

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

// Global variables
volatile bool fanRunning = false;
volatile int fanSpeed = STOPPED;

void setup() {
  Serial.begin(9600);

  // Set up DC motor using L293D
  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  stopMotor();

  // Set up RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Set up LCD
  lcd.begin(16, 2);

  // Set up timer interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), updateInfoISR, RISING);
}

void loop() {
  // Main loop tasks...

  // Read the value from the sound sensor analog pin
  int soundSensorValue = analogRead(SOUND_SENSOR_PIN);

  // Print the sound sensor value to the LCD for debugging
  lcd.clear();
  lcd.print("Sound Sensor Value:");
  lcd.setCursor(0, 1);
  lcd.print(soundSensorValue);

  // Use the sound sensor value to adjust fan speed or perform FFT processing
  // Example: If soundSensorValue exceeds a certain threshold, increase fan speed
  if (soundSensorValue > SOUND_THRESHOLD) {
    // Perform FFT analysis and identify the peak frequency
    double frequencies[SAMPLES];
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute();
    FFT.MajorPeak(frequencies, SAMPLES, SAMPLING_FREQUENCY);

    // Check if the peak frequency matches the target notes with the allowed error
    if (isInRange(frequencies[0], TARGET_NOTE_C4)) {
      increaseFanSpeed();
    } else if (isInRange(frequencies[0], TARGET_NOTE_A4)) {
      decreaseFanSpeed();
    }
  }

  // Other main loop tasks...
}

void updateInfoISR() {
  DateTime now = rtc.now();

  // Display the time on the LCD
  lcd.clear();
  lcd.print("Time:");
  lcd.setCursor(0, 1);
  lcd.print(now.hour(), DEC);
  lcd.print(':');
  lcd.print(now.minute(), DEC);
  lcd.print(':');
  lcd.print(now.second(), DEC);

  // Other information update tasks...
}

void stopMotor() {
  digitalWrite(MOTOR_PIN1, LOW);
  digitalWrite(MOTOR_PIN2, LOW);
  analogWrite(MOTOR_ENABLE_PIN, 0);
}

void increaseFanSpeed() {
  if (fanSpeed < 3) {
    fanSpeed++;
    updateMotorSpeed();
  }
}

void decreaseFanSpeed() {
  if (fanSpeed > 0) {
    fanSpeed--;
    updateMotorSpeed();
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
}

void runMotorHalfSpeed() {
  digitalWrite(MOTOR_PIN1, HIGH);
  digitalWrite(MOTOR_PIN2, LOW);
  analogWrite(MOTOR_ENABLE_PIN, HALF_SPEED);
}

void runMotorThreeQuarterSpeed() {
  digitalWrite(MOTOR_PIN1, LOW);
  digitalWrite(MOTOR_PIN2, HIGH);
  analogWrite(MOTOR_ENABLE_PIN, THREE_QUARTER_SPEED);
}

void runMotorFullSpeed() {
  digitalWrite(MOTOR_PIN1, HIGH);
  digitalWrite(MOTOR_PIN2, HIGH);
  analogWrite(MOTOR_ENABLE_PIN, FULL_SPEED);
}

// Function to check if a value is within a specified range with an allowed error
bool isInRange(double value, double target) {
  double lowerBound = target - (target * FREQUENCY_ERROR);
  double upperBound = target + (target * FREQUENCY_ERROR);
  return (value >= lowerBound && value <= upperBound);
}
