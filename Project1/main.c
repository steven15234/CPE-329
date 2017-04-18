/* Project 1
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

char num_to_char(int num){
    if(num > 0 && num <10)
        return num + '0';
    else if(num == 10)
        return '*';
    else if(num == 11)
        return '0';
    else if(num == 12)
        return '#';
    else
        return 'E';
}


void menu(){
    LCD_home(1);
    LCD_write("LOCKED");
    LCD_home(2);
    LCD_write("ENTER KEY");
}
int main(void) {
    unsigned int done = 1;
    char code[5];
    char key;
    code[0] = 'X';
    code[1] = 'X';
    code[2] = 'X';
    code[3] = 'X';
    code[4] = 0;
    set_DC0(FREQ_12_MHz);
    keypad_init();
    LCD_init();
    menu();
    int i=0;
    while(done) {
        delay_ms(200);
        key = num_to_char(keypad_getkey());
        if('*' == key || 4 == i){
            LCD_clear();
            menu();
            i = 0;
        }
        else if ('E' != key){
            LCD_write_char(key);
            code[i++] = key;
        }
        if(0 == strcmp(code, KEYCODE)){
            LCD_clear();
            LCD_write("HELLO WORLD");
            done = 0;
        }
    }
    return 0;
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










