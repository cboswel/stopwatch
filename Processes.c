/**
 * Various processes that can be run concurrently
 */

#include <Stopwatch.h>

void clock() {
    /**
     * Runs in parallel with the clock state machine to keep the clock up to date.
     * Every ms a counter is incremented, called time. Regardless of state or mode.
     * When we run this process, we look at how many ms have elapsed and update m, hr, day accordingly.
     * We then remove any minutes that we counted from the ms timer.
     */
    for (;;) {
        if (time > MINUTE) {   // Only act after at least a minute has elapsed (60,000 ms)
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
    }
}

void clockState() {
    /**
     * Process to pick up user input and to modify behaviour accordingly.
     */
    for (;;) {
        wait(&buttonEvent);
        if (startPressed == 1) {
            if ((P1IN & (1 << START_BUTT)) == 0) {
                monthMode = 1;
            }
            else {
                monthMode = 0;
            }
            if ((P2IN & (1 << LAP_BUTT)) == 0) {
                if (getTime() > alarmTime) {
                    alarmTime = (alarmTime % DAY) + ((getTime() / DAY) + DAY);
                }
                alarmActive ^= 1;
            }
            else {
                STATE = CLOCK;
            }
        }
        else if (lapPressed == 1) {
            if ((P2IN & (1 << LAP_BUTT)) == 0) {
                alarmSetMode = 1;
            }
            else {
                alarmSetMode = 0;
            }
        }
        else if (modePressed == 1) {
            alarmSetMode = 0;
            modePressed = 0;
            buttonEvent = 0;
            if ((P2IN & (1 << LAP_BUTT)) == 0) {
                chimeActive ^= 1;
            }
            else {
               change_state(TIMESET);
            }
        }
        buttonEvent = 0;
        modePressed = 0;
        startPressed = 0;
        lapPressed = 0;
    }
}

void timeset() {
    /**
     * Process to run during the time set mode. Receives user input and uses the timeAdv function
     * to increment the value of each settable unit of time.
     */
    for (;;) {
        selectedField = 0;
        wait(&buttonEvent);
        if (alarmSetMode == 0) {
            if (startPressed == 1) {
                timeAdv(selectedField % 5);
            }
            else if (lapPressed == 1) {
                selectedField++;
            }
            else if (modePressed == 1) {
                alarmSetMode = 1;
                selectedField = 1;      // Start off on left-most digits
            }
        }
        else if (alarmSetMode == 1) {
            if (startPressed == 1) {
                alarm_update(selectedField % 2);
            }
            else if (lapPressed == 1) {
                selectedField++;
            }
            else if (modePressed == 1) {
                modePressed = 0;
                buttonEvent = 0;
                alarmSetMode = 0;
                change_state(CHRONO);
            }
        }
        modePressed = 0;
        startPressed = 0;
        lapPressed = 0;
        buttonEvent = 0;
    }
}

void alarmCheck() {
    for (;;) {
        while ((alarmActive == 0) & (chimeActive == 0)); // do nothing if alarms and chimes not set
        if (alarmActive == 1) {
            if (getTime() > alarmTime) {
                currentState = STATE;
                change_state(ALARM);
            }
            if (chimeActive == 1) {
                if ((minutes == 0) & (time < SECOND)) {
                    currentState = STATE;
                    change_state(ALARM);
                }
            }
        }
    }
}

void alarmRing() {
    long sixtySeconds = getTime() + MINUTE;
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
            if (startPressed == 1) {
                startPressed = 0;
                if (STATE == CHRONO) {
                    stopwatchRunning = 1;
                }
            }
            if (lapPressed == 1) {
                stopwatchTime = 0;
            }
        }
        else if (stopwatchRunning == 1) {
            if (startPressed == 1) {
                stopwatchRunning = 0;
                STATE = CHRONO;
            }
            if (lapPressed == 1) {
                STATE = LAP;
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
            change_state(CLOCK);
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
           if (alarmSetMode == 1) {
               display_alarm();
           }
           else if (monthMode == 0) {
               display_clock();
           }
           else {
               display_month();
           }
           blink_digit(selectedField % 5 % 3);
       }
       if (STATE == ALARM) {
           display_alert();
       }
       LCD_extras();
    }
}
