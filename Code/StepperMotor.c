#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include "StepperMotor.h"
#include "LCD_I2C.h"
#include <pthread.h>

volatile int step;

// Function to control stepper motor steps
void setStep(int a, int b, int c, int d) {
    digitalWrite(IN1, a);
    digitalWrite(IN2, b);
    digitalWrite(IN3, c);
    digitalWrite(IN4, d);
}

// Stop function to turn off all pins
void stop(void) {
    setStep(0, 0, 0, 0);
}

// Function for moving forward 360 degrees
void forward(int t) {
    setStep(1, 0, 0, 0);
    delay(t);
    setStep(0, 1, 0, 0);
    delay(t);
    setStep(0, 0, 1, 0);
    delay(t);
    setStep(0, 0, 0, 1);
    delay(t);
    //printf("Step value: %d\n", step);
}

// Function for moving backward 360 degrees
void backward(int t) {
    setStep(0, 0, 0, 1);
    delay(t);
    setStep(0, 0, 1, 0);
    delay(t);
    setStep(0, 1, 0, 0);
    delay(t);
    setStep(1, 0, 0, 0);
    delay(t);
    //printf("Step value: %d\n", step);
}

/*void displayPosition(int step, int direction){
    float degrees = step/512.0 * 360.0;
    int pixelPos;

    if(direction == 1)
        pixelPos = (int)(degrees/4.5);
    else
        pixelPos = 79 - (int)(degrees/4.5);

    lcd_clear();
    drawBar(pixelPos);
}*/

// goes back until at position 0/origin
void returnToOrigin(int currStep){
    for(int i = currStep; i > 0; i--){
	backward(3);
    }
}