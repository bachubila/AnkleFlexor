#include "menu.h"
#include "encoder.h"
#include "display.h"
#include "states.h"
#include "config.h"

#include <Arduino.h>
#include <Servo.h>


extern Screen currentScreen;
extern Servo servo;


int selectedMode = 0;

int therapyMin = 60;
int therapyMax = 100;
int therapySpeed = 15;

int manualAngle = 90;

int therapyAngle = SERVO_CENTER;
int therapyDirection = 1;
unsigned long lastTherapyMove = 0;



void handleMainMenu(){


  int move = getEncoderDirection();


  if(move){

    selectedMode += move;


    if(selectedMode < 0)
      selectedMode=1;


    if(selectedMode > 1)
      selectedMode=0;


    drawMainMenu(selectedMode);

  }



  if(encoderClicked()){


    if(selectedMode==0){

      currentScreen = MANUAL_MODE;
      drawManual(manualAngle);

    }
    else{

      currentScreen = THERAPY_SET_MIN;
      drawTherapySetMin(therapyMin);

    }

  }

}


void handleManualMode(){

  int move = getEncoderDirection();

  if(move){

    manualAngle += move;

    if(manualAngle < 0)
      manualAngle = 0;

    if(manualAngle > 180)
      manualAngle = 180;

    if(manualAngle < 50 || manualAngle > 110){
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
    }
    else{
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
    }

    drawManual(manualAngle);
    servo.write(manualAngle);

  }

  if(encoderClicked()){

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    currentScreen = MAIN_MENU;
    selectedMode = 0;
    manualAngle = 90;
    drawMainMenu(selectedMode);

  }

}


void handleTherapySetMin(){

  int move = getEncoderDirection();

  if(move){

    therapyMin += move * 5;

    if(therapyMin < 50)
      therapyMin = 50;

    if(therapyMin > 70)
      therapyMin = 70;

    drawTherapySetMin(therapyMin);

  }

  if(encoderClicked()){

    currentScreen = THERAPY_SET_MAX;
    drawTherapySetMax(therapyMax);

  }

}


void handleTherapySetMax(){

  int move = getEncoderDirection();

  if(move){

    therapyMax += move * 5;

    if(therapyMax < 95)
      therapyMax = 95;

    if(therapyMax > 110)
      therapyMax = 110;

    drawTherapySetMax(therapyMax);

  }

  if(encoderClicked()){

    currentScreen = THERAPY_SET_SPEED;
    drawTherapySetSpeed(therapySpeed);

  }

}


void handleTherapySetSpeed(){

  int move = getEncoderDirection();

  if(move){

    therapySpeed += move * 5;

    if(therapySpeed < 10)
      therapySpeed = 10;

    if(therapySpeed > 100)
      therapySpeed = 100;

    drawTherapySetSpeed(therapySpeed);

  }

  if(encoderClicked()){

    currentScreen = THERAPY_MODE;
    therapyAngle = SERVO_CENTER;
    therapyDirection = 1;
    lastTherapyMove = millis();
    drawTherapy(therapyMin, therapyMax, therapyAngle);

  }

}


void handleTherapyMode(){

  if(millis() - lastTherapyMove >= therapySpeed){

    lastTherapyMove = millis();

    therapyAngle += therapyDirection;

    if(therapyAngle >= therapyMax)
      therapyDirection = -1;

    if(therapyAngle <= therapyMin)
      therapyDirection = 1;

    servo.write(therapyAngle);

    if(therapyAngle < 50 || therapyAngle > 110){
      digitalWrite(BUZZER_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
    }
    else{
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(LED_PIN, LOW);
    }

  }

  if(encoderClicked()){

    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    currentScreen = THERAPY_PAUSED;
    selectedMode = 0;
    drawPaused(selectedMode);

  }

}


void handlePauseMenu(){

  int move = getEncoderDirection();

  if(move){

    selectedMode += move;

    if(selectedMode < 0)
      selectedMode=1;

    if(selectedMode > 1)
      selectedMode=0;

    drawPaused(selectedMode);

  }

  if(encoderClicked()){

    if(selectedMode==0){

      currentScreen = THERAPY_MODE;
      drawTherapy(therapyMin, therapyMax, therapyAngle);

    }
    else{

      currentScreen = MAIN_MENU;
      selectedMode = 0;
      drawMainMenu(selectedMode);

    }

  }

}
