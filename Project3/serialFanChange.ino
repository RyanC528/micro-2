#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>

RTC_DS1307 rtc;

// Define pins for DC motor using L293D
#define ENABLE 5
#define DIRA 3
#define DIRB 4

#define LCD_D7 13
#define LCD_D6 12
#define LCD_D5 11
#define LCD_D4 10
#define LCD_ENABLE 32
#define LCD_READ 28

// Motor speed levels
#define FULL_SPEED 255
#define THREE_QUARTER_SPEED 198
#define HALF_SPEED 255 / 2
#define STOPPED 0

// Define LCD pins
LiquidCrystal lcd(LCD_READ, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Global variables
volatile bool fanRunning = false;
volatile int fanSpeed = STOPPED;
volatile bool fanClockwise = true; // Initial fan direction

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

  if (!rtc.isrunning()) { // Check if RTC lost power
    Serial.println("RTC lost power, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Set up LCD
  lcd.begin(16, 2);

  // Set initial fan direction
  updateFanDirection();

  // Set up timer interrupt
  attachInterrupt(digitalPinToInterrupt(2), updateInfoISR, RISING);
}

void loop() {
  // Check for serial input to change fan direction or speed
  while (Serial.available() > 0) {
    char input = Serial.read();
    if (input == 'E' || input == 'e'){
      analogWrite(ENABLE, HIGH);
    }
    else if (input == 'C' || input == 'c') {
      fanClockwise = true;
      lcd.clear();
      updateFanDirection();
      updateInfoISR();
      digitalWrite(DIRA, LOW);
      digitalWrite(DIRB, HIGH);
    } else if (input == 'R' || input == 'r') {
      fanClockwise = false;
      lcd.clear();
      updateFanDirection();
      updateInfoISR();
    } else if (input == '0') {
      fanSpeed = STOPPED;
      fanRunning = false;
      lcd.clear();
      updateFanInfo();
      updateInfoISR();
      analogWrite(ENABLE, 0);
    } else if (input == '1') {
      fanSpeed = HALF_SPEED;
      fanRunning = true;
      lcd.clear();
      updateFanInfo();
      updateInfoISR();
      analogWrite(ENABLE, HALF_SPEED);
    } else if (input == '2') {
      fanSpeed = THREE_QUARTER_SPEED;
      fanRunning = true;
      lcd.clear();
      updateFanInfo();
      updateInfoISR();
      analogWrite(ENABLE, THREE_QUARTER_SPEED);
    } else if (input == '3') {
      fanSpeed = FULL_SPEED;
      fanRunning = true;
      lcd.clear();
      updateFanInfo();
      updateInfoISR();
      analogWrite(ENABLE, FULL_SPEED);
    }
  }

  // Update fan-related information and time on the LCD
  updateFanInfo();
  updateInfoISR();

  delay(1000); // Update information every second
}

void updateFanInfo() {
  lcd.setCursor(0, 1);
  lcd.print(getFanSpeedText());
  lcd.print(" ");
  lcd.print(getRotationDirection());
}

String getFanSpeedText() {
  switch (fanSpeed) {
    case STOPPED:
      return "OFF";
    case HALF_SPEED:
      return "HALF";
    case THREE_QUARTER_SPEED:
      return "3/4 ";
    case FULL_SPEED:
      return "FULL";
    default:
      return "UNKNOWN";
  }
}

String getRotationDirection() {
  if (fanSpeed > 0) {
    return (fanClockwise ? "C" : "CC");
  } else {
    return "S";
  }
}

void updateInfoISR() {
  static unsigned long lastUpdateMillis = 0;
  const unsigned long updateInterval = 1000; // Update interval in milliseconds

  unsigned long currentMillis = millis();

  // Check if the specified update interval has passed
  if (currentMillis - lastUpdateMillis >= updateInterval) {
    DateTime now = rtc.now();

    // Display the time on the LCD
    lcd.setCursor(0, 0);
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

void updateFanDirection() {
  if (fanClockwise) {
    digitalWrite(DIRA, HIGH);
    digitalWrite(DIRB, LOW);
  } else {
    digitalWrite(DIRA, LOW);
    digitalWrite(DIRB, HIGH);
  }
}
