#ifndef DISPLAY_H
#define DISPLAY_H

void displayInit();

void showGreeting();

void drawCalibrationWarning();

void drawCalibrating();

void drawMainMenu(int selected);

void drawManual(int angle);

void drawTherapySetMin(int value);

void drawTherapySetMax(int value);

void drawTherapy(int minVal, int maxVal);

void drawPaused(int selected);

#endif
