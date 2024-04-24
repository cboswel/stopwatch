/**
 * Miscellaneous functions that running processes can call to mimimise
 * repeated code.
 */

#include <MyStopwatch.h>


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

void send(BYTE RecipientID, BYTE ButtonType, BYTE ButtonState){

    if(buffer == EMPTY){
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
BYTE receive(BYTE RecipientID) { //BYTE address

    BYTE temp;
    temp = RecipientID;
    BYTE tempMod;
    tempMod = buffer >> 4;
    if((buffer != EMPTY) && (RecipientID == tempMod)) {
        BYTE message = buffer;
        buffer = EMPTY; // Reset buffer to EMPTY
        _enable_interrupts();
        return message;

    }
    return 0;
}

/**/

void wait(volatile char* s) {
    volatile int z;
    do {
        _enable_interrupts();
        z++;
        _disable_interrupts();
    } while ( *s == 0 );
    *s = 0;
    _enable_interrupts();
}

void clock_update() {
    minutes += (time / MINUTE);
    hours += (minutes / 60);
    day += (hours / 24);
    date += (hours / 24);
    month += (day / monthLength[month]);

    minutes %= 60;
    hours %= 24;
    day %= 7;
    date %= monthLength[month];
    month %= 12;

    time %= MINUTE;
}

void display_stopwatch() {
    /**
     * Displays the stopwatch view: Minutes, seconds, milliseconds.
     */
    int ms = stopwatchTime % 1000;
    int s = (stopwatchTime / 1000) % 60;
    int min = (stopwatchTime / (1000 * 60)) % 60;
    if (lapMode == 1) {
        ms = lapTime % 1000;
        s = (lapTime / 1000) % 60;
        min = (lapTime / (1000 * 60)) % 60;
    }
    show_digit((min / 10), 0);
    show_digit((min % 10), 1);
    show_digit((s / 10), 2);
    show_digit((s % 10), 3);
    show_digit((ms / 100), 4);
    show_digit(((ms / 10) % 10), 5);
}

void display_clock() {
    /**
     * Displays a view of a clock, hours and minutes with a text day name e.g. Mo
     */
    show_digit((hours / 10), 0);
    show_digit((hours % 10), 1);
    show_digit((minutes / 10), 2);
    show_digit((minutes % 10), 3);
    show_digit(days[day][0], 4);
    show_digit(days[day][1], 5);
}

void display_month() {
    /**
     * Displays the calendar view: Day of the month, month, text day name
     */
    show_digit(((date + 1) / 10), 0);    // +1 because of zero indexing
    show_digit(((date + 1) % 10), 1);
    show_digit((month + 1 / 10), 2);
    show_digit((month + 1 % 10), 3);
    show_digit(days[day][0], 4);
    show_digit(days[day][1], 5);
}

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

void change_state() {
    /**
     * Change between the different states that the MODE button cycles through
     */
    if (STATE == CHRONO) {
        LCDBLKCTL &= ~(0b11);   // No blinking
    }
    else if (STATE == TIMESET) {
        selectedField = 0;
        LCDBLKCTL |= 0b11;   // Start alternating mode blinking
    }
    else if (STATE == CLOCK) {
        LCDBLKCTL &= ~(0b11);   // No blinking
    }
    else if (STATE == ALARM) {
        LCDBLKCTL |= 0b11;   // Start alternating mode blinking
    }
}

void timeAdv(char field) {
    /**
     * Used to increment whichever unit of time the user currently has selected during timeset mode.
     */
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
        date++;
    }
    if (field == 4) {
        month++;
    }
}

void alarm_update(char field) {
    if (field == 0) {          // field 0 = minutes
        alarmTime += MINUTE;
    } else if (field == 1) {   // field 1 = hours
        alarmTime += HOUR;
    }
}

long getTime() {
    int mins = 60 * 1000;
    int hrs = 60 * mins;
    int dys = 24 * hrs;
    long longTime = (time + (minutes * mins) + (hours * hrs) + (date * dys));
    return longTime;
}

void initialise_process(unsigned int process_index, void (*funct)()) {
    /**
     * Function to add a function pointer to the start of a Process Control Block and to wipe it
     */
    if (process_index < MAX_PROCESSES + 4) {
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

void setup() {
    /**
     * Initialisation code that runs once when device is powered on
     */
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    // Initialisation
    PM5CTL0 &= ~LOCKLPM5;           // Disable the GPIO power-on default high-impedance mode
                                    // to activate previously configured port settings

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

    SFRRPCR = 0b1111;              // Reset pin control register. Resistor used, Pullup, Falling Edge, NMI mode.
                                    // NMI = Non-Maskable Interrupt. Means this interrupt is unaffected by 'general interrupt enable' GIE.
    SFRIE1 |= (1 << 4);             // NMIIE - enable interrupts on NMI pins (for the reset button)
    SFRIFG1 &= ~(1 << 4);           // This is how you lower an NMI interrupt flag

    TA1CCR0 = 128;                  // Count up to 1024
    //TA1CCTL0 = 0x10;                // Enable counter interrupts, bit 4=1
    TA1CTL =  TASSEL_2 + MC_0;      // Timer A using subsystem master clock, SMCLK(1.1 MHz)
    TA1R = 0;                      //Ensuring timer is empty


                                    // Timer A0 (1ms interrupt)
    TA0CCR0 =  1024;                // Count up to 1024



    _BIS_SR(GIE);                   // interrupts enabled


    // initialise global variables to 0

    currentState, current_process, stopwatchTime, lapTime, alarmTime, time, minutes, day, date,\
    month, startPressed, lapPressed, modePressed, buttonEvent, alarmActive, chimeActive,\
    alarmSetMode, lapMode, monthMode, stopwatchRunning, selectedField = 0;
    STATE = CLOCK;
    hours = 12; // start off at 12:00
}