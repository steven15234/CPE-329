/* www.MicroDigitalEd.com
 * p3_3.c: Initialize and display "hello" on the LCD using 4-bit data mode.
 * Data and control pins share Port 4.
 * This program does not poll the status of the LCD.
 * It uses delay to wait out the time LCD controller is busy.
 * Timing is more relax than the HD44780 datasheet to accommodate the
 * variations among the LCD modules.
 * You may want to adjust the amount of delay for your LCD controller.
 *
 * Tested with Keil 5.20 and MSP432 Device Family Pack V2.2.0
 * on XMS432P401R Rev C.
 */
#include "msp.h"
#include "LCD.h"

int main(void) {
    LCD_init();
    set_DC0(FREQ_24_MHz);
    for(;;) {
        LCD_clear();     /* clear display */
        delay_ms(1000);
        LCD_data('h');      /* write the word "Hello" */
        LCD_data('e');
        LCD_data('l');
        LCD_data('l');
        LCD_data('o');
        LCD_home();
        delay_ms(5000);
    }
}
