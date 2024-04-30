/**
 * This project implements a stopwatch on an MSP430FR4133 microcontroller.
 * It uses time slicing to run several concurrent processes and increments a
 * counter every time the interrupt triggers to swap processes. This counter
 * is used to determine the time on the stopwatch.
 *
 * This file contains the main loop as well as interrupt service routines.
 */
#include "Stopwatch.h"

/**
 * List of unused interrupt vectors as recommended in the MSP430 Student Guide. Chapter 5, Page 28
 * https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=2ahUKEwjGl4mhmuKEAxXH_7sIHcRoATc4ChAWegQIAhAB&url=https%3A%2F%2Fweb.eng.fiu.edu%2Fwatsonh%2Fintromicros%2FM6-Timers%2FTimersAndClocks%2FMSP430_Workshop_v4_01.pdf&usg=AOvVaw0bComI2nEMa9LVctmoKnIL&opi=89978449
 */

#pragma vector = ADC_VECTOR
#pragma vector = LCD_E_VECTOR
#pragma vector = USCI_B0_VECTOR
#pragma vector = USCI_A0_VECTOR
#pragma vector = RTC_VECTOR
#pragma vector = TIMER1_A1_VECTOR
#pragma vector = TIMER0_A1_VECTOR
#pragma vector = WDT_VECTOR
__interrupt void UNUSED_HWI_ISR(void)
{
    __no_operation();
}

// My port interrupt
#pragma vector = PORT1_VECTOR
__interrupt void Port_1(void)
{

    P1IFG &= ~(1 << 2);
    P1IE &= ~(1<<2);
    P1IES ^=  (1 << 2);
    TA1CCTL0 = 0x10;         // Enable counter interrupts, bit 4=1
    TA1CTL |= 1 << 5;        //Un-pause timer

    P1OUT ^= 0x01;
    P4OUT |= 1;


    if ((P1IN & (1 << 2)) != (1 << 2)) //here we can check if the button is up or down to determine the message
        {
            send(ProcessID, START_BUTT, BUTT_PRESSED);
        }
    else
        {
            send(ProcessID, START_BUTT, BUTT_RELEASED);
        }

    }


#pragma vector = PORT2_VECTOR
__interrupt void Port_2(void)
{

    P2IFG &= ~(1 << 6);
    P2IE &= ~(1 << 6);
    P2IES ^=  (1 << 6);
    TA1CCTL0 = 0x10;         // Enable counter interrupts, bit 4=1
    TA1CTL |= 1 << 5;           //Un-pause timer


    if ((P2IN & (1 << 6)) != (1 << 6)) //here we can check if the button is up or down to determine the message
        {
            send(ProcessID, LAP_BUTT, BUTT_PRESSED);
        }
    else
        {
            send(ProcessID, LAP_BUTT, BUTT_RELEASED);
        }


        P1OUT ^= 0x01;
        P4OUT |= 1;
        P2IE &= ~(1<<6);
    }

#pragma vector = UNMI_VECTOR
__interrupt void UNMI(void)
{

    SFRIFG1 &= ~(1 << 4);
    SFRIE1 &= ~(1 << 4);             // NMIIE - enable interrupts on NMI pins (for the reset button)
    TA1CCTL0 = 0x10;         // Enable counter interrupts, bit 4=1
    TA1CTL |= 1 << 5;

    send(ProcessID, MODE_BUTT, 0x00);
    P1OUT ^= 0x01;
    }



#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0(void) // Timer0 A0 1ms interrupt service routine
{

    TA1CCTL0 &= ~0x10;      // Stop interrupts
    TA1CTL &= ~(1 << 5);        // Pause timer
    TA1CCTL0 = 0x01;        //Reset interrupt flag
    TA1R = 0;               //Set timer back to 0


    P1IE |= 1<<2 ;
    P2IE |= 1<<6 ;
    SFRIE1 |= (1 << 4);             // NMIIE - enable interrupts on NMI pins (for the reset button)
}


#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer0_A0(void) // Timer0 A0 1ms interrupt service routine
{
    _disable_interrupts();
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
    // instead of this lot, we would equally swap between processes. Each process would check the mailbox for permission to run
    process[toggle_process].sp = stack_pointer; //toggle = current, current = offset;
    current_process = (current_process + 1) % MAX_PROCESSES;
    toggle_process = current_process;
    if (current_process == 2)
    {
        toggle_process += STATE;
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
    }
    time++;
    _enable_interrupts();
}

int main(void)
{
    setup();
    LCD_init();
    initialise_process(0, update_LCD);
    initialise_process(1, alarm_check);
    initialise_process(2, clock);
    initialise_process(3, timeset);
    initialise_process(4, stopwatch);
    initialise_process(5, alarmRing);
    TA0CCTL0 = 0x10;  // Enable counter interrupts, bit 4=1
    TA0CTL = TASSEL_2 + MC_1;  // Timer A using subsystem master clock, SMCLK(1.1 MHz)
    // and count UP to create a 1ms interrupt
    run_process(0);
    _BIS_SR(GIE); // interrupts enabled
    for (;;);
}
