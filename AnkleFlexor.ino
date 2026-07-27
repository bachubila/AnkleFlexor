#include <LiquidCrystal_I2C.h>
#include <Encoder.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

// Encoder pins
Encoder encoder(3, 4);
const int SW = 5;

long lastPosition = 0;

void setup() {
    pinMode(2, OUTPUT);
    pinMode(SW, INPUT_PULLUP);

    lcd.init();
    lcd.backlight();

    lcd.print("Ankle Therapy");
    delay(1000);
    lcd.clear();

    lastPosition = encoder.read();
}

void loop() {
    long position = encoder.read();

    // The encoder changes by 4 for each physical click
    if (position != lastPosition) {

        if (position > lastPosition) {
            lcd.clear();
            lcd.print("Counterclockwise");
        } else {
            lcd.clear();
            lcd.print("Clockwise");
        }

        lastPosition = position;
    }

    if (digitalRead(SW) == LOW) {
        lcd.clear();
        lcd.print("Clicked!");

        delay(200); // debounce
    }
}
