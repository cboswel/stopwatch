#include "stopwatch.h"


#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void)    // Timer0 A0 1ms interrupt service routine
{
    // Save first process details...
    asm(
            " push.a R10\n"
            " push.a R9\n"
            " push.a R8\n"
            " push.a R7\n"
            " push.a R6\n"
            " push.a R5\n"
            " push.a R4\n"
            " push.a R3\n"
            " movx.a sp,&stack_pointer\n"
        );

    process[current_process].sp = stack_pointer;
    current_process = (current_process+1) % MAX_PROCESSES;
    stack_pointer = process[current_process].sp;

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
    );
    time++;
}

int main(void)
{
    setup();
    LCD_init();
    current_process = 0;
    STATE = CHRONO;
    initialise_process(0, red_led);
    initialise_process(1, update_LCD);
    run_process(current_process);
    _BIS_SR(GIE);                   // interrupts enabled

    for (;;)
    {

    }

    return 0;
}
