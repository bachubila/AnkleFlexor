#include "display.h"
#include "config.h"

#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(
  LCD_ADDRESS,
  LCD_COLUMNS,
  LCD_ROWS
);



void displayInit(){

  lcd.init();
  lcd.backlight();

}



void showGreeting(){

  lcd.clear();

  lcd.print("Ankle Flexor");
  lcd.setCursor(0,1);
  lcd.print("Initializing...");

}



void drawCalibrationWarning(){

  lcd.clear();

  lcd.print("!! WARNING !!");
  lcd.setCursor(0,1);
  lcd.print("Do NOT place");
  lcd.setCursor(0,2);
  lcd.print("patient's foot");
  lcd.setCursor(0,3);
  lcd.print("Click to continue");

}



void drawCalibrating(){

  lcd.clear();

  lcd.print("Calibrating...");
  lcd.setCursor(0,1);
  lcd.print("Keep foot clear");

}



void drawMainMenu(int selected){

  lcd.clear();

  lcd.print("Select Mode");

  lcd.setCursor(0,1);
  if(selected==0)
    lcd.print("> Manual");
  else
    lcd.print("  Manual");

  lcd.setCursor(0,2);
  if(selected==1)
    lcd.print("> Therapy");
  else
    lcd.print("  Therapy");

}



void drawManual(int angle){

  lcd.clear();

  lcd.print("Manual Mode");

  lcd.setCursor(0,1);
  lcd.print("Angle: ");
  lcd.print(angle);

  lcd.setCursor(0,2);
  lcd.print("Rotate to adjust");
  lcd.setCursor(0,3);
  lcd.print("Click: exit");

}



void drawTherapySetMin(int value){

  lcd.clear();

  lcd.print("Set Min Angle");

  lcd.setCursor(0,1);
  lcd.print("Value: ");
  lcd.print(value);

  lcd.setCursor(0,2);
  lcd.print("Rotate: adjust");
  lcd.setCursor(0,3);
  lcd.print("Click: confirm");

}



void drawTherapySetMax(int value){

  lcd.clear();

  lcd.print("Set Max Angle");

  lcd.setCursor(0,1);
  lcd.print("Value: ");
  lcd.print(value);

  lcd.setCursor(0,2);
  lcd.print("Rotate: adjust");
  lcd.setCursor(0,3);
  lcd.print("Click: confirm");

}



void drawTherapy(int minVal, int maxVal){

  lcd.clear();

  lcd.print("Therapy Mode");

  lcd.setCursor(0,1);
  lcd.print("Running...");

  lcd.setCursor(0,2);
  lcd.print("Min: ");
  lcd.print(minVal);

  lcd.setCursor(0,3);
  lcd.print("Max: ");
  lcd.print(maxVal);

}



void drawPaused(int selected){

  lcd.clear();

  lcd.print("Paused Therapy");

  lcd.setCursor(0,1);
  if(selected==0)
    lcd.print("> Resume");
  else
    lcd.print("  Resume");

  lcd.setCursor(0,2);
  if(selected==1)
    lcd.print("> Exit");
  else
    lcd.print("  Exit");

}
