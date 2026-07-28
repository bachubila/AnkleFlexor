#include "menu.h"
#include "encoder.h"
#include "display.h"
#include "states.h"


extern Screen currentScreen;


int selectedMode = 0;

int therapyMin = 60;
int therapyMax = 100;

int manualAngle = 90;



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

    drawManual(manualAngle);

  }

  if(encoderClicked()){

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

    if(therapyMax < 90)
      therapyMax = 90;

    if(therapyMax > 110)
      therapyMax = 110;

    drawTherapySetMax(therapyMax);

  }

  if(encoderClicked()){

    currentScreen = THERAPY_MODE;
    drawTherapy(therapyMin, therapyMax);

  }

}


void handleTherapyMode(){

  if(encoderClicked()){

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
      drawTherapy(therapyMin, therapyMax);

    }
    else{

      currentScreen = MAIN_MENU;
      selectedMode = 0;
      drawMainMenu(selectedMode);

    }

  }

}
