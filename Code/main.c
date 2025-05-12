#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "StepperMotor.h"
#include "UltraSonicSensor.h"
#include "Buzzer.h"
#include "LCD_I2C.h"
#include <signal.h>
#include <pthread.h>

//extern void *startStepperThread(void *arg);
//volatile int lcdPos = 0;
volatile int stopFlag = 0;
volatile int buzzerFlag = 0;
volatile int objFlag = 0;

typedef enum {STEPPER_INIT, FORWARD, BACKWARD}stepperState;
typedef enum {SENSOR_INIT, MEASURE_DISTANCE, OBJECT_DETECTED, NO_OBJECT}ultraState;
typedef enum {BUZZER_INIT, BUZZER_ON, BUZZER_OFF}buzzerState;
typedef enum {LCD_INIT, CLEAR, DRAW, UPDATE}lcdState;
//typedef enum{INIT, CLEAR, DRAW, UPDATE} LINE1State;
//


typedef enum{INIT2, CURRENT_BLOCK, ON, OFF, UPDATE2}Line2State;

pthread_t stepperID;
pthread_t ultraSonicID;
pthread_t buzzerID;
pthread_t lcdID;

// Thread & corresponding FSM functions
void *startStepperThread(void *arg);
void *startUltraSonicSensorThread(void *arg);
void *startBuzzerThread(void *arg);
void stepperFSM();
void ultraSonicFSM();
void buzzerFSM();
void sigint_handler(int signum);

int main(){
    signal(SIGINT, sigint_handler);

    step = 0;

    printf("Before stepper Thread\n");
    
    if(pthread_create(&stepperID, NULL, startStepperThread, NULL) != 0){
        perror("Failed to create stepper thread\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Stepper thread successful\n"
           "Before ultrasonic thread\n");

    if(pthread_create(&ultraSonicID, NULL, startUltraSonicSensorThread, NULL) != 0){
        perror("Failed to create stepperThread\n");
        exit(EXIT_FAILURE);
    }

    printf("Ultrasonic thread successfull\n"
           "Before buzzer thread\n");
    
    if(pthread_create(&buzzerID, NULL, startBuzzerThread, NULL) != 0){
        perror("Failed to create buzzer thread\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Buzzer thread successfull\n"
           "Before lcd thread\n");
    
    if(pthread_create(&lcdID, NULL, lcdThread, NULL) != 0){
        perror("failed to create lcd thread\n");
        exit(EXIT_FAILURE);
    }
    printf("LCD thread successfull\n");

    printf("\n\n\nJoining buzzer thread...\n");
    pthread_join(buzzerID, NULL);
    printf("Joining LCD thread...\n");
    pthread_join(lcdID, NULL);
    printf("Joining stepper thread\n");
    pthread_join(stepperID, NULL);
    printf("Joining ultrasonic thread");
    pthread_join(ultraSonicID, NULL);

    return 0;
}

/*void *startLCDThread(void *arg){
    if(wiringPiSetup() < 0){
        printf("Setup wiringPi for LCD failed\n");
        exit(EXIT_FAILURE);
    }else{
        printf("lcd wiringPi successful!\n");
    }

    fd = wiringPiI2CSetup(I2C_ADDR);
    if(fd < 0){
        printf("failed to initialize I2C communication\n");
        exit(EXIT_FAILURE);
    }


    lcd_init();
    create_bar();

    int prevPos = -1;

    while(!stopFlag){
        lcdFSM();
    }

    pthread_exit(NULL);
}*/

/*void lcdFSM(){
    static lcdState state = LCD_INIT;
    static int prevStep = -1;
    int pixelPos, block, column;

    //State transitions
    switch(state){
        case LCD_INIT:
            state = CLEAR;
            break;

        case CLEAR: 
            state = DRAW;
            break;

        case DRAW:
            if(prevStep != step){
                state = CLEAR;
            }
            break;

        case UPDATE:
            if(prevStep != step){
                state = CLEAR;
            }
            break;
    }

    //State Actions
    switch(state){
        case CLEAR:
            if(prevStep >= 0){
                pixelPos = mapStepToPixel(prevStep);
                clear_bar_on_both_lines(pixelPos);
            }
            break;

        case DRAW:
            pixelPos = mapStepToPixel(step);
            draw_bar_on_both_lines(pixelPos);
            prevStep = step;
            break;
    }
}*/

void *startUltraSonicSensorThread(void *arg){
    if(wiringPiSetup() < 0){
        printf("Setup wiringPi for Sensor failed\n");
        exit(EXIT_FAILURE);
    }else{
        printf("WiringPi for sensor complete!\n");
    }

    ultraInit();

    if(wiringPiISR(Echo, INT_EDGE_BOTH, &echoISR) < 0){
        printf("wiringPiISR failed\n");
        exit(EXIT_FAILURE);
    }else{
        printf("wiringPiISR complete!\n");
    }

    // FSM
    while(!stopFlag){
        ultraSonicFSM();
    }

    // Ending sensor thread
    printf("Exiting ultrasonic sensor thread...\n");
    pthread_exit(NULL);
}

void *startBuzzerThread(void *arg){
    if(wiringPiSetup() < 0){
        printf("Setup wiringPi for buzzer failed\n");
        exit(EXIT_FAILURE);
    }else{
        printf("wiringPi setup complete");
    }

    initBuzzer();

    while(!stopFlag){
        buzzerFSM();
    }

    printf("Exiting buzzer thread...\n");
    pthread_exit(NULL);

}

void buzzerFSM(){
    static buzzerState state = BUZZER_INIT;

    // State Transitions
    switch(state){
        case BUZZER_INIT:
            state = BUZZER_OFF;
            break;

        case BUZZER_OFF:
            if(distance > distThreshold)
                state = BUZZER_OFF;
            else if(distance <= distThreshold)
                state = BUZZER_ON;
            break;

        case BUZZER_ON:
            if(distance > distThreshold){
                state = BUZZER_OFF;
            }else{
                state = BUZZER_ON;
            }
            break;
    }

    // State Actions
    switch(state){
        case BUZZER_OFF:
            buzzerOFF();
            break;

        case BUZZER_ON:
            calcFreq(distance, distThreshold);
            beep();
            break;
    }    
}

void ultraSonicFSM(){
    static ultraState state = SENSOR_INIT;

    // State Transitions
    switch(state){
        case SENSOR_INIT:
            state = MEASURE_DISTANCE;
            break;

        case MEASURE_DISTANCE:
            if(distance > distThreshold)
                state = NO_OBJECT;
            else if(distance <= distThreshold)
                state = OBJECT_DETECTED;
            break;

        case OBJECT_DETECTED:
            state = MEASURE_DISTANCE;
            break;

        case NO_OBJECT:
            state = MEASURE_DISTANCE;
            break;
    }

    // State Actions
    switch(state){
        case MEASURE_DISTANCE:
            //printf("Measuring Distance\n");
            distance = disMeasure();
            //printf("Ending Measuring\n");
            break;

        case OBJECT_DETECTED:
            objFlag = 1;
            printf("Distance = %.2f cm\n", distance);
            break;

        case NO_OBJECT:
            objFlag = 0;
            //printf("No object detected\n");
            break;
    }
}

// thread function for stepper motor
void *startStepperThread(void *arg){
    if(wiringPiSetup() < 0){
	    printf("Setup wiringPi for stepper failed\n");
	    exit(EXIT_FAILURE);
    }else{
        printf("stepper wiringPi setup complete!\n");
    }

    //set pin modes as output
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    //lcd_init();
    //lcd_clear();

    while(!stopFlag){
        stepperFSM();
    }

	while(step > 0){
	    backward(15);
        step--;
    }
    stop();
    //clear_bar_on_both_lines(0);
    printf("Exiting stepper motor thread...\n");

    pthread_exit(NULL);
}

// FSM for stepper motor. 
void stepperFSM(){
    static stepperState state = STEPPER_INIT;

    // State Transitions
    switch(state){
        case STEPPER_INIT:
            state = FORWARD;
            break;

        case FORWARD:
            if(step == 512){
                state = BACKWARD;
                delay(1000);
            }
            break;

        case BACKWARD:
            if(step == 0){
                state = FORWARD;
                delay(1000);
            }
            break;
    }

    // State Actions
    switch(state){
        case FORWARD:
            //clearBar(lcdPos);

            forward(15);
            step++;

            //lcdPos = step * 5 / 32; // 512 steps/80 pixel columns = 6.4 = 32 / 5
            //drawBar(lcdPos);

            break;
        case BACKWARD:
            //clearBar(lcdPos);

            backward(15);
	        step--;

            //lcdPos = step * 5 / 32;
            //drawBar(lcdPos);

            break;
    }
}

void sigint_handler(int signum){ // Handler for SIGINT
    // Reset handler to catch SIGINT next time
    signal(SIGINT, sigint_handler);

    // flag to stop threads
    stopFlag = 1;

    printf("Caught SIGINT. \nReturning stepper motor to position 0\n");

    //make sure to finish current thread program before continuing
    pthread_join(stepperID, NULL);
    //pthread_join(ultraSonicID, NULL);



    printf("Step value: %d\n", step);
    fflush(stdout);

    exit(0);
}