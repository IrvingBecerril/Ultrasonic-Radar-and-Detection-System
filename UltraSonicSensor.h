#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <wiringPi.h>
#include <softTone.h>

#define PIN 25
#define freq 100

volatile int bps = 1;  // number of beeps per second

void beep(){
    softToneWrite(PIN, freq);
    delay(1000 / (bps*2));
    softToneWrite(PIN, 0);
    delay(1000 / (bps*2));
}

void buzzerOFF(){
    softToneWrite(PIN, 0);
    delay(1000);
}

void calcFreq(int currVal, int maxVal){
    bps = 1 + 9 - 9 * currVal / maxVal;
}

void initBuzzer(){
    softToneCreate(PIN);
}