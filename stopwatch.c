#include "stopwatch.h"

#pragma vector=ADC_VECTOR
#pragma vector=LCD_E_VECTOR
#pragma vector=USCI_B0_VECTOR
#pragma vector=USCI_A0_VECTOR               // List of unused interrupt vectors as recommended in the MSP430 Student Guide. Chapter 5, Page 28
#pragma vector=RTC_VECTOR                   // https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=2ahUKEwjGl4mhmuKEAxXH_7sIHcRoATc4ChAWegQIAhAB&url=https%3A%2F%2Fweb.eng.fiu.edu%2Fwatsonh%2Fintromicros%2FM6-Timers%2FTimersAndClocks%2FMSP430_Workshop_v4_01.pdf&usg=AOvVaw0bComI2nEMa9LVctmoKnIL&opi=89978449
#pragma vector=TIMER1_A1_VECTOR
#pragma vector=TIMER1_A0_VECTOR
#pragma vector=TIMER0_A1_VECTOR
#pragma vector=USB_UBM_VECTOR
#pragma vector=WDT_VECTOR
__interrupt void UNUSED_HWI_ISR (void)
{
__no_operation();
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    _disable_interrupts();
    startPressed = 1;
    buttonEvent = 1;
    P1IFG &= ~(1 << START_BUTT);
    _enable_interrupts();
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    _disable_interrupts();
    lapPressed = 1;
    buttonEvent = 1;
    P2IFG &= ~(1 << LAP_BUTT);
    _enable_interrupts();
}

// The NMI signal from the RST button also resets the JTAG. You have to remove the far right jumper for it to work, but then it won't enter the debugger.
// Page 20 for more info https://www.ti.com/lit/ug/slau320aj/slau320aj.pdf?ts=1709806533301
#pragma vector=UNMI_VECTOR
__interrupt void UNMI(void) {
    _disable_interrupts();
    modePressed = 1;
    buttonEvent = 1;
    SFRIFG1 &= ~(1 << 4);
    _enable_interrupts();
}

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
    if (stopwatchRunning == 1) {
        time++;
    }
}

int main(void)
{
    setup();
    LCD_init();
    current_process = 0;
    startPressed, lapPressed, modePressed, buttonEvent = 0;
    STATE = CHRONO;
    initialise_process(0, stopwatch);
    initialise_process(1, update_LCD);
    run_process(current_process);
    _BIS_SR(GIE);                   // interrupts enabled

    for (;;)
    {

    }

    return 0;
}
