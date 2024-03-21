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
extern void stopwatchState();
extern void clock();
extern void clockState();
extern void timesetState();
extern void show_digit(char character, int pos);
extern void initialise_process(unsigned int process_index, void (*funct)());

struct ProcessControlBlock
{
    LONG sp;
    BYTE stack[STACK_SIZE];
};
struct ProcessControlBlock process[MAX_PROCESSES];

enum state{STARTUP, CLOCK, MONTH, ALARM, TIMESET, ALARMSET, CHRONO, LAP};
static const char *days[7] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
int STATE;

unsigned int current_process;
volatile unsigned int time, minutes, hours, day, month;
volatile unsigned int r;
volatile int startPressed, lapPressed, modePressed, buttonEvent;
volatile int stopwatchRunning, lapTime;
volatile char alarmActive, chimeActive;
LONG status;
LONG stack_pointer;
LONG program_counter;
LONG saved_sp;
WORD pc1;
WORD pc2;

#endif
