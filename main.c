#include <msp430.h> 
/**
 * main.c
 */
volatile int i;
volatile int comm;
void show_digit(char character, int pos) {
    /**
     * This is the part that needs some work!
     *
     * I've done some, and this code should run. I want it to scroll through
     * all the numbers and letters we'll need. As you'll see, most of them still
     * need programming!
     *
     * So, the job is to fill in these 8-bit binary numbers to make the right
     * segments light up. A sheet like this might help:
     * https://i0.wp.com/www.partsnotincluded.com/wp-content/uploads/2017/08/14-Segment-ASCII-All.jpg?ssl=1
     * But don't be afraid of being artistic if you wanna make some unique ones :)
     *
     * There are two binary numbers. The first is like a 7-seg display:
     *   _
     *  |_|   I think it goes clockwise from the top, then there are two middle parts
     *  |_|
     *
     *  The second is for the diagonal lines in the middle kinda like this:
     *
     *  \/
     *  |   Two in the middle, then there's other stuff like colons and minuses.
     *  /\
     *
     *  Give it a go!
     */
    char ans[2] = {0, 0};
    if (character == '9') {
        ans[0] = 0b11110111;
        ans[1] = 0b00000000;
    }
    else if (character == '8') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == '7') {
        ans[0] = 0b10000000;
        ans[1] = 0b00100100;
    }
    else if (character == '6') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == '5') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == '4') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == '3') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == '2') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == '1') {
        ans[0] = 0b11111111;
        ans[1] = 0b00000000;
    }
    else if (character == '0') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    // and letters for days of week
    else if (character == 'M') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == 'T') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == 'W') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == 'F') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == 'S') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == 'o') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == 'u') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == 'e') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == 'h') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == 'r') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }
    else if (character == 'a') {
        ans[0] = 0b00000000;
        ans[1] = 0b00000000;
    }

    // This part draws it in the right spot, should work already
    if (pos == 0) {
        LCDM4 = ans[0];
        LCDM5 = ans[1];
    }
    else if (pos == 1) {
        LCDM6 = ans[0];
        LCDM7 = ans[1];
    }
    else if (pos == 2) {
        LCDM8 = ans[0];
        LCDM9 = ans[1];
    }
    else if (pos == 3) {
        LCDM10 = ans[0];
        LCDM11 = ans[1];
    }
    else if (pos == 4) {
        LCDM2 = ans[0];
        LCDM3 = ans[1];
    }
    else if (pos == 5) {
        LCDM18 = ans[0];
        LCDM19 = ans[1];
    }
}

void setup() {
    // setup code: runs once
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    // L0~L26 & L36~L39 pins selected
    LCDPCTL0 = 0b1111111111111111; // All reg 1 pins on
    LCDPCTL1 = 0b1111111111111111; // All reg 2 pins on
    LCDPCTL2 = 0b1111111111111111; // All reg 3 pins on  #more than necessary - couldn't hurt?

    LCDCSSEL0 = 0b1111; // 0 - 3 are COM

    LCDCTL0 |= (1 << 2); // LCDSON - turn segments on
    LCDCTL0 |= (0b011 << 3); // LCDMXx - Set 4 mux mode as per datasheet

    //Also of interest: LCDDIVx and LCDSSEL for clocked blinking

    // LCD Operation - Mode 3, internal 3.02v, charge pump 256Hz
    LCDVCTL |= (0b11110111101 << 5); // 3.02 V, 256 Hz, Charge Pump on, Internal Ref on R13 Enabled, R33 internally connected

    // Clear LCD memory
    LCDMEMCTL |= (1 << 1); // LCDCLRM - clear LCD memory buffer
    LCDCTL0 |= 1; // LCDON - turn on the power!

    // set Comm pins as per page 13 https://www.ti.com/lit/ug/slau595b/slau595b.pdf?ts=1709648945332&ref_url=https%253A%252F%252Fwww.ecosia.org%252F
    LCDM0 = 0b00100001;
    LCDM1 = 0b10000100;
}

int main(void)
{
    setup();
    for (;;) {
        // 24 chars to test
        char testVector[24] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'M', 'o', 'T', 'u', 'W', 'e', 'T', 'h', 'F', 'r', 'S', 'a', 'S', 'u'};

        for (i = 0; i < 24-6; i++) {
            show_digit(testVector[i], 0);
            show_digit(testVector[i+1], 1);
            show_digit(testVector[i+2], 2);
            show_digit(testVector[i+3], 3);
            show_digit(testVector[i+4], 4);
            show_digit(testVector[i+5], 5);
            int j;
            for (j = 0; j<30000; j++);
        }
    }
	return 0;
}
