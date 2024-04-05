
#include <Stopwatch.h>

/**
 * A set of functions used to drive the LCD display
 */

void LCD_init() {
    /**
     *  Setup code to initialise the LCD. Runs once.
     */

     LCDPCTL0 = 0b1111111111111111; //
     LCDPCTL1 = 0b0000011111111111; // Pins L27 - L35 not required, and L30 in fact needs
     LCDPCTL2 = 0b1111111111110000; // to be deactivated to free up the switch on P2.6

     LCDCSSEL0 = 0b1111; // 0 - 3 are COM

     LCDCTL0 |= (1 << 2); // LCDSON - turn segments on
     LCDCTL0 |= (0b011 << 3); // LCDMXx - Set 4 mux mode as per datasheet
     LCDBLKCTL = 0b1000;    // Prescalar = 16, blinking off. Alternating blink mode = 0b11

     //Also of interest: LCDDIVx and LCDSSEL for clocked blinking

     // LCD Operation - Mode 3, internal 3.02v, charge pump 256Hz
     LCDVCTL |= (0b11110111101 << 5); // 3.02 V, 256 Hz, Charge Pump on, Internal Ref on R13 Enabled, R33 internally connected

     // Clear LCD memory
     LCDMEMCTL |= (1 << 1); // LCDCLRM - clear LCD memory buffer
     LCDCTL0 |= 1; // LCDON - turn on the power!

     // set Comm pins as per page 13 https://www.ti.com/lit/ug/slau595b/slau595b.pdf?ts=1709648945332&ref_url=https%253A%252F%252Fwww.ecosia.org%252F
     LCDM0 = 0b00100001;
     LCDM1 = 0b10000100;
     // blinking comm pins
     LCDBM0 = 0b00100001;
     LCDBM1 = 0b10000100;
}

void char_to_digit(char character, char shape[2]) {

    if (character == '9' || character == '\9') {  // For convenience: int 9 == char '\9' so we can send ints as input
        shape[0] = 0b11110111;
        shape[1] = 0b00000000;
    }
    else if (character == '8' || character == '\8') {
        shape[0] = 0b11111111;
        shape[1] = 0b00000000;
    }
    else if (character == '7' || character == '\7') {
        shape[0] = 0b11100000;
        shape[1] = 0b00000000;
    }
    else if (character == '6' || character == '\6') {
        shape[0] = 0b10111111;
        shape[1] = 0b00000000;
    }
    else if (character == '5' || character == '\5') {
        shape[0] = 0b10110111;
        shape[1] = 0b00000000;
    }
    else if (character == '4' || character == '\4') {
        shape[0] = 0b01100111;
        shape[1] = 0b00000000;
    }
    else if (character == '3' || character == '\3') {
        shape[0] = 0b11110011;
        shape[1] = 0b00000000;
    }
    else if (character == '2' || character == '\2') {
        shape[0] = 0b11011011;
        shape[1] = 0b00000000;
    }
    else if (character == '1' || character == '\1') {
        shape[0] = 0b01100000;
        shape[1] = 0b00100000;
    }
    else if (character == '0' || character == '\0') {
        shape[0] = 0b11111100;
        shape[1] = 0b00101000;
    }
    // and letters for days of week
    else if (character == 'M') {
        shape[0] = 0b01101100;
        shape[1] = 0b10100000;
    }
    else if (character == 'T') {
        shape[0] = 0b10000000;
        shape[1] = 0b01010000;
    }
    else if (character == 'W') {
        shape[0] = 0b01101100;
        shape[1] = 0b00001010;
    }
    else if (character == 'F') {
        shape[0] = 0b10001111;
        shape[1] = 0b00000000;
    }
    else if (character == 'S') {
        shape[0] = 0b10110111;
        shape[1] = 0b00000000;
    }
    else if (character == 'o') {
        shape[0] = 0b00111011;
        shape[1] = 0b00000000;
    }
    else if (character == 'u') {
        shape[0] = 0b00111000;
        shape[1] = 0b00000000;
    }
    else if (character == 'e') {
        shape[0] = 0b10011111;
        shape[1] = 0b00000000;
    }
    else if (character == 'h') {
        shape[0] = 0b00101111;
        shape[1] = 0b00000000;
    }
    else if (character == 'r') {
        shape[0] = 0b00001010;
        shape[1] = 0b00000000;
    }
    else if (character == 'a') {
        shape[0] = 0b00011010;
        shape[1] = 0b00010000;
    }
}

void colons(char shape[2], char pos) {
    if (pos == 1) {
        if (monthMode == 0) {
            shape[1] |= 0b00000100; // add a colon
        }
        else {
            shape[1] |= 0b00000001; // add a decimal point
        }
    }
    if (pos == 3) {
        if (monthMode == 1) {
            shape[1] |= 0b00000001; // add a decimal point
        }
        else if (STATE == CHRONO || lapMode == 1) {
            shape[1] |= 0b00000100; // add a colon
        }
    }
}

void LCD_extras() {
    if (alarmActive == 1) {
        LCDM12 |= (1 << 1);
    }
    else {
        LCDM12 &= ~(1 << 1);
    }
    if (chimeActive == 1) {
        LCDM19 |= (0b101);
    }
    else {
        LCDM19 &= ~(0b101);
    }
}

void set_digit(char shape[2], char pos) {
    if (pos == 0) {
        LCDM4 = shape[0];
        LCDM5 = shape[1];
    }
    else if (pos == 1) {
        LCDM6 = shape[0];
        LCDM7 = shape[1];
    }
    else if (pos == 2) {
        LCDM8 = shape[0];
        LCDM9 = shape[1];
    }
    else if (pos == 3) {
        LCDM10 = shape[0];
        LCDM11 = shape[1];
    }
    else if (pos == 4) {
        LCDM2 = shape[0];
        LCDM3 = shape[1];
    }
    else if (pos == 5) {
        LCDM18 = shape[0];
        LCDM19 = shape[1];
    }
}

void show_digit(char character, char pos) {
    /**
     *  Function to display a given character in the LCD screen.
     *  Input arguments: The character to display and a position
     *  indicating on which of the 6 14-seg displays to show it.
    **/
    char shape[2] = {0, 0};
    char_to_digit(character, shape);
    colons(shape, pos);
    set_digit(shape, pos);
}

void blink_digit(char selectedField) {
    /**
     * Function to blink a pair of digits on the LCD screen.
     * The argument is a "field". When the time is set, the selected
     * thing to set cycles as follows:
     *      - Minutes in the middle two digits
     *      - Hours in the first two digits
     *      - Day in the last two digits
     *      - Month in the middle two digits
     *      - Date in the first two digits.
     *
     * When alarm is being set, it alternates between minutes and hours
     * as above.
     */

    selectedField %= 3; // The pattern is middle, left, right, middle, left, right. Saves repeating code.

    // Set all blinking memory registers to the same as the normal register
    LCDBM4 = LCDM4;
    LCDBM5 = LCDM5;
    LCDBM6 = LCDM6;
    LCDBM7 = LCDM7;
    LCDBM8 = LCDM8;
    LCDBM9 = LCDM9;
    LCDBM10 = LCDM10;
    LCDBM11 = LCDM11;
    LCDBM2 = LCDM2;
    LCDBM3 = LCDM3;
    LCDBM18 = LCDM18;
    LCDBM19 = LCDM19;

    // Make one of them blink by setting the blinking register to "--"
    if (selectedField == 0) {
        LCDBM8 = 0b11;
        LCDBM9 = 0;
        LCDBM10 = 0b11;
        LCDBM11 = 0;
    }
    else if (selectedField == 1) {
        LCDBM4 = 0b11;
        LCDBM5 = 0;
        LCDBM6 = 0b11;
        LCDBM7 = 0;
    }
    else if (selectedField == 2) {
        LCDBM2 = 0b11;
        LCDBM3 = 0;
        LCDBM18 = 0b11;
        LCDBM19 = 0;
    }
}
