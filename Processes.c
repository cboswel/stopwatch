/**
 * Various processes that can be run concurrently
 */

#include <Stopwatch.h>

void clock() {
    /**
     * Process to pick up user input and to modify behaviour accordingly.
     */
    for (;;) {
        while (buttonEvent == 0 && time < MINUTE);
        if (startPressed == 1) {
            if (alarmSetMode == 0 && (P1IN & (1 << START_BUTT)) == 0) {
                monthMode = 1;
            }
            else {
                monthMode = 0;
            }
            if (alarmSetMode == 1 && (P1IN & (1 << START_BUTT)) == 0) {
                if (getTime() > alarmTime) {
                    alarmTime = (alarmTime % DAY) + ((getTime() / DAY) + DAY);
                }
                alarmActive ^= 1;
            }
        }
        if (lapPressed == 1) {
            if ((P2IN & (1 << LAP_BUTT)) == 0) {
                alarmSetMode = 1;
            }
            else {
                alarmSetMode = 0;
            }
        }
        if (modePressed == 1) {
            alarmSetMode = 0;
            modePressed = 0;
            buttonEvent = 0;
            if ((P2IN & (1 << LAP_BUTT)) == 0) {
                chimeActive ^= 1;
            }
            else {
                STATE = TIMESET;
                change_state();
            }
        }
        buttonEvent = 0;
        modePressed = 0;
        startPressed = 0;
        lapPressed = 0;
        clock_update();
    }
}

void timeset() {
    /**
     * Process to run during the time set mode. Receives user input and uses the timeAdv function
     * to increment the value of each settable unit of time.
     */
    selectedField = 0;
    for (;;) {
        wait(&buttonEvent);
        if (alarmSetMode == 0) {
            if ((startPressed == 1) && (P1IN & (1 << START_BUTT)) == 0) {
                timeAdv(selectedField % 5);
            }
            if ((lapPressed == 1) && (P2IN & (1 << LAP_BUTT)) == 0) {
                selectedField++;
                if ((selectedField % 5) < 3) {
                    monthMode = 0;
                } else {
                    monthMode = 1;
                }
            }
            else if (modePressed == 1) {
                modePressed =  0;
                alarmSetMode = 1;
                selectedField = 1;      // Start off on left-most digits
            }
        }
        else if (alarmSetMode == 1) {
            if ((startPressed == 1) && (P1IN & (1 << START_BUTT)) == 0) {
                alarm_update(selectedField % 2);
            }
            else if ((lapPressed == 1) && (P2IN & (1 << LAP_BUTT)) == 0) {
                selectedField++;
            }
            else if (modePressed == 1) {
                modePressed = 0;
                buttonEvent = 0;
                alarmSetMode = 0;
                STATE = CHRONO;
                change_state();
            }
        }
        modePressed = 0;
        startPressed = 0;
        lapPressed = 0;
        buttonEvent = 0;
        clock_update();
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
    int prevTime = 0;
    for (;;) {
        wait(&buttonEvent);
        if (stopwatchRunning == 0) {
            if ((lapMode == 0) && (P1IN & (1 << START_BUTT)) == 0) {
                stopwatchRunning = 1;
            }
            if ((P2IN & (1 << LAP_BUTT)) == 0) {
                stopwatchTime = 0;
            }
        }
        else {
            if ((P1IN & (1 << START_BUTT)) == 0) {
                stopwatchRunning = 0;
                lapMode = 0;
            }
            if ((P2IN & (1 << LAP_BUTT)) == 0) {
                lapMode = 1;
                lapTime = stopwatchTime - prevTime;
                prevTime = stopwatchTime;
            }
        }
        if (modePressed == 1) {
            modePressed = 0;
            startPressed = 0;
            lapPressed = 0;
            buttonEvent = 0;
            stopwatchRunning = 0;
            STATE = CLOCK;
            change_state();
        }
        modePressed = 0;
        startPressed = 0;
        lapPressed = 0;
        buttonEvent = 0;
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
           }
           else if (monthMode == 1) {
               display_month();
           }
           else {
               display_clock();
           }
       }
       if (STATE == TIMESET) {
           if (alarmSetMode == 0) {
               if (monthMode == 0) {
                   display_clock();
               }
               else {
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
