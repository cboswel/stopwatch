#include "stopwatch.h"

void red_led()
{
    volatile unsigned int i;             // make sure i is not optimized way

    for (;;)
    {
    P4OUT |=  0x01;                 // Set P4.0 on  (Red LED)
        for (i=0; i<10000; i++)
        {
        }

    P4OUT &= ~0x01;                 // Set P4.0 off (Red LED)
        for (i=0; i<10000; i++)
        {
        }
    }
}

void green_led()
{
    volatile unsigned int i;             // make sure i is not optimized away

    for (;;)
    {
      P1OUT |=  0x01;               // Set P1.0 on  (Green LED)
        for (i=0; i<20000; i++)
        {
        }

      P1OUT &= ~0x01;               // Set P1.0 off (Green LED)
        for (i=0; i<20000; i++)
        {
        }
    }
}

void wait(char* s) {
    volatile int z;
    do
    {
        _enable_interrupts();
        z++;
        _disable_interrupts();
    } while(*s==0);
    *s = 0;
    _enable_interrupts();
}

void update_LCD() {
    for (;;) {
       if (STATE == CHRONO) {
           int ms = time % 1000;
           int s = (time / 1000) % 60;
           int min = (time / (1000 * 60)) % 60;

           show_digit((min / 10), 0);
           show_digit((min % 10), 1);
           show_digit((s / 10), 2);
           show_digit((s % 10), 3);
           show_digit((ms / 100), 4);
           show_digit(((ms /10 )% 10), 5);
       }
       if (STATE == LAP) {
           int ms = lapTime % 1000;
           int s = (lapTime / 1000) % 60;
           int min = (lapTime / (1000 * 60)) % 60;

           show_digit((min / 10), 0);
           show_digit((min % 10), 1);
           show_digit((s / 10), 2);
           show_digit((s % 10), 3);
           show_digit((ms / 100), 4);
           show_digit(((ms /10 )% 10), 5);
       }
    }
}

void stopwatch() {
    time = 0;
    lapTime = 0;
    int prevTime = 0;
    for (;;) {
        wait(&buttonEvent);
        if (stopwatchRunning == 0) {
            if (startPressed == 1) {
                startPressed = 0;
                if (STATE == CHRONO) {
                    stopwatchRunning = 1;
                }
            }
            if (lapPressed == 1) {
                lapPressed = 0;
                time = 0;
            }
        }
        else if (stopwatchRunning == 1) {
            if (startPressed == 1) {
                startPressed == 0;
                stopwatchRunning = 0;
                STATE = CHRONO;
            }
            if (lapPressed == 1) {
                lapPressed = 0;
                STATE = LAP;
                lapTime = time - prevTime;
                prevTime = time;
            }
        }
    }
}

void idle() {
    /**
     *  Sometimes we might need a process to just do nothing so there is
     *  a deterministic amount of time spent on the other time sensitive
     *  processes. If we're really clever, we will implement a way to
     *  spawn processes while tracking how many are running, and
     *  change our timing calculations accordingly.
     */
    for (;;);
}

void initialise_process(unsigned int process_index, void (*funct)())
{
    if (process_index < MAX_PROCESSES)
    {
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

void run_process(unsigned int process_index)
{
    if (process_index < MAX_PROCESSES)
    {
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

void setup()
{
    WDTCTL = WDTPW | WDTHOLD;       // Stop watchdog timer

    // Initialisation
    PM5CTL0 &= ~LOCKLPM5;           // Disable the GPIO power-on default high-impedance mode
                                    // to activate previously configured port settings

    P1DIR |=  0x01;                 // Set P1.0 to output direction
    P4DIR |=  0x01;                 // Set P4.6 to output direction
    P1OUT &= ~0x01;                 // Set P1.0 off (Green LED)
    P4OUT &= ~0x01;                 // Set P4.6 off (Red LED)

    // button setup
    P1DIR &= ~(1 << START_BUTT);
    P1REN |= (1 << START_BUTT);
    P1OUT |= (1 << START_BUTT);
    P1IE |= (1 << START_BUTT);      // Interrupt enable
    P1IES |= (1 << START_BUTT);     // Interrupt Edge Select
    P1IFG &= ~(1 << START_BUTT);    // Clear trhe interrupt flag just in case

    P2DIR &= ~(1 << LAP_BUTT);
    P2REN |= (1 << LAP_BUTT);
    P2OUT |= (1 << LAP_BUTT);
    P2IE |= (1 << LAP_BUTT);
    P2IES |=(1 << LAP_BUTT);
    P2IFG &= ~(1 << LAP_BUTT);

    SFRRPCR = 0b1111;              // Reset pin control register. Resistor used, Pullup, Falling Edge, NMI mode.
    // NMI = Non-Maskable Interrupt. Means this interrupt is unaffected by 'general interrupt enable' GIE.
    SFRIE1 |= (1 << 4);             // NMIIE - enable interrupts on NMI pins (for the reset button)
    SFRIFG1 &= ~(1 << 4);           // This is how you lower an NMI interrupt flag


                                    // Timer A0 (1ms interrupt)
    TA0CCR0 =  1024;                // Count up to 1024
    TA0CCTL0 = 0x10;                // Enable counter interrupts, bit 4=1
    TA0CTL =  TASSEL_2 + MC_1;      // Timer A using subsystem master clock, SMCLK(1.1 MHz)
                                    // and count UP to create a 1ms interrupt


    _BIS_SR(GIE);                   // interrupts enabled
}
