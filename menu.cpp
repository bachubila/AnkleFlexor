#include "menu.h"
#include "encoder.h"
#include "display.h"
#include "states.h"


extern Screen currentScreen;


int selectedMode = 0;



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
      selectedMode = 0;
      drawManual();

    }
    else{

      currentScreen = THERAPY_MODE;
      selectedMode = 0;
      drawTherapy();

    }

  }

}


void handleManualMode(){

  if(encoderClicked()){

    currentScreen = MAIN_MENU;
    selectedMode = 0;
    drawMainMenu(selectedMode);

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
      selectedMode = 0;
      drawTherapy();

    }
    else{

      currentScreen = MAIN_MENU;
      selectedMode = 0;
      drawMainMenu(selectedMode);

    }

  }

}
