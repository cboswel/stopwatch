/**
 * Header file containing #defines, process function prototypes
 * (necessary to pass function pointers), data structures and global
 * variables.
 */

#ifndef STOPWATCH_STOPWATCH_H_
#define STOPWATCH_STOPWATCH_H_

#include <msp430.h>

#define EMPTY 0
#define START_BUTT 4    // 0100
#define LAP_BUTT 8      // 1000
#define MODE_BUTT 0xC    // 1100
#define BYTE unsigned char
#define BUTT_PRESSED 1  // 0001
#define BUTT_RELEASED 2   // 0010
#define ButtonID 2        // 0010
#define ProcessID 1       // 0001

#define RLED 0
#define GLED 0
#define ALL 9
#define SECOND 1000
#define MINUTE 60000
#define HOUR 3600000
#define DAY 86400000

#define LONG unsigned long
#define WORD unsigned short
#define BYTE unsigned char
#define MAX_PROCESSES   3
#define STACK_SIZE      100

// Process prototypes
extern void update_LCD();
extern void stopwatch();
extern void clock();
extern void timeset();
extern void alarm_check();
extern void alarm_ring();

struct ProcessControlBlock {
    LONG sp;
    BYTE stack[STACK_SIZE];
};
struct ProcessControlBlock process[MAX_PROCESSES + 3];

enum state {
    CLOCK, TIMESET, CHRONO, ALARM
};
static const char *days[7] = { "Mo", "Tu", "We", "Th", "Fr", "Sa", "Su" };
static const int monthLength[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, \
                                     30, 31 };
int STATE, currentState;

volatile int buffer;

volatile unsigned int current_process, process_slot;
volatile unsigned long time, stopwatchTime, lapTime, alarmTime, sixtySeconds;
volatile unsigned int minutes, hours, day, date, month;
// day = day of the week (out of 7), date = day of the month (out of 31)
volatile char alarmActive, chimeActive;
volatile char alarmSetMode, lapMode, monthMode, stopwatchRunning, selectedField;
LONG status;
LONG stack_pointer;
LONG program_counter;
LONG saved_sp;
WORD pc1;
WORD pc2;

// Misc function prototypes

extern void alarm_update(char field);
extern void blink_digit(char field);
extern void change_state();
extern void clock_update();
extern void display_alarm();
extern void display_alert();
extern void display_clock();
extern void display_month();
extern void display_stopwatch();
extern long get_time();
extern void initialise_process(unsigned int process_index, void (*funct)());
extern void LCD_extras();
extern void LCD_init();
extern void process_scheduler();
extern BYTE receive(BYTE RecipitentID);
extern void run_process(unsigned int process_index);
extern void send(BYTE Recipitent, BYTE ButtonType, BYTE ButtonState);
extern void setup();
extern void show_digit(char character, char pos);
extern void time_adv(char field);

#endif  // STOPWATCH_STOPWATCH_H_
