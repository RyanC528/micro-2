#include <Wire.h>
#include <DS3231.h>
#include <arduinoFFT.h>

// Define pins
#define MOTOR_PIN1 9
#define MOTOR_PIN2 10
#define MOTOR_ENABLE_PIN 8
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

// Initialize libraries
DS3231 clock;
RTCDateTime dt;
ArduinoFFT FFT = ArduinoFFT();

// Global variables
volatile bool fanRunning = false;
volatile int fanSpeed = STOPPED;

void setup() {
  Serial.begin(9600);

  // Set up DC motor using L293D
  pinMode(MOTOR_PIN1, OUTPUT);
  pinMode(MOTOR_PIN2, OUTPUT);
  pinMode(MOTOR_ENABLE_PIN, OUTPUT);
  stopMotor();

  // Set up RTC
  clock.begin();

  if (clock.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    clock.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Set up button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Set up timer interrupt
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), updateInfoISR, RISING);
}

void loop() {
  // Main loop tasks...

  // Read the current time from the RTC
  dt = clock.getDateTime();

  // Print the time to the serial monitor
  Serial.print("Time: ");
  Serial.print(dt.year);   Serial.print("-");
  Serial.print(dt.month);  Serial.print("-");
  Serial.print(dt.day);    Serial.print(" ");
  Serial.print(dt.hour);   Serial.print(":");
  Serial.print(dt.minute); Serial.print(":");
  Serial.println(dt.second);

  // Read the value from the sound sensor analog pin
  int soundSensorValue = analogRead(SOUND_SENSOR_PIN);

  // Print the sound sensor value to the serial monitor for debugging
  Serial.print("Sound Sensor Value: ");
  Serial.println(soundSensorValue);

  // Use the sound sensor value to adjust fan speed or perform FFT processing
  // Example: If soundSensorValue exceeds a certain threshold, increase fan speed
  if (soundSensorValue > SOUND_THRESHOLD) {
    // Perform FFT analysis and identify the peak frequency
    double real[SAMPLES];
    double imag[SAMPLES];
    FFT.Windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD, FFT_INVERT);
    FFT.Compute(soundSensorValue, SAMPLES, SAMPLING_FREQUENCY, real, imag);
    FFT.ComplexToMagnitude(real, imag, SAMPLES);
    int peak = FFT.MajorPeak(real, SAMPLES, SAMPLING_FREQUENCY);

    // Check if the peak frequency matches the target notes with the allowed error
    if (isInRange(peak, TARGET_NOTE_C4)) {
      increaseFanSpeed(); // Increase fan speed for "C4" note
    } else if (isInRange(peak, TARGET_NOTE_A4)) {
      decreaseFanSpeed(); // Decrease fan speed for "A4" note
    }
  }

  // Other main loop tasks...
  delay(1000);
}

void updateInfoISR() {
  // Update other information using the ISR if needed
}

void stopMotor() {
  digitalWrite(MOTOR_PIN1, LOW);
  digitalWrite(MOTOR_PIN2, LOW);
  analogWrite(MOTOR_ENABLE_PIN, 0);
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

bool isInRange(int value, int target) {
  int lowerBound = target - int(target * FREQUENCY_ERROR);
  int upperBound = target + int(target * FREQUENCY_ERROR);
  return (value >= lowerBound && value <= upperBound);
}