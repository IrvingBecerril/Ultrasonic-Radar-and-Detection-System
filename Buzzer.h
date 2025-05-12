#ifndef BUZZER_H_
#define BUZZER_H_

#define PIN 25

void beep();
void buzzerOFF();
void calcFreq(int currVal, int maxVal);
void initBuzzer();

#endif