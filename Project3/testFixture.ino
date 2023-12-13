#include <LiquidCrystal.h>

#define LCD_D7 13
#define LCD_D6 12
#define LCD_D5 11
#define LCD_D4 10
#define LCD_ENABLE 32
#define LCD_READ 28

#define FULL 255
#define THREE_QUARTER 198
#define HALF 255/2

// Define LCD pins
LiquidCrystal lcd(LCD_READ, LCD_ENABLE, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// Motor control pins
#define ENABLE 5
#define DIRA 3
#define DIRB 4

int i;

void setup() {
  // Set pin direction
  pinMode(ENABLE, OUTPUT);
  pinMode(DIRA, OUTPUT);
  pinMode(DIRB, OUTPUT);

  // Initialize LCD
  lcd.begin(16, 2);
  lcd.clear();

  Serial.begin(9600);
}

void loop() {
  lcd.clear();

  // Back and forth example
  lcd.print("One way, then reverse");
  lcd.clear();
  digitalWrite(ENABLE, HIGH); // enable on
  for (i = 0; i < 5; i++) {
    digitalWrite(DIRA, HIGH); // one way
    digitalWrite(DIRB, LOW);
    lcd.setCursor(0, 0);
    lcd.print("Dir: Forward");
    lcd.setCursor(1, 2);
    lcd.print("Speed: Full");
    delay(5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dir: Forward");
    lcd.setCursor(1, 2);
    analogWrite(ENABLE, THREE_QUARTER);
    lcd.print("Speed: 3/4");
    delay(5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dir: Forward");
    lcd.setCursor(1, 2);
    analogWrite(ENABLE, HALF);
    lcd.print("Speed: Half");
    delay(5000);

    lcd.clear();
    digitalWrite(DIRA, LOW);  // reverse
    digitalWrite(DIRB, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Dir: Reverse");
    lcd.setCursor(1, 2);
    lcd.print("Speed: Full");
    delay(5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dir: Reverse");
    lcd.setCursor(1, 2);
    analogWrite(ENABLE, THREE_QUARTER);
    lcd.print("Speed: 3/4");
    delay(5000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dir: Reverse");
    lcd.setCursor(1, 2);
    analogWrite(ENABLE, HALF);
    lcd.print("Speed: Half");
    delay(5000);
  }
}
