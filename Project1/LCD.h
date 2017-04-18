/*
 * LCD.h
 *
 *  Created on: Apr 13, 2017
 *      Author: Derek
 */

#ifndef LCD_H_
#define LCD_H_

#include "delay.h"
#include <string.h>
/*Assumed that LCD is correctly connected to P4 on the board
* with D4-D7 connected to P4.4-P4.7
*/
#define LCD_RS 1     /* P4.0 mask */
#define LCD_RW 2     /* P4.1 mask */
#define LCD_EN 4     /* P4.2 mask */



/* With 4-bit mode, each command or data is sent twice with upper
 * nibble first then lower nibble.
 */
void LCD_nibble_write(unsigned char data, unsigned char control) {
    data &= 0xF0;           /* clear lower nibble for control */
    control &= 0x0F;        /* clear upper nibble for data */
    P4->OUT = data | control;       /* RS = 0, R/W = 0 */
    P4->OUT = data | control | LCD_EN;  /* pulse E */
    delay_us(40);
    P4->OUT = data;                 /* clear E */
    P4->OUT = 0;
}

/* clears screen, move cursor to home */
void LCD_clear(){
    uint clear = 0x01;
    LCD_nibble_write(0, 0);               /* upper nibble */
    LCD_nibble_write(clear << 4, 0);      /* lower nibble */
}
/* set cursor at beginning of first line */
void LCD_home(uint line){
    uint home = 0x80;
    if(line == 2)
        home = 0xC0;
    LCD_nibble_write(home, 0);   /* upper nibble */
    LCD_nibble_write(home << 4, 0);      /* lower nibble */
}

void LCD_write_char(unsigned char data) {
    LCD_nibble_write(data & 0xF0, LCD_RS);    /* upper nibble first */
    LCD_nibble_write(data << 4, LCD_RS);      /* then lower nibble  */

    delay_ms(1);
}

void LCD_write(const char * string){
    int len = strlen(string);
    int i;
    delay_ms(10);
    for(i = 0; i < len; i++)
        LCD_write_char(string[i]);
}
void LCD_command(unsigned char command) {
    LCD_nibble_write(command & 0xF0, 0);    /* upper nibble first */
    LCD_nibble_write(command << 4, 0);      /* then lower nibble */

    if (command < 4)
        delay_ms(2);         /* commands Clear and Go_Home need up to 1.64ms */
    else
        delay_us(40);         /* all others 40 us */
}


void LCD_init(void) {
    P4->DIR = 0xFF;     /* make P4 pins output for data and controls */
    delay_ms(30);                /* initialization sequence */
    LCD_nibble_write(0x30, 0);
    delay_ms(10);
    LCD_nibble_write(0x30, 0);
    delay_ms(1);
    LCD_nibble_write(0x30, 0);
    delay_ms(1);
    LCD_nibble_write(0x20, 0);  /* use 4-bit data mode */
    delay_ms(1);

    LCD_command(0x28);      /* set 4-bit data, 2-line, 5x7 font */
    LCD_command(0x06);      /* move cursor right after each char */
    LCD_command(0x01);      /* clear screen, move cursor to home */
    LCD_command(0x0F);      /* turn on display, cursor blinking */
}








#endif /* LCD_H_ */
