//
//#include "msp.h"
//#include "LCD.h"
//
//int main(void) {
//    LCD_init();
//    set_DC0(FREQ_24_MHz);
//    for(;;) {
//        LCD_clear();     /* clear display */
//        delay_ms(1000);
//        LCD_write('h');      /* write the word "Hello" */
//        LCD_write('e');
//        LCD_write('l');
//        LCD_write('l');
//        LCD_write('o');
//        LCD_home();
//        delay_ms(5000);
//    }
//}
//


/* p3_5.c: Matrix keypad scanning
 *
 * This program scans a 4x3 matrix keypad and returns a unique number
 * for each key pressed.
 *
 * Port 5.7-5 are connected to the columns and Port 5.4 and 5.2-0 are connected
 * to the rows of the keypad.
 *
 */

#include "msp.h"
#include "LCD.h"
#include "keypad.h"

void LED_init(void);
void LED_set(int value);

int main(void) {
    unsigned int key;

    keypad_init();
    LED_init();

    while(1) {
        key = keypad_getkey();  /* read the keypad */
        LED_set(key);           /* set LEDs according to the key code */
    }
}


/* Initialize tri-color LEDs on the LaunchPad board.
 * P2.0 - red
 * P2.1 - green
 * P2.2 - blue
 */
void LED_init(void) {
    P2->DIR |= 0x07;         /* make LED pins output */
    P2->OUT &= ~0x07;       /* turn the LEDs off */
}

/* turn on or off the LEDs according to bit 2-0 of the value */
void LED_set(int value) {
    value &= 0x07;          /* only bit 2-0 are affected */
    P2->OUT = (P2->OUT & ~0x07) | value;
}










