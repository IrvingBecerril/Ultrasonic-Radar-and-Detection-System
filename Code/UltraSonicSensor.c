/*
 * Filename    : distance.c
 * Description : Measure distance with ultrasonic sensor module.
 * Website     : www.adeept.com
 * E-mail      : support@adeept.com
 * Author      : Jason
 * Date        : 2015/06/21
 */
#include <wiringPi.h>  
#include <stdio.h>  
#include <stdlib.h>
#include <sys/time.h>  
#include "UltraSonicSensor.h"
#include <pthread.h>

volatile long start = 0, end = 0;
struct timeval tv1, tv2;
volatile int measurementCmplt = 0;
volatile float distance;

void echoISR(){
	if(digitalRead(Echo) == HIGH){
		gettimeofday(&tv1, NULL);
		start = tv1.tv_sec * 1000000 + tv1.tv_usec;
	} else {
		gettimeofday(&tv2, NULL);
		end = tv2.tv_sec * 1000000 + tv2.tv_usec;
		measurementCmplt = 1;
	}
}

void ultraInit()  
{  
	pinMode(Echo, INPUT);  
	pinMode(Trig, OUTPUT); 
	printf("Sensor pins initialized!\n"); 
}  

float disMeasure()  
{  
	delay(60);
	digitalWrite(Trig, LOW);  
	delayMicroseconds(2);  

	digitalWrite(Trig, HIGH);  //produce a pluse
	delayMicroseconds(10); 
	digitalWrite(Trig, LOW);  

	while(!measurementCmplt){}

	measurementCmplt = 0;
	distance = (float)(end - start) / 1000000 * 34000 / 2;  //count the distance 

	return distance;
}

/*void *startUltraSonicThread(void *arg){
	if(wiringPiSetup() < 0){
		printf("wiringPi Failed\n");
		exit(EXIT_FAILURE);
	}

	ultraInit();
	
	if(wiringPiISR(Echo, INT_EDGE_BOTH, &echoISR) < 0){
		printf("wiringPiISR failed\n");
		exit(EXIT_FAILURE);
	}

	while(!stopFlag){
		distance = disMeasure();
		if(distance < distThreshold){
			printf("Distance = %.2f cm\n", distance);
			//set buzzer on
		}
		else{
			//set buzzer off
		}
	}

	printf("Exiting ultrasonic sensor thread...\n");
	pthread_exit(NULL);
}*/


/*int main(void)  // UltraSonic main
{  
	float dis;  

	if(wiringPiSetup() == -1){ //when initialize wiring failed,print message to screen  
		printf("setup wiringPi failed !\n");  
		return -1;   
	}  

	ultraInit();  

	while(1){  
		dis = disMeasure();  
		printf("Distance = %0.2f cm\n",dis);  
		delay(1000);  
	}  

	return 0;  
}  

int main() {    // LCD main
    if (wiringPiSetup() == -1) return 1;
    fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init();
    lcd_clear();
    createCustomChars();
    sweepBar(250);
    return 0;
}*/