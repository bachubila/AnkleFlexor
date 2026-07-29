#include <Arduino.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h>

#define LCD_ADDRESS 0x27
#define LCD_COLUMNS 20
#define LCD_ROWS 4

#define ENCODER_CLK 3
#define ENCODER_DT 4
#define ENCODER_SW 5

#define SERVO_PIN 6
#define BUZZER_PIN 7
#define LED_PIN 2
#define RESET_PIN 8
#define SERVO_CENTER 90

enum Screen {
  BOOT,
  MAIN_MENU,
  MANUAL_MODE,
  THERAPY_SET_MIN,
  THERAPY_SET_MAX,
  THERAPY_MODE,
  THERAPY_PAUSED
};

LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

void displayInit() {
  lcd.init();
  lcd.backlight();
}

void showGreeting() {
  lcd.clear();
  lcd.print("Ankle Flexor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
}

void drawCalibrationWarning() {
  lcd.clear();
  lcd.print("!! WARNING !!");
  lcd.setCursor(0, 1);
  lcd.print("Do NOT place");
  lcd.setCursor(0, 2);
  lcd.print("patient's foot");
  lcd.setCursor(0, 3);
  lcd.print("Click to continue");
}

void drawCalibrating() {
  lcd.clear();
  lcd.print("Calibrating...");
  lcd.setCursor(0, 1);
  lcd.print("Keep foot clear");
}

void drawMainMenu(int selected) {
  lcd.clear();
  lcd.print("Select Mode");
  lcd.setCursor(0, 1);
  if (selected == 0)
    lcd.print("> Manual");
  else
    lcd.print("  Manual");
  lcd.setCursor(0, 2);
  if (selected == 1)
    lcd.print("> Therapy");
  else
    lcd.print("  Therapy");
}

void drawManual(int angle) {
  lcd.clear();
  lcd.print("Manual Mode");
  lcd.setCursor(0, 1);
  lcd.print("Angle: ");
  lcd.print(angle);
  lcd.setCursor(0, 2);
  lcd.print("Rotate to adjust");
  lcd.setCursor(0, 3);
  lcd.print("Click: exit");
}

void drawTherapySetMin(int value) {
  lcd.clear();
  lcd.print("Set Min Angle");
  lcd.setCursor(0, 1);
  lcd.print("Value: ");
  lcd.print(value);
  lcd.setCursor(0, 2);
  lcd.print("Rotate: adjust");
  lcd.setCursor(0, 3);
  lcd.print("Click: confirm");
}

void drawTherapySetMax(int value) {
  lcd.clear();
  lcd.print("Set Max Angle");
  lcd.setCursor(0, 1);
  lcd.print("Value: ");
  lcd.print(value);
  lcd.setCursor(0, 2);
  lcd.print("Rotate: adjust");
  lcd.setCursor(0, 3);
  lcd.print("Click: confirm");
}

void drawTherapy(int minVal, int maxVal, int current) {
  lcd.clear();
  lcd.print("Therapy Mode");
  lcd.setCursor(0, 1);
  lcd.print("Running...");
  lcd.setCursor(0, 2);
  lcd.print("Min: ");
  lcd.print(minVal);
  lcd.print(" Max: ");
  lcd.print(maxVal);
}

void drawPaused(int selected) {
  lcd.clear();
  lcd.print("Paused Therapy");
  lcd.setCursor(0, 1);
  if (selected == 0)
    lcd.print("> Resume");
  else
    lcd.print("  Resume");
  lcd.setCursor(0, 2);
  if (selected == 1)
    lcd.print("> Exit");
  else
    lcd.print("  Exit");
}

Encoder knob(ENCODER_CLK, ENCODER_DT);
long lastPosition = 0;
bool clicked = false;
int direction = 0;

void encoderInit() {
  pinMode(ENCODER_SW, INPUT_PULLUP);
}

void encoderUpdate() {
  long pos = knob.read() / 4;
  direction = 0;
  if (pos > lastPosition)
    direction = 1;
  if (pos < lastPosition)
    direction = -1;
  lastPosition = pos;
  if (digitalRead(ENCODER_SW) == LOW) {
    clicked = true;
    delay(150);
  }
}

int getEncoderDirection() {
  int temp = direction;
  direction = 0;
  return temp;
}

bool encoderClicked() {
  if (clicked) {
    clicked = false;
    return true;
  }
  return false;
}

int selectedMode = 0;
int therapyMin = 60;
int therapyMax = 100;
int manualAngle = 90;
int therapyAngle = SERVO_CENTER;
int therapyDirection = 1;
unsigned long lastTherapyMove = 0;

void handleMainMenu() {
  int move = getEncoderDirection();
  if (move) {
    selectedMode += move;
    if (selectedMode < 0)
      selectedMode = 1;
    if (selectedMode > 1)
      selectedMode = 0;
    drawMainMenu(selectedMode);
  }
  if (encoderClicked()) {
    if (selectedMode == 0) {
      currentScreen = MANUAL_MODE;
      drawManual(manualAngle);
    } else {
      currentScreen = THERAPY_SET_MIN;
      drawTherapySetMin(therapyMin);
    }
  }
}

void handleManualMode() {
  int move = getEncoderDirection();
  if (move) {
    manualAngle += move;
    if (manualAngle < 0)
      manualAngle = 0;
    if (manualAngle > 180)
      manualAngle = 180;
    if (manualAngle < 50 || manualAngle > 110) {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
    }
    drawManual(manualAngle);
  }
  if (encoderClicked()) {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    currentScreen = MAIN_MENU;
    selectedMode = 0;
    manualAngle = 90;
    drawMainMenu(selectedMode);
  }
}

void handleTherapySetMin() {
  int move = getEncoderDirection();
  if (move) {
    therapyMin += move * 5;
    if (therapyMin < 50)
      therapyMin = 50;
    if (therapyMin > 70)
      therapyMin = 70;
    drawTherapySetMin(therapyMin);
  }
  if (encoderClicked()) {
    currentScreen = THERAPY_SET_MAX;
    drawTherapySetMax(therapyMax);
  }
}

void handleTherapySetMax() {
  int move = getEncoderDirection();
  if (move) {
    therapyMax += move * 5;
    if (therapyMax < 95)
      therapyMax = 95;
    if (therapyMax > 110)
      therapyMax = 110;
    drawTherapySetMax(therapyMax);
  }
  if (encoderClicked()) {
    currentScreen = THERAPY_MODE;
    therapyAngle = SERVO_CENTER;
    therapyDirection = 1;
    lastTherapyMove = millis();
    drawTherapy(therapyMin, therapyMax, therapyAngle);
  }
}

void handleTherapyMode() {
  if (millis() - lastTherapyMove >= 15) {
    lastTherapyMove = millis();
    therapyAngle += therapyDirection;
    if (therapyAngle >= therapyMax)
      therapyDirection = -1;
    if (therapyAngle <= therapyMin)
      therapyDirection = 1;
    servo.write(therapyAngle);
    if (therapyAngle < 50 || therapyAngle > 110) {
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
    } else {
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
    }
  }
  if (encoderClicked()) {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    currentScreen = THERAPY_PAUSED;
    selectedMode = 0;
    drawPaused(selectedMode);
  }
}

void handlePauseMenu() {
  int move = getEncoderDirection();
  if (move) {
    selectedMode += move;
    if (selectedMode < 0)
      selectedMode = 1;
    if (selectedMode > 1)
      selectedMode = 0;
    drawPaused(selectedMode);
  }
  if (encoderClicked()) {
    if (selectedMode == 0) {
      currentScreen = THERAPY_MODE;
      drawTherapy(therapyMin, therapyMax, therapyAngle);
    } else {
      currentScreen = MAIN_MENU;
      selectedMode = 0;
      drawMainMenu(selectedMode);
    }
  }
}

Servo servo;
Screen currentScreen = BOOT;
void (*resetFunc)(void) = 0;

void setup() {
  displayInit();
  encoderInit();
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(RESET_PIN, INPUT_PULLUP);
  showGreeting();
  delay(2000);
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  drawCalibrationWarning();
  while (!encoderClicked()) {
    encoderUpdate();
  }
  drawCalibrating();
  servo.attach(SERVO_PIN);
  servo.write(SERVO_CENTER);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
  currentScreen = MAIN_MENU;
  drawMainMenu(0);
}

void loop() {
  if (digitalRead(RESET_PIN) == LOW)
    resetFunc();
  encoderUpdate();
  switch (currentScreen) {
    case MAIN_MENU:
      handleMainMenu();
      break;
    case MANUAL_MODE:
      handleManualMode();
      break;
    case THERAPY_SET_MIN:
      handleTherapySetMin();
      break;
    case THERAPY_SET_MAX:
      handleTherapySetMax();
      break;
    case THERAPY_MODE:
      handleTherapyMode();
      break;
    case THERAPY_PAUSED:
      handlePauseMenu();
      break;
    default:
      break;
  }
}
