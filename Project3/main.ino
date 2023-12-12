#include <Wire.h>
#include <RTClib.h>
#include <arduinoFFT.h>
#include <LiquidCrystal.h>

RTC_DS1307 rtc;
arduinoFFT FFT = arduinoFFT();

// Define pins for DC motor using L293D
#define MOTOR_ENABLE_PIN 5
#define MOTOR_PIN1 4
#define MOTOR_PIN2 3

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

  if (!rtc.isrunning()) {  // Check if RTC lost power
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Set up LCD
  lcd.begin(16, 2);

  // Set up timer interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), updateInfoISR, RISING);
}

void loop() {
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

void updateFanInfo() {
  lcd.setCursor(0, 1);
  lcd.print("Fan: ");
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
  switch (fanSpeed) {
    case 0:
      return "0";
    case 1:
      return "1/2";
    case 2:
      return "3/4";
    case 3:
      return "FULL";
    default:
      return "UNKNOWN";
  }
}

String getRotationDirection() {
  if (fanSpeed > 0) {
    return "C";
  } else if (fanSpeed < 0) {
    return "CC";
  } else {
    return "S";
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
