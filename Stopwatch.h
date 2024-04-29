/**
 * Header file containing #defines, process function prototypes
 * (necessary to pass function pointers), data structures and global
 * variables.
 */

#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <msp430.h>

#define RLED 0
#define GLED 0
#define START_BUTT 2
#define LAP_BUTT 6
#define SECOND 1000
#define MINUTE 60000
#define HOUR 3600000
#define DAY 86400000

#define LONG unsigned long
#define WORD unsigned short
#define BYTE unsigned char
#define MAX_PROCESSES   3
#define STACK_SIZE      100

struct ProcessControlBlock
{
    LONG sp;
    BYTE stack[STACK_SIZE];
};
struct ProcessControlBlock process[MAX_PROCESSES + 3];
typedef struct ProtectedLong protectedLong;
typedef struct ProtectedInt protectedInt;

extern struct ProtectedLong {
    LONG value;
    char unlocked;
    LONG (*get)(protectedLong);
    void (*set)(protectedLong, LONG *value);
 //   void (*increment)(struct mutex);
};

extern struct ProtectedInt {
    unsigned int value;
    char unlocked;
    LONG (*get)(protectedInt);
    void (*set)(protectedInt, int *value);
 //   void (*increment)(struct mutex);
};

// Process prototypes
extern void update_LCD();
extern void stopwatch();
extern void stopwatchState();
extern void clock();
extern void clockState();
extern void timeset();
extern void alarmCheck();
extern void alarmRing();

enum state{CLOCK, TIMESET, CHRONO, ALARM};
static const char *days[7] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};
static const int monthLength[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
int STATE, currentState, newState;

volatile unsigned int current_process, toggle_process;
volatile LONG stopwatchTime, lapTime;
volatile unsigned int minutes, hours, day, date, month; // day = day of the week (out of 7), date = day of the month (out of 31)
volatile unsigned int r;
volatile char startPressed, lapPressed, modePressed, buttonEvent, toggle;
volatile char alarmActive, chimeActive;
volatile char lapMode, stopwatchRunning, selectedField;
LONG status;
LONG stack_pointer;
LONG program_counter;
LONG saved_sp;
WORD pc1;
WORD pc2;

//Misc function prototypes

extern void alarm_update(char field);
extern void blink_digit(char field);
extern void change_state();
extern void display_alarm();
extern void display_alert();
extern void display_clock();
extern void display_month();
extern void display_stopwatch();
extern long getTime();
extern void initialise_process(unsigned int process_index, void (*funct)());
extern void LCD_init();
extern void run_process(unsigned int process_index);
extern void setup();
extern void show_digit(char character, char pos);
extern void timeAdv(char field);
extern void wait(volatile char *sema);
extern int getInt(protectedInt mutex);
extern void setInt(protectedInt mutex, int value);
extern LONG getLong(protectedLong mutex);
extern void setLong(protectedLong mutex, LONG value);

protectedLong time, alarmTime;
protectedInt alarmSetMode, monthMode;

#endif
