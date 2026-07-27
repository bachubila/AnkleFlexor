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



void drawManual(){

  lcd.clear();

  lcd.print("Manual Mode");

  lcd.setCursor(0,1);
  lcd.print("Click to exit");

}



void drawTherapy(){

  lcd.clear();

  lcd.print("Therapy Mode");

  lcd.setCursor(0,1);
  lcd.print("Running...");

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
