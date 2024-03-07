#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <msp430.h>

#define RLED 0
#define GLED 0
#define START_BUTT 2
#define LAP_BUTT 6

#define LONG unsigned long
#define WORD unsigned short
#define BYTE unsigned char
#define MAX_PROCESSES   2
#define STACK_SIZE      100

extern void red_led();
extern void green_led();
extern void update_LCD();
extern void stopwatch();
extern void show_digit(char character, int pos);

struct ProcessControlBlock
{
    LONG sp;
    BYTE stack[STACK_SIZE];
};
struct ProcessControlBlock process[MAX_PROCESSES];

enum state{STARTUP, CLOCK, MONTH, TIMESET, ALARMSET, CHRONO, LAP};
int STATE;

unsigned int current_process;
volatile unsigned int time;
volatile unsigned int r;
volatile int startPressed, lapPressed, modePressed, buttonEvent;
volatile int stopwatchRunning, lapTime;
LONG status;
LONG stack_pointer;
LONG program_counter;
LONG saved_sp;
WORD pc1;
WORD pc2;

#endif
