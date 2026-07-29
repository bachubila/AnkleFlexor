// ============================================================
// AnkleFlexor — Consolidated Firmware
// ============================================================
// This file merges all project sources (config.h, states.h,
// display.h/.cpp, encoder.h/.cpp, menu.h/.cpp, AnkleFlexor.ino)
// into a single compilation unit for ease of review and upload.
// ============================================================

// ── Library Includes ──────────────────────────────────────────
// Arduino.h       : core I/O, delay, millis, etc.
// Servo.h         : PWM servo control
// LiquidCrystal_I2C.h : I2C 20x4 LCD driver
// Encoder.h       : rotary encoder quadrature decoding
#include <Arduino.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <Encoder.h>

// ── Pin & Hardware Configuration ──────────────────────────────
// LCD: I2C address 0x27, 20 columns x 4 rows
// Encoder: CLK (interrupt-capable), DT, SW (push-button)
// Servo on pin 6, buzzer on 7, LED on 2, reset switch on 8
// SERVO_CENTER = 90° neutral position
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

// ── Finite-State Machine ──────────────────────────────────────
// BOOT            -> startup sequence
// MAIN_MENU       -> mode selection (Manual / Therapy)
// MANUAL_MODE     -> direct encoder-to-servo control
// THERAPY_SET_MIN -> user sets lower angle bound
// THERAPY_SET_MAX -> user sets upper angle bound
// THERAPY_MODE    -> automatic oscillation between bounds
// THERAPY_PAUSED  -> pause overlay (Resume / Exit)
enum Screen {
  BOOT,
  MAIN_MENU,
  MANUAL_MODE,
  THERAPY_SET_MIN,
  THERAPY_SET_MAX,
  THERAPY_MODE,
  THERAPY_PAUSED
};

// ── LCD Object ────────────────────────────────────────────────
// Global I2C LCD instance shared by all display functions.
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// ══════════════════════════════════════════════════════════════
// DISPLAY MODULE
// ══════════════════════════════════════════════════════════════
// Every draw*() function clears the LCD then renders a specific
// screen layout. The `selected` parameter highlights the current
// menu item with a '>' arrow.

// ── Initialisation ────────────────────────────────────────────
void displayInit() {
  lcd.init();
  lcd.backlight();
}

// ── Boot Greeting ─────────────────────────────────────────────
// Shown on power-up before the calibration step.
void showGreeting() {
  lcd.clear();
  lcd.print("Ankle Flexor");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
}

// ── Calibration Warning ───────────────────────────────────────
// Instructs the user to keep the foot clear before the servo
// homes to centre. Waits for a click to proceed.
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

// ── Calibrating ───────────────────────────────────────────────
// Displayed while the servo sweeps to centre.
void drawCalibrating() {
  lcd.clear();
  lcd.print("Calibrating...");
  lcd.setCursor(0, 1);
  lcd.print("Keep foot clear");
}

// ── Main Menu (Manual / Therapy) ──────────────────────────────
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

// ── Manual Mode Screen ────────────────────────────────────────
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

// ── Therapy: Set Minimum Angle ────────────────────────────────
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

// ── Therapy: Set Maximum Angle ────────────────────────────────
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

// ── Therapy Active ────────────────────────────────────────────
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

// ── Pause Menu (Resume / Exit) ────────────────────────────────
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

// ══════════════════════════════════════════════════════════════
// ENCODER MODULE
// ══════════════════════════════════════════════════════════════
// Reads a quadrature rotary encoder and a push-button switch.
// `knob.read()` returns a signed count; dividing by 4 gives
// one tick per detent. The direction is latched until consumed
// by getEncoderDirection(). The button is debounced with a
// 150 ms delay inside encoderUpdate().

Encoder knob(ENCODER_CLK, ENCODER_DT);
long lastPosition = 0;
bool clicked = false;
int direction = 0;

// ── Initialisation ────────────────────────────────────────────
// Sets the push-button pin to INPUT_PULLUP (active LOW).
void encoderInit() {
  pinMode(ENCODER_SW, INPUT_PULLUP);
}

// ── Poll Encoder ──────────────────────────────────────────────
// Must be called every loop iteration. Updates direction and
// click state from the current hardware readings.
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

// ── Consume Direction ─────────────────────────────────────────
// Returns +1 (CW), -1 (CCW), or 0 (no movement since last call).
// Self-clearing — safe to call multiple times per frame.
int getEncoderDirection() {
  int temp = direction;
  direction = 0;
  return temp;
}

// ── Consume Click ─────────────────────────────────────────────
// Returns true once per button press, then clears the flag.
bool encoderClicked() {
  if (clicked) {
    clicked = false;
    return true;
  }
  return false;
}

// ══════════════════════════════════════════════════════════════
// MENU / STATE HANDLERS
// ══════════════════════════════════════════════════════════════
// Each handle*() function corresponds to one FSM state and is
// called from loop() when currentScreen matches. They read the
// encoder for navigation / value adjustment and use the click
// to confirm selections or change screens.

int selectedMode = 0;   // currently highlighted menu row
int therapyMin = 60;    // lower bound for therapy oscillation
int therapyMax = 100;   // upper bound for therapy oscillation
int manualAngle = 90;   // current manual-mode angle
int therapyAngle = SERVO_CENTER; // current therapy-mode angle
int therapyDirection = 1;        // +1 = increasing, -1 = decreasing
unsigned long lastTherapyMove = 0; // timestamp of last servo step

// ── MAIN MENU ─────────────────────────────────────────────────
// Two options: Manual (0) and Therapy (1). Encoder rotates the
// selection; click enters the chosen mode.
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

// ── MANUAL MODE ───────────────────────────────────────────────
// Encoder directly sets the servo angle (0-180). When the angle
// leaves the 50-110° safe zone the buzzer and LED activate.
// Click returns to the main menu and resets the angle to 90°.
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

// ── THERAPY SET MIN ──────────────────────────────────────────
// Encoder adjusts the minimum angle in steps of 5, clamped to
// [50, 70]. Click proceeds to set the maximum.
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

// ── THERAPY SET MAX ──────────────────────────────────────────
// Encoder adjusts the maximum angle in steps of 5, clamped to
// [95, 110]. Click starts therapy.
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

// ── THERAPY MODE ─────────────────────────────────────────────
// Every 15 ms the servo moves one degree in the current
// direction. When it reaches a bound the direction flips.
// Buzzer/LED warn when the angle exits the 50-110° safe zone.
// Click pauses therapy.
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

// ── THERAPY PAUSED ───────────────────────────────────────────
// Two options: Resume (0) continues oscillation, Exit (1)
// returns to the main menu. The servo stays at its last angle.
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

// ══════════════════════════════════════════════════════════════
// MAIN ENTRY POINT
// ══════════════════════════════════════════════════════════════

// ── Global Objects & State ────────────────────────────────────
Servo servo;                     // PWM servo motor
Screen currentScreen = BOOT;     // current FSM state
void (*resetFunc)(void) = 0;     // software reset via address 0

// ── Boot Sequence ─────────────────────────────────────────────
// 1. Init LCD & encoder, configure I/O pins.
// 2. Show greeting for 2 seconds.
// 3. Light LED & buzzer, show calibration warning — wait for
//    encoder click.
// 4. Attach servo, centre it, wait 1 second.
// 5. Turn off LED/buzzer, transition to MAIN_MENU.
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

// ── Main Loop ─────────────────────────────────────────────────
// 1. Check hardware reset button; trigger soft reset if pressed.
// 2. Poll the encoder (reads position & button state).
// 3. Dispatch to the handler for the current FSM state.
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
