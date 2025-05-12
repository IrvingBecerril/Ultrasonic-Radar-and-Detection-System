#ifndef LCD_I2C_H_
#define LCD_I2C_H_

#define I2C_ADDR 0x27
#define LCD_CHR 1
#define LCD_CMD 0
#define LINE1 0x80
#define LINE2 0xC0
#define LCD_BACKLIGHT 0x08
#define ENABLE 0b00000100

extern int fd;
extern volatile int objFlag;
int mapStepToPixel(int step);
void lcdByte(int bits, int mode);
void lcdInit();
void lcdInit(int line);
void createBar();
void drawBar(int step);
void clearBar(int step);
void fsm();
void *lcdThread(void *arg);

// TESTING---------------
void clearLine1Block(int block);
void lightUpLine1Block(int block);
void updateLine1Dispaly();
void line1FSM();
void clearScreen();
// --------------------------

#endif