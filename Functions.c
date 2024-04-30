/**
 * Miscellaneous functions that running processes can call to mimimise
 * repeated code.
 */

#include "Stopwatch.h"

/*F ----------------------------------------------------------------------------
 *
    NAME :      send(BYTE)

  DESCRIPTION :
               Sends data to the mailbox

  INPUTS :    - LOCATION OF BUFFER (global variable)
              - The message - long or short press
              - The target button

  RETURNS :   void

  PROCESS :
              [1] Check if mailbox is empty
              [2] Combine two parameters
              [3] Populate mailbox
 *
 *F ----------------------------------------------------------------------------*/

void send(BYTE RecipientID, BYTE ButtonType, BYTE ButtonState) {
    if (buffer == EMPTY) {
        buffer = ((int) RecipientID << 4) | ButtonType | ButtonState;
    }
    return;
}

/*F ----------------------------------------------------------------------------
  NAME :      receive ()

  DESCRIPTION :
               Reads from the mailbox

  INPUTS :     Reads from the buffer- a global variable


  returns :   Result from mailbox

  PROCESS :
              [1] Read from buffer
              [2] If address is correct, return message
              [3] Else return 0
*F ---------------------------------------------------------------------------*/

BYTE receive(BYTE RecipientID) {   // BYTE address
    if ((buffer != EMPTY) && (RecipientID == (buffer >> 4))) {
        BYTE message = buffer;
        buffer = EMPTY;            // Reset buffer to EMPTY
        _enable_interrupts();
        return message;
    }
    return 0;
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      clock_update()

  DESCRIPTION :
               converts a count of elapsed milliseconds to mins, hrs and days.

  INPUTS :    - ms count (global variable)

  RETURNS :   void

  PROCESS :
              for each time unit:
              [1] divide time by the number of milliseconds in that unit
                  to find quantity that have elapsed
              [2] Modulate by the number of that unit in the unit above
                  so it rolls over to zero when it is exceeded
 *
 *F ----------------------------------------------------------------------------*/

void clock_update() {
    minutes += (time / MINUTE);
    minutes %= 60;
    hours += (time / HOUR);
    hours %= 24;
    day += (time / DAY);
    day %= 7;
    date += (time / DAY);
    month += (date / monthLength[month]);
    month %= 12;

    date %= monthLength[month];
    // There is a map of month lengths in the header file; cannot have Feb 30th
    time %= MINUTE;
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      display_stopwatch()

  DESCRIPTION :
                Print the relevant data for the stopwatch mode onto the LCD

  INPUTS :    - count of ms since stopwatch started or last lap (globals)
              - Flag showing whether to display stopwatch time or laptime

  RETURNS :   void

  PROCESS :
              [1] Use lapmode to tell whether to show laptime or stopwatch time
              [2] Convert the relavent time in ms to ms, secs and mins
              [3] Display one digit at a time. Use div and mod to split digits
 *
 *F ----------------------------------------------------------------------------*/

void display_stopwatch() {
    int ms = stopwatchTime % SECOND;  // rollover if exceeding 1000
    int s = (stopwatchTime / SECOND) % 60;
    int min = (stopwatchTime / MINUTE) % 60;
    if (lapMode == 1) {  // In lapmode, laptime must be displayed
        ms = lapTime % SECOND;
        s = (lapTime / SECOND) % 60;
        min = (lapTime / MINUTE) % 60;
    }
    show_digit((min / 10), 0);
    show_digit((min % 10), 1);
    show_digit((s / 10), 2);
    show_digit((s % 10), 3);
    show_digit((ms / 100), 4);  // divide by 100 for tenths, ms = 3 places
    show_digit(((ms / 10) % 10), 5);  // get hundredths place
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      display_clock()

  DESCRIPTION : Displays a view of a 24-hr clock (hours and minutes) with a
		text day name e.g. Mo, Tu

  INPUTS :    - count of hours, minutes and day of the week (globals)

  RETURNS :   void

  PROCESS :
              [1] Check global variables for relevant time values
              [2] Display one digit at a time. Use div and mod to split digits
 *
 *F ----------------------------------------------------------------------------*/

void display_clock() {
    show_digit((hours / 10), 0);
    show_digit((hours % 10), 1);
    show_digit((minutes / 10), 2);
    show_digit((minutes % 10), 3);
    show_digit(days[day][0], 4);  // days is a list of strings. Find the first
    show_digit(days[day][1], 5);  // then second element in the right string
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      display_month()

  DESCRIPTION : Displays the calendar view: Date, month, text day name

  INPUTS :    - date, month, text day (global variable)

  RETURNS :   void

  PROCESS :
              [1] Check global variables for relevant time values
              [2] Display one digit at a time. Use div and mod to split digits
 *
 *F ----------------------------------------------------------------------------*/

void display_month() {
    show_digit(((date + 1) / 10), 0);    // +1 because of zero indexing
    show_digit(((date + 1) % 10), 1);
    show_digit(((month + 1) / 10), 2);
    show_digit(((month + 1) % 10), 3);
    show_digit(days[day][0], 4);
    show_digit(days[day][1], 5);
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      display_alarm()

  DESCRIPTION : Displays the time that the alarm is set to trigger if active

  INPUTS :      Alarm time (global variable)

  RETURNS :   void

  PROCESS :
              [1] Convert the alarm time in ms to mins and hours
              [2] Display one digit at a time. Use div and mod to split digits
 *
 *F ----------------------------------------------------------------------------*/

void display_alarm() {
    int mins = (alarmTime / MINUTE) % 60;
    int hrs = (alarmTime / HOUR) % 24;
    show_digit((hrs / 10), 0);
    show_digit((hrs % 10), 1);
    show_digit((mins / 10), 2);
    show_digit((mins % 10), 3);
    show_digit(days[day][0], 4);
    show_digit(days[day][1], 5);
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      display_alert()

  DESCRIPTION : writes "ALARM!" on the LCD to show that alarm has triggered

  INPUTS :    None

  RETURNS :   void

  PROCESS :
              [1] Display each character on the screen
	      [2] Blink the display to make it flash
 *
 *F ----------------------------------------------------------------------------*/

void display_alert() {
    /**
     * Blinks "ALARM!"
     */
    show_digit('A', 0);
    show_digit('L', 1);
    show_digit('A', 2);
    show_digit('R', 3);
    show_digit('M', 4);
    show_digit('!', 5);

    // blink the entire display
    blink_digit(0);
    blink_digit(1);
    blink_digit(2);
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      change_state()

  DESCRIPTION : Performs some admin when states change like making fields blink

  INPUTS :    - State (global variable)

  RETURNS :   void

  PROCESS :
              [1] Check state
	      [2] Set it up
 *
 *F ----------------------------------------------------------------------------*/

void change_state() {
    if (STATE == CHRONO) {
        LCDBLKCTL &= ~(0b11);   // No blinking
    } else if (STATE == TIMESET) {
        selectedField = 0;
        LCDBLKCTL |= 0b11;   // Start alternating mode blinking
    } else if (STATE == CLOCK) {
        LCDBLKCTL &= ~(0b11);   // No blinking
    } else if (STATE == ALARM) {
        LCDBLKCTL |= 0b11;   // Start alternating mode blinking
    }
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      time_adv(char)

  DESCRIPTION : increments the time by an amount depending on selected field

  INPUTS :    - field. This is the selected number in timeset.

  RETURNS :   void

  PROCESS :
              [1] Check which field is selected
              [2] Increment the relevant time unit
 *
 *F ----------------------------------------------------------------------------*/

void time_adv(char field) {
    if (field == 0) {
        minutes++;
    }
    if (field == 1) {
        hours++;
    }
    if (field == 2) {
        day++;
    }
    if (field == 3) {
        month++;
    }
    if (field == 4) {
        date++;
    }
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      alarm_update(char)

  DESCRIPTION : increments the alarm time in timeset

  INPUTS :    - field - this is the blinking number on the screen

  RETURNS :   void

  PROCESS :
              [1] Check which field is selected
              [2] Add a corresponding amount of time to the alarm time
 *
 *F ----------------------------------------------------------------------------*/

void alarm_update(char field) {
    if (field == 0) {          // field 0 = minutes
        alarmTime += MINUTE;
    } else if (field == 1) {   // field 1 = hours
        alarmTime += HOUR;
    }
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      get_time()

  DESCRIPTION : converts all of the units of time to a number in ms.

  INPUTS :    - mins, hours, seconds (global variables)

  RETURNS :   void

  PROCESS :
              [1] Sum all of the denominations multiplied by their size
 *
 *F ----------------------------------------------------------------------------*/

long get_time() {
    long longTime = (time + (minutes * MINUTE) + (hours * HOUR) + (date * DAY));
    return longTime;
}

/*F ----------------------------------------------------------------------------
 *
    NAME :     initialise_process(unsigned int, *funct)

  DESCRIPTION : Sets up a PCB with a program counter and function pointer
                so it begins at the start of the function when switched to

  INPUTS :    - process_index. Which process is it?
              - a pointer to the process function

  RETURNS :   void

  PROCESS :
              [1] Use the address of the function as a program counter, combine
	          with a status register in 2 words.
              [2] Save current stack pointer
              [3] Push a new stack pointer onto the stack
              [4] Followed by the program counter / status register
              [5] Followed by a bunch of zeros so the CPU registers are wiped 
	          when process is started
              [6] Save current stack pointer onto PCB
              [7] reload saved stack pointer
 *
 *F ----------------------------------------------------------------------------*/

void initialise_process(unsigned int process_index, void (*funct)()) {
    if (process_index < MAX_PROCESSES + 3) {
    // MAX_PROCESSES is max concurrent processes. 3 can be chosen from
        asm(
                " movx.a SR,&status\n"
            );
        stack_pointer = (LONG)&process[process_index] + STACK_SIZE - 2;
        program_counter = (LONG)funct;

        // Construct combined PC+SR used by interrupt
        pc1 = (WORD)program_counter;
        pc2 = (WORD)(((program_counter>>4)&0x0F000) | status&0x00FFF);
        asm(
                " movx.a sp,&saved_sp\n"
                " movx.a &stack_pointer,sp\n"
                " push.w &pc1\n"
                " push.w &pc2\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " push.a #0\n"
                " movx.a sp,&stack_pointer\n"
                " movx.a &saved_sp,sp\n"
            );
        process[process_index].sp = stack_pointer;
    }
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      run_process(int)

  DESCRIPTION : Starts a process running so it can be switched from

  INPUTS :    - index of process to start

  RETURNS :   void

  PROCESS :
              [1] Place the data from a PCD into the CPU registers
 *
 *F ----------------------------------------------------------------------------*/

void run_process(unsigned int process_index) {
    /**
     * Function to set an initialised process running. Timeslicing assumes a running process, so they
     * must be started manually
     */
    if (process_index < MAX_PROCESSES + 4) {
    stack_pointer = process[process_index].sp;
        asm(
                " movx.a &stack_pointer,SP \n"
                " pop.a R3 \n"
                " pop.a R4 \n"
                " pop.a R5 \n"
                " pop.a R6 \n"
                " pop.a R7 \n"
                " pop.a R8 \n"
                " pop.a R9 \n"
                " pop.a R10 \n"
                " pop.a R11 \n"
                " pop.a R12 \n"
                " pop.a R13 \n"
                " pop.a R14 \n"
                " pop.a R15 \n"
                " RETI \n"
        );
    }
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      setup()

  DESCRIPTION : Runs once to configure the MCU and intialise variables

  INPUTS :    None

  RETURNS :   void

  PROCESS :
              [1] Run intialisation code
 *
 *F ----------------------------------------------------------------------------*/

void setup() {
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    // Initialisation
    PM5CTL0 &= ~LOCKLPM5;  // Disable the GPIO power-on default high-impedance
                         // mode to activate previously configured port settings

    P1DIR |=  0x01;                 // Set P1.0 to output direction
    P4DIR |=  0x01;                 // Set P4.6 to output direction
    P1OUT &= ~0x01;                 // Set P1.0 off (Green LED)
    P4OUT &= ~0x01;                 // Set P4.6 off (Red LED)

    // button setup
    P1DIR &= ~(1 << 2);
    P1REN |= (1 << 2);
    P1OUT |= (1 << 2);
    P1IE  |= (1 << 2);      // Interrupt enable
    P1IES |= (1 << 2);     // Interrupt Edge Select
    P1IFG &= ~(1 << 2);    // Clear the interrupt flag just in case

    P2DIR &= ~(1 << 6);
    P2REN |= (1 << 6);
    P2OUT |= (1 << 6);
    P2IE  |= (1 << 6);
    P2IES |= (1 << 6);
    P2IFG &= ~(1 << 6);

    SFRRPCR = 0b1111;             // Reset pin control register. Resistor used,
                                  // Pullup, Falling Edge, NMI mode.
                                  // NMI = Non-Maskable Interrupt. Means this
                                  // interrupt is unaffected by 'general
                                  // interrupt enable' GIE.
    SFRIE1 |= (1 << 4);           // NMIIE - interrupts on NMI pins for reset
    SFRIFG1 &= ~(1 << 4);         // lower NMI interrupt flag

    TA1CCR0 = 128;                // Count up to 1024
    TA1CCTL0 = 0x10;              // Enable counter interrupts, bit 4=1
    TA1CTL =  TASSEL_2 + MC_0;    // Timer A using master clock, 1.1 MHz
    TA1R = 0;                     // Ensuring timer is empty

                                  // Timer A0 (1ms interrupt)
    TA0CCR0 =  1024;              // Count up to 1024

    _BIS_SR(GIE);                 // interrupts enabled


    // initialise global variables to 0

    currentState, current_process, stopwatchTime, lapTime, alarmTime, time, \
            minutes, day, date, month, startPressed, lapPressed, modePressed, \
            buttonEvent, alarmActive, chimeActive, alarmSetMode, lapMode, \
            monthMode, stopwatchRunning, selectedField = 0;
    STATE = CLOCK;
    hours = 12;  // start off at 12:00
    alarmTime = MINUTE + (12 * HOUR);
    alarmActive = 1;
}
