/**
 * This project implements a stopwatch on an MSP430FR4133 microcontroller.
 * It uses time slicing to run several concurrent processes and increments a
 * counter every time the interrupt triggers to swap processes. This counter
 * is used to determine the time on the stopwatch.
 *
 * This file contains the main loop as well as interrupt service routines.
 */
#include <Stopwatch.h>

/**
 * List of unused interrupt vectors as recommended in the MSP430 Student Guide. Chapter 5, Page 28
 * https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=2ahUKEwjGl4mhmuKEAxXH_7sIHcRoATc4ChAWegQIAhAB&url=https%3A%2F%2Fweb.eng.fiu.edu%2Fwatsonh%2Fintromicros%2FM6-Timers%2FTimersAndClocks%2FMSP430_Workshop_v4_01.pdf&usg=AOvVaw0bComI2nEMa9LVctmoKnIL&opi=89978449
 */

#pragma vector=ADC_VECTOR
#pragma vector=LCD_E_VECTOR
#pragma vector=USCI_B0_VECTOR
#pragma vector=USCI_A0_VECTOR
#pragma vector=RTC_VECTOR
#pragma vector=TIMER1_A1_VECTOR
#pragma vector=TIMER1_A0_VECTOR
#pragma vector=TIMER0_A1_VECTOR
#pragma vector=WDT_VECTOR
__interrupt void UNUSED_HWI_ISR (void)
{
__no_operation();
}

#pragma vector=PORT1_VECTOR
/**
 * ISR for the START/STOP button on port 1
 */
__interrupt void Port_1(void)
{
    _disable_interrupts();
    buttonEvent = 1;
    startPressed = 1;
    P1IES ^= (1 << START_BUTT);
    P1IFG &= ~(1 << START_BUTT);
    _enable_interrupts();
}

#pragma vector=PORT2_VECTOR
/**
 * ISR for the LAP/RESET button on port 2
 */
__interrupt void Port_2(void)
{
    _disable_interrupts();
    buttonEvent = 1;
    lapPressed = 1;
    P2IES ^= (1 << LAP_BUTT);
    P2IFG &= ~(1 << LAP_BUTT);
    _enable_interrupts();
}


#pragma vector=UNMI_VECTOR
/**
 * ISR for the MODE button on the RST button.
 * The NMI signal from the RST button also resets the JTAG. You have to
 * remove the far right jumper for it to work, but then it won't enter the debugger.
 * Page 20 for more info https://www.ti.com/lit/ug/slau320aj/slau320aj.pdf?ts=1709806533301
 */
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

    process[toggle_process].sp = stack_pointer;
    current_process = (current_process+1) % MAX_PROCESSES;
    toggle_process = current_process;
    if (current_process == 3) {
        toggle_process += toggle;
    }
    stack_pointer = process[toggle_process].sp;

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
        stopwatchTime++;
        time++;
    }
    else {
        time++;
    }
}

int main(void)
{
    setup();
    LCD_init();
    initialise_process(0, update_LCD);
    initialise_process(1, alarmCheck);
    initialise_process(2, clock);
    initialise_process(3, clockState);
    initialise_process(4, timeset);
    initialise_process(5, stopwatch);
    initialise_process(6, alarmRing);
    TA0CCTL0 = 0x10;                // Enable counter interrupts, bit 4=1
    TA0CTL =  TASSEL_2 + MC_1;      // Timer A using subsystem master clock, SMCLK(1.1 MHz)
                                    // and count UP to create a 1ms interrupt
    run_process(0);
    _BIS_SR(GIE);                  // interrupts enabled
    for (;;);
}
