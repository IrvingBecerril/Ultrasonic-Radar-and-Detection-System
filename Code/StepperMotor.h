#ifndef STEPPERMOTOR_H_
#define STEPPERMOTOR_H_

#define IN1 0    // wiringPi GPIO0(pin11)   bcm 17
#define IN2 1    // pin12                   bcm 18
#define IN3 2    // pin13                   bcm 27
#define IN4 3    // pin15                   bcm 22

extern volatile int step;
extern volatile int stopFlag;

void setStep(int a, int b, int c, int d);
void stop(void);
void forward(int delay);
void backward(int delay);
void displayPosition(int step, int direction);
void returnToOrigin(int currStep);
//void *startStepperThread(void *arg);


#endif
