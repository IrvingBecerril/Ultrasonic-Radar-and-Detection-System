#include "LCD_I2C.h"
#include "StepperMotor.h"
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define NUM_COLUMNS 80
#define NUM_BLOCKS 16

volatile int objDetected[NUM_BLOCKS] = {0};
int fd;
int newBlock;

typedef enum{INIT, CLEAR, DRAW, UPDATE} State;
typedef enum{INIT2, CURRENT_BLOCK, ON, OFF, UPDATE2}Line2State;

int mapStepToPixel(int step);
void lcdByte(int bits, int mode);
void lcdInit();
void lcdLoc(int line);
void createBar();
void drawBar(int step);
void clearBar(int step);
void fsm();

//int step = 0;     // Starting step value
int prevStep = -1; // Track previous step value to clear the bar
int direction = 1; // 1 = increment, -1 = decrement

void *lcdThread(void *arg) {
    if (wiringPiSetup() == -1) {
        printf("Failed to initialize wiringPi\n");
        exit(EXIT_FAILURE);
    }

    fd = wiringPiI2CSetup(I2C_ADDR);
    if (fd == -1) {
        printf("Failed to initialize I2C communication\n");
        exit(EXIT_FAILURE);
    }

    lcdInit();       // Initialize LCD
    createBar();     // Create custom bar characters

    

    while (!stopFlag) {
        fsm();
        line1FSM();
    }
    clearScreen();
    pthread_exit(NULL);
}

// TESTING ALTERNATIVES---------------------------------------------------
void updateLine1Display() {
    static int currentBlock = -1; // Tracks the block being scanned on LINE2
    static int blockObjFlag = 0;  // Tracks if objFlag == 1 during the scan of the current block

    // Determine the current block based on the step
    newBlock = mapStepToPixel(step) / 5;

    // Check if we've moved to a new block
    if (newBlock != currentBlock) {
        // Update LINE1 for the previous block
        if (currentBlock >= 0) { // Ensure valid block index
            if (blockObjFlag) {
                lightUpLine1Block(currentBlock); // Light up the block on LINE1
            } else {
                clearLine1Block(currentBlock);    // Leave the block blank
            }
        }

        // Reset for the new block
        blockObjFlag = 0;
        currentBlock = newBlock;
    }

    // Update blockObjFlag if objFlag == 1
    if (objFlag) {
        blockObjFlag = 1;
    }
}

void line1FSM(){
    static Line2State state = INIT;
    static int currentBlock = -1; // Tracks the block being scanned on LINE2
    static int blockObjFlag = 0;

    //State transitions
    switch(state){
        case INIT2:
            state = CURRENT_BLOCK;
            break;
        case CURRENT_BLOCK:
            if(newBlock != currentBlock){
                if(currentBlock >= 0){
                    if(blockObjFlag){
                        state = ON;
                    }else{
                        state = OFF;
                    }
                }else{
                    blockObjFlag = 0;
                    currentBlock = newBlock;
                }
            }else if(objFlag){
                state = UPDATE2;
            }

            break;

        case ON:
            if(objFlag)
                state = UPDATE2;
            else
                state = CURRENT_BLOCK;
            break;

        case OFF:
            if(objFlag)
                state = UPDATE2;
            else
                state = CURRENT_BLOCK;
            break;

        case UPDATE2:
            state = CURRENT_BLOCK;
            break;
    }

    //State Actions
    switch(state){
        case CURRENT_BLOCK:
            newBlock = mapStepToPixel(step) / 5;
            break;
        case ON:
            lightUpLine1Block(currentBlock);

            // Reset for the new block
            blockObjFlag = 0;
            currentBlock = newBlock;
            break;
        case OFF:
            clearLine1Block(currentBlock);
            
            // Reset for the new block
            blockObjFlag = 0;
            currentBlock = newBlock;
            break;
        case UPDATE2:
            blockObjFlag = 1;
            break;
    }
}

// Function to light up a block on LINE1
void lightUpLine1Block(int block) {
    lcd_loc(LINE1 + block); // Move to the block's position
    lcd_byte(0xFF, LCD_CHR); // Use a predefined character to light up the block
}

// Function to clear a block on LINE1
void clearLine1Block(int block) {
    lcd_loc(LINE1 + block); // Move to the block's position
    lcd_byte(' ', LCD_CHR); // Clear the block by overwriting with a space
}

//----------------------------------------------------------------------

// DO NOT DELETE OR EDIT
void fsm(){ 
    static State state = INIT;
    
    switch(state){
        case INIT:
            state = DRAW;
            break;

        case CLEAR:
            state = DRAW;
            break;

        case DRAW: 
            state = UPDATE;
            break;

        case UPDATE:
            if(step!=prevStep){
                if(prevStep >= 0){
                    state = CLEAR;
                }else{
                    state = DRAW;
                }
            }
            break;
    }

    switch(state){
        case CLEAR:
            clearBar(prevStep);
            break;
        case DRAW:
            drawBar(step);         // Draw the bar at the current step
            prevStep = step;
            break;
    }
}


// Send byte to the LCD
void lcdByte(int bits, int mode) {
    int bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    int bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    wiringPiI2CWrite(fd, bits_high);
    usleep(500);
    wiringPiI2CWrite(fd, (bits_high | ENABLE));
    usleep(500);
    wiringPiI2CWrite(fd, (bits_high & ~ENABLE));
    usleep(500);

    wiringPiI2CWrite(fd, bits_low);
    usleep(500);
    wiringPiI2CWrite(fd, (bits_low | ENABLE));
    usleep(500);
    wiringPiI2CWrite(fd, (bits_low & ~ENABLE));
    usleep(500);
}

// Initialize the LCD
void lcdInit() {
    lcdByte(0x33, LCD_CMD); // Initialize
    lcdByte(0x32, LCD_CMD); // Set to 4-bit mode
    lcdByte(0x06, LCD_CMD); // Cursor move direction
    lcdByte(0x0C, LCD_CMD); // Turn on, cursor off, no blink
    lcdByte(0x28, LCD_CMD); // 2 line display
    lcdByte(0x01, LCD_CMD); // Clear display
    usleep(500);
}

// Move cursor to the specified line
void lcdLoc(int line) {
    lcdByte(line, LCD_CMD);
}

// Create custom characters for 5 positions of the bar within a block
void createBar() {
    unsigned char barPatterns[5][8] = {
        {0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10}, // Bar in 1st column
        {0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08}, // Bar in 2nd column
        {0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}, // Bar in 3rd column
        {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02}, // Bar in 4th column
        {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01}  // Bar in 5th column
    };

    for (int i = 0; i < 5; i++) {
        lcdByte(0x40 + (i * 8), LCD_CMD); // Set CGRAM address for each character
        for (int j = 0; j < 8; j++) {
            lcdByte(barPatterns[i][j], LCD_CHR); // Write custom character pattern
        }
    }
}

// Print the vertical bar at a specific position on both lines
void drawBar(int step) {
    int pixelPos = mapStepToPixel(step);  // Map step to pixel position
    int block = pixelPos / 5;               // Calculate block (5-pixel blocks)
    int column = pixelPos % 5;              // Calculate column within the block

    lcdLoc(LINE2 + block);    // Move to Line 1, specific block position
    lcdByte(column, LCD_CHR); // Display custom character for the column

    
}


// Clear the bar at a specific position on both lines
void clearBar(int step) {
    int pixelPos = mapStepToPixel(step);  // Map step to pixel position
    int block = pixelPos / 5;               // Calculate block (5-pixel blocks)

    lcdLoc(LINE2 + block);    // Move to Line 1, specific block position
    lcdByte(' ', LCD_CHR);    // Overwrite with a space
}

// DO NOT DELETE OR EDIT
int mapStepToPixel(int step){
    return (step * 5) / 32;
}

void clearScreen() {
    // Clear LINE1
    for (int block = 0; block < NUM_BLOCKS; block++) {
        lcdLoc(LINE1 + block); // Move to each block on LINE1
        lcdByte(' ', LCD_CHR); // Overwrite with a blank space
    }

    // Clear LINE2
    for (int block = 0; block < NUM_BLOCKS; block++) {
        lcdLoc(LINE2 + block); // Move to each block on LINE2
        lcdByte(' ', LCD_CHR); // Overwrite with a blank space
    }

    // Reset any other relevant state variables if needed
    for (int i = 0; i < NUM_BLOCKS; i++) {
        objDetected[i] = 0; // Clear detected object flags
    }
}
