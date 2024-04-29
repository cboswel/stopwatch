/**
 * Various processes that can be run concurrently
 */

#include <MyStopwatch.h>

void clock() {
    /**
     * Process to pick up user input and to modify behaviour accordingly.
     */
    BYTE msg;
    BYTE ButtonAddress = 0;
    BYTE ButtonState = 0;
    //BYTE ButtonHistory [2] = {0,0}; //0 is not pressed, 1 is pressed
    //enum Index {START, LAP};
    for (;;) {
        do {

            msg = receive(ProcessID); // Get msg from mailbox

        } while (msg == EMPTY);

        ButtonAddress = msg & 0xC; // Read address
        ButtonState = msg & 0x03; // read status
        P4OUT ^= 1;
        if ((ButtonAddress) == START_BUTT) { // Activity on start
           // ButtonHistory [START] = ButtonState;
            if (alarmSetMode == 0 && ButtonState == BUTT_PRESSED) { // check if pressed
                monthMode = 1; // enter month mode
            } else {
                monthMode = 0; // If button released leave month mode
            }
            if (alarmSetMode == 1 && ButtonState == BUTT_PRESSED) { // If in alarm set mode, and the start pressed
                if (getTime() > alarmTime) { // Check if alarm time has already occured that day
                    alarmTime = (alarmTime % DAY) + ((getTime() / DAY) + DAY); // Set alarm to next day
                }
                alarmActive ^= 1; // toggle alarm
            }
        }

        if (ButtonAddress == LAP_BUTT) { // Activity on lap
            if (ButtonState == BUTT_PRESSED) { // If pressed
                alarmSetMode = 1; // Enter alarm set mode
              //  P2OUT |= 1<<6;
            } else {
                alarmSetMode = 0; // if no longer pressed, leave
               // P2OUT &= ~(0x01);
            }
        }
        if (ButtonAddress == MODE_BUTT) { // if mode pressed

            if (alarmSetMode == 1) { // If lap button is also pressed
                chimeActive ^= 1; // Toggle chime
            }
            else {
                alarmSetMode = 0; // leave alarm set mode
                ButtonAddress = 0; // Clear message variables
                ButtonState = 0;
                STATE = TIMESET; // If lap button not also pressed, enter Timeset
                change_state(); // Update state
            }
        }
        ButtonAddress = 0; // clear button variables
        ButtonState = 0;
        clock_update(); // update time
}
}
void timeset() {
    selectedField = 0;
    BYTE msg;
    BYTE ButtonAddress;
    BYTE ButtonState;

    for (;;) {
        do {
            msg = receive(ProcessID); // Get msg from mailbox
        } while (msg == EMPTY);

        ButtonAddress = msg & 0xC; // Read address
        ButtonState = msg & 0x03; // read status

        if (alarmSetMode == 0) {
            if (ButtonAddress == START_BUTT && ButtonState == BUTT_PRESSED) {
                timeAdv(selectedField % 5);
            }
            if (ButtonAddress == LAP_BUTT && ButtonState == BUTT_PRESSED) {
                selectedField++;
                if ((selectedField % 5) < 3) {
                    monthMode = 0;
                } else {
                    monthMode = 1;
                }
            } else if (ButtonAddress == MODE_BUTT) {
                alarmSetMode = 1;
                selectedField = 1; // Start off on left-most digits
            }
        } else if (alarmSetMode == 1) {
            if (ButtonAddress == START_BUTT && ButtonState == BUTT_PRESSED) {
                alarm_update(selectedField % 2);
            } else if (ButtonAddress == LAP_BUTT && ButtonState == BUTT_PRESSED) {
                selectedField++;
            } else if (ButtonAddress == MODE_BUTT) {
                alarmSetMode = 0;
                STATE = CHRONO;
                change_state();
            }
        }
        clock_update(); // update time
    }
}

void alarmCheck() {
    for (;;) {
        while ((alarmActive == 0) & (chimeActive == 0)); // do nothing if alarms and chimes not set
        if (alarmActive == 1) {
            if (getTime() > alarmTime) {
                currentState = STATE;
                STATE = ALARM;
                change_state();
            }
            if (chimeActive == 1) {
                if ((minutes == 0) & (time < SECOND)) {
                    currentState = STATE;
                    STATE = ALARM;
                    change_state();
                }
            }
        }
    }
}

void alarmRing() {
    long sixtySeconds = getTime() + MINUTE;
    STATE = ALARM;
    while (getTime() < sixtySeconds | buttonEvent == 0);
    buttonEvent = 0;
    if ((lapPressed == 1) & (alarmTime > getTime())) {
        alarmTime += MINUTE * 5; // 5 minutes snooze
    }
    lapPressed = 0;
    modePressed = 0;
    startPressed = 0; // Just change state below to stop alarm
    STATE = currentState;
}

void stopwatch() {
    /**
     * Process to accept user input during the stopwatch mode of operation.
     */

    BYTE msg;
    BYTE ButtonAddress;
    BYTE ButtonState;
    int prevTime = 0;
    for (;;) {
        do {
            msg = receive(ProcessID); // Get msg from mailbox
        } while (msg == EMPTY);

        ButtonAddress = msg & 0xC; // Read address
        ButtonState = msg & 0x03; // read status

        if (stopwatchRunning == 0) {
            if ((lapMode == 0) && (ButtonAddress == START_BUTT) && (ButtonState == BUTT_PRESSED)) {
                stopwatchRunning = 1;
            }
            if (ButtonAddress == LAP_BUTT && ButtonState == BUTT_PRESSED) {
                stopwatchTime = 0;
            }
        } else {
            if (ButtonAddress == START_BUTT && ButtonState == BUTT_PRESSED) {
                stopwatchRunning = 0;
                lapMode = 0;
            }
            if (ButtonAddress == LAP_BUTT && ButtonState == BUTT_PRESSED) {
                lapMode = 1;
                lapTime = stopwatchTime - prevTime;
                prevTime = stopwatchTime;
            }
        }
        if (modePressed == 1) {
            ButtonAddress = 0; // Clear message variables
            ButtonState = 0;
            /**
            STATE = TIMESET; // If lap button not also pressed, enter Timeset
            change_state(); // Update state
            **/
            stopwatchRunning = 0;
            STATE = CLOCK;
            change_state();
        }
    }
}

void update_LCD() {
    /**
     * This process runs constantly to keep the display active.
     */
    for (;;) {
        if (STATE == CHRONO) {
            display_stopwatch();
        }
        if (STATE == CLOCK) {
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
            } else {
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
