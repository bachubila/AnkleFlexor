#include "encoder.h"
#include "config.h"

#include <Encoder.h>


Encoder knob(
  ENCODER_CLK,
  ENCODER_DT
);


long lastPosition = 0;

bool clicked = false;
int direction = 0;



void encoderInit(){

  pinMode(
    ENCODER_SW,
    INPUT_PULLUP
  );

  pinMode(
      ENCODER_CLK,
      INPUT_PULLUP
      );

  pinMode(
      ENCODER_DT,
      INPUT_PULLUP
      );
}



void encoderUpdate(){

  long pos = knob.read()/4;


  direction = 0;


  if(pos > lastPosition)
    direction = 1;


  if(pos < lastPosition)
    direction = -1;


  lastPosition = pos;



  if(digitalRead(ENCODER_SW)==LOW){

    clicked = true;
    delay(150);

  }

}



int getEncoderDirection(){

  int temp = direction;
  direction = 0;

  return temp;

}



bool encoderClicked(){

  if(clicked){

    clicked=false;
    return true;

  }

  return false;
}
