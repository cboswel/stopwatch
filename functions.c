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

void update_LCD() {
    time = 0;
    for (;;) {
       if (STATE == CHRONO) {
           int ms = time % 100;
           int s = (time / 100) % 60;
           int min = (time / (100 * 60)) % 60;

           show_digit((min / 10) + '0', 0);
           show_digit((min % 10) + '0', 1);
           show_digit((s / 10) + '0', 2);
           show_digit((s % 10) + '0', 3);
           show_digit((ms / 10) + '0', 4);
           show_digit((ms % 10) + '0', 5);
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
    P1DIR &= ~(1 << 3);
    P1REN |= (1 << 3);
    P1OUT |= (1 <<3);

                                    // Timer A0 (1ms interrupt)
    TA0CCR0 =  1024;                // Count up to 1024
    TA0CCTL0 = 0x10;                // Enable counter interrupts, bit 4=1
    TA0CTL =  TASSEL_2 + MC_1;      // Timer A using subsystem master clock, SMCLK(1.1 MHz)
                                    // and count UP to create a 1ms interrupt

    _BIS_SR(GIE);                   // interrupts enabled
}
