#include "display.h"
#include "encoder.h"
#include "menu.h"
#include "states.h"
#include "config.h"

#include <Servo.h>

Servo servo;

Screen currentScreen = BOOT;


void setup() {

  displayInit();
  encoderInit();

  showGreeting();
  delay(2000);

  drawCalibrationWarning();
  while(!encoderClicked()){
    encoderUpdate();
  }

  drawCalibrating();
  servo.attach(SERVO_PIN);
  servo.write(SERVO_CENTER);
  delay(1000);

  currentScreen = MAIN_MENU;
  drawMainMenu(0);
}


void loop() {

  encoderUpdate();


  switch(currentScreen) {


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
