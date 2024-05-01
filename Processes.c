/**
 * Various processes that can be run concurrently
 */

#include "Stopwatch.h"

/*F ----------------------------------------------------------------------------
 *
    NAME :      clock()

  DESCRIPTION : Process that handles user interface in the clock mode.

  INPUTS :    - LOCATION OF BUFFER (global variable)
              - The message - long or short press
              - The target button

  RETURNS :   void

  PROCESS :
              [1] Wait for mail or for a minute to have expired
              [2] Work out which button was pressed
              [3] React accordingly
	      [4] Update the clock by converting the expired ms to mins and hrs
 *
 *F ----------------------------------------------------------------------------*/

void clock() {
    BYTE msg;
    BYTE ButtonAddress = 0;
    BYTE ButtonState = 0;
    for (;;) {
        do {
            msg = receive(ProcessID);  // Get msg from mailbox
        } while ((msg == EMPTY) && (time < MINUTE));
        // runs whenever a button is pressed or the clock needs updating

        ButtonAddress = msg & 0xC;  // Read address
        ButtonState = msg & 0x03;  // read status
        P4OUT ^= 1;
        if ((ButtonAddress) == START_BUTT) {  // Activity on start
           // ButtonHistory [START] = ButtonState;
            if (alarmSetMode == 0 && ButtonState == BUTT_PRESSED) {
           // BUTT_PRESSED checks that the button is currently down
                monthMode = 1;  // enter month mode
            } else {
                monthMode = 0;  // If button released leave month mode
            }
            if (alarmSetMode == 1 && ButtonState == BUTT_PRESSED) {
                if (get_time() > alarmTime) {
                // if alarm time has already expired set to next day
                    alarmTime = (alarmTime % DAY) + ((get_time() / DAY) + DAY);
                }
                alarmActive ^= 1;  // toggle alarm
            }
        }

        if (ButtonAddress == LAP_BUTT) {
            if (ButtonState == BUTT_PRESSED) {
                alarmSetMode = 1;  // Enter alarm set mode
            } else {
                alarmSetMode = 0;  // if no longer pressed, leave
            }
        }
        if (ButtonAddress == MODE_BUTT) {  // if mode pressed
            if (alarmSetMode == 1) {  // If lap button is also pressed
                chimeActive ^= 1;  // Toggle chime
            } else {
                ButtonAddress = 0;  // Clear message variables
                ButtonState = 0;
                STATE = TIMESET;  // Next state is timeset
                change_state();  // Update state
            }
        }
        ButtonAddress = 0;  // clear button variables
        ButtonState = 0;
        clock_update();  // update time
    }
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      timeset()

  DESCRIPTION : Process that handles user interface in the clock mode.

  INPUTS :    - mailbox (global variable)
              - bytes indicating: (global variables)
	          * Whether the alarm, date or time should be set
		  * Which field on the display is selected

  RETURNS :   void

  PROCESS :
              [1] Wait for mail
              [2] Work out which button was pressed
              [3] React accordingly
 *
 *F ----------------------------------------------------------------------------*/

void timeset() {
    selectedField = 0;
    BYTE msg;
    BYTE ButtonAddress;
    BYTE ButtonState;

    for (;;) {
        do {
            msg = receive(ProcessID);  // Get msg from mailbox
        } while (msg == EMPTY);

        ButtonAddress = msg & 0xC;  // Read address
        ButtonState = msg & 0x03;  // read status

        if (alarmSetMode == 0) {
            if (ButtonAddress == START_BUTT && ButtonState == BUTT_PRESSED) {
            // increment the value of the selected digit
                time_adv(selectedField % 5);
            }
            if (ButtonAddress == LAP_BUTT && ButtonState == BUTT_PRESSED) {
            // Change which digit is selected
                selectedField++;
                if ((selectedField % 5) < 3) {
                // The second two options are on a different view
                    monthMode = 0;
                } else {
                    monthMode = 1;
                }
            } else if (ButtonAddress == MODE_BUTT) {
              // Progress to alarm set mode
                alarmSetMode = 1;
                selectedField = 1;  // Start off on left-most digits
            }
        } else if (alarmSetMode == 1) {
	// Same as above, but the alarm only has two settable fields
            if (ButtonAddress == START_BUTT && ButtonState == BUTT_PRESSED) {
                alarm_update(selectedField % 2);
            } else if (ButtonAddress == LAP_BUTT && ButtonState == BUTT_PRESSED) {
                selectedField++;
            } else if (ButtonAddress == MODE_BUTT) {
            // Progress to stopwatch mode
                alarmSetMode = 0;
                STATE = CHRONO;
                change_state();
            }
        }
        clock_update();  // update time after each increment. Ensures rollover.
    }
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      alarm_check()

  DESCRIPTION : Process that runs in background. Checks whether to raise alarm.

  INPUTS :    - Bytes showing: (global variables)
                  * Whether the alarm and chime are active

  RETURNS :   void

  PROCESS :
              [1] Check whether chime or alarm should ring
              [2] Go to the alarm state if so
 *
 *F ----------------------------------------------------------------------------*/

void alarm_check() {
    for (;;) {
        while ((alarmActive == 0) & (chimeActive == 0));
        // do nothing if alarms and chimes not set
        if (alarmActive == 1) {
            if (get_time() > alarmTime) {
            // if alarm time has expired go to alarm state
                alarmActive = 0;
                currentState = STATE;  // Save current state first to return to
                STATE = ALARM;
                alarmTime += DAY;
                sixtySeconds = get_time() + MINUTE;
                change_state();
            }
            if (chimeActive == 1) {
                if ((minutes == 0) & (time < SECOND)) {
                // if it's the first second of a new minute, go to alarm state
                    alarmActive = 0;
                    currentState = STATE;
                    STATE = ALARM;
                    sixtySeconds = get_time() + MINUTE;
                    change_state();
                }
            }
        }
    }
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      alarm_ring()

  DESCRIPTION : Process that runs while alarm is ringing. Print "ALARM!" on LCD

  INPUTS :    - message box (global variable)
              - current time

  RETURNS :   void

  PROCESS :
              [1] Wait for a button press or 60 secs to elapse
              [2] If it was the lap button pressed, snooze
 *
 *F ----------------------------------------------------------------------------*/

void alarm_ring() {
    BYTE msg;
    for (;;) {
        do {
            msg = receive(ProcessID);  // Get msg from mailbox
        } while ((msg == EMPTY) && (get_time() < sixtySeconds));
	// Do nothing for 60 secs if there is no button press
	// Any button press will cancel the alarm
        if ((msg & 0xC) == LAP_BUTT) {
        // Lap button snoozes the alarm for 5 mins
            alarmTime += (MINUTE * 5); // 5 minutes snooze
        }
    // regardless of how we got here, button or timeout, return to prev state
        alarmActive = 1;
        STATE = currentState;
        change_state();
    }
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      stopwatch()

  DESCRIPTION : Process that handles user interface in the stopwatch mode

  INPUTS :    - message box (global variable)
              - global variable bytes to indicate:
	          * Whether the stopwatch is running
		  * Whether stopwatch is counting normally / showing laptime

  RETURNS :   void

  PROCESS :
              [1] Wait for mail
              [2] Work out which button was pressed
              [3] React accordingly
 *
 *F ----------------------------------------------------------------------------*/

void stopwatch() {
    BYTE msg;
    BYTE ButtonAddress;
    BYTE ButtonState;
    int prevTime = 0;
    for (;;) {
        do {
            msg = receive(ProcessID);  // Get msg from mailbox
        } while (msg == EMPTY);

        ButtonAddress = msg & 0xC;  // Read address
        ButtonState = msg & 0x03;  // read status

        if (stopwatchRunning == 0) {
            if ((lapMode == 0) && (ButtonAddress == START_BUTT) &&\
                                  (ButtonState == BUTT_PRESSED)) {
            // START/STOP was pushed while stopped, so run
                stopwatchRunning = 1;
            }
            if (ButtonAddress == LAP_BUTT && ButtonState == BUTT_PRESSED) {
            // Reset the stopwatch
                stopwatchTime = 0;
            }
        } else {
            if (ButtonAddress == START_BUTT && ButtonState == BUTT_PRESSED) {
            // START/STOP was pushed while running, so stop
                stopwatchRunning = 0;
                lapMode = 0;
            }
            if (ButtonAddress == LAP_BUTT && ButtonState == BUTT_PRESSED) {
            // Start showing laptimes
                lapMode = 1;
                lapTime = stopwatchTime - prevTime;  // time since last pressed
                prevTime = stopwatchTime;
            }
        }
        if (ButtonAddress == MODE_BUTT) {
            ButtonAddress = 0; // Clear message variables
            ButtonState = 0;
            stopwatchRunning = 0;
            STATE = CLOCK;
            change_state();
        }
    }
}

/*F ----------------------------------------------------------------------------
 *
    NAME :      update_LCD()

  DESCRIPTION : Process runs in the background to keep LCD up to date

  INPUTS :    - State (global variable)
              - bytes showing other operational details like alarm set (globals)

  RETURNS :   void

  PROCESS :
              [1] Check state
	      [2] Show the corresponding view on the LCD
 *
 *F ----------------------------------------------------------------------------*/

void update_LCD() {
    for (;;) {
        if (STATE == CHRONO) {
            display_stopwatch();
        }
        if (STATE == CLOCK) {
	// Clock has different views for setting alarm and checking date
            if (alarmSetMode == 1) {
                display_alarm();
            } else if (monthMode == 1) {
                display_month();
            } else {
                display_clock();
            }
        }
        if (STATE == TIMESET) {
            if (alarmSetMode == 0) {
                if (monthMode == 0) {
                    display_clock();
                } else {
                    display_month();
                }
                blink_digit(selectedField % 5 % 3);
		/** I should explain % 5 % 3.
		 * The selected field goes between two different views, time
		 * then date. Time has three fields and date has two. % 5 % 3
		 * goes through all 5 possible fields... but it does so
		 * irregularly.
		 *
		 * It goes through three for the time, then there are two left
		 * in the mod 5 so it resets to 0 from the mod 5 before the mod
		 * 3 can kick in.
		 *
		 * In short, the pattern goes
		 * 1 - 2 - 3 - 1 - 2
		 * and repeats
		 **/
            } else {
		// Alarm mode only has two fields. Much simpler!
                display_alarm();
                blink_digit(selectedField % 2);
            }
        }
        if (STATE == ALARM) {
            display_alert();
        }
        LCD_extras();
    }
}
