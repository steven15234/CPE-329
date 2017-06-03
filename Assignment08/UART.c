/*
 * UART.c
 *
 *  Created on: May 8, 2017
 *      Author: Derek
 */

#include "UART.h"


static char* UART_BUFFER = NULL;
static int UART_BUFFER_FLAG;
static int UART_BUFFER_SIZE;



//Fast 10^n lookup
int pow10(int n){

    static int LUT_pow10[10] = {
        1, 10, 100, 1000, 10000,
        100000, 1000000, 10000000, 100000000, 1000000000
    };

    return LUT_pow10[n];
}


//fast string to unsigned number
uint32_t a_to_i(char* str, uint8_t len){
    int i, j, result;
    j = len - 2;
    for(i = result = 0; i < len - 1; i++){
       result +=  (str[i] - '0') * pow10(j--);
    }
    return result;
}



void UART0_init(uint32_t buffer_size, uint32_t baud_rate){
    EUSCI_A0->CTLW0 |= 1;     /* put in reset mode for config */
    EUSCI_A0->MCTLW = 0;      /* disable oversampling */
    EUSCI_A0->CTLW0 = 0x0081; /* 1 stop bit, no parity, SMCLK, 8-bit data */
    EUSCI_A0->BRW = (uint32_t) GLOBAL_FREQ * 1000000/baud_rate;       /* SMCLK/Baud_rate*/
    P1->SEL0 |= 0x0C;         /* P1.3, P1.2 for UART */
    P1->SEL1 &= ~0x0C;
    EUSCI_A0->CTLW0 &= ~1;    /* take UART out of reset mode */
    EUSCI_A0->IE |= 1;        /* enable receive interrupt */

    //set buffer size to five
    UART_BUFFER = malloc(buffer_size);
    //set 4th index to NULL ending
    UART_BUFFER[buffer_size - 1] = 0;
    UART_BUFFER_FLAG = 0;
    UART_BUFFER_SIZE = buffer_size;
}


char* UART_get_buffer(){
    return UART_BUFFER;
}

int UART_get_flag(){
    return UART_BUFFER_FLAG;
}

void UART_reset_flag(){
    UART_BUFFER_FLAG = 0;
}

//UART Read Interrupt only allows ASCII 0-9
void EUSCIA0_IRQHandler(void) {
    static int i = 0;


    UART_BUFFER[i++] = EUSCI_A0->RXBUF;  /* read the receive char*/
    /* interrupt flag is cleared by reading RXBUF */

    //Only allow ASCII 0-9 chars, 0 is set for invalids
    if(UART_BUFFER[i-1] > '9' || UART_BUFFER[i - 1] < '0'){
        UART_BUFFER[i-1] = '0';
    }

    //if buffer is full, raise the flag
    if(UART_BUFFER_SIZE - 1 == i){
       i = 0;
       UART_BUFFER_FLAG = 1;
   }
   else{
       UART_BUFFER_FLAG = 0;
   }
}


//Send string to Console
void UART_send(char* string, int newline){
    int i;
    for(i = 0; i < strlen(string); i++){
        while(!(EUSCI_A0->IFG & 0x02));  /* wait for transmit buffer empty */
           EUSCI_A0->TXBUF = string[i];              /* send a char */
    }
    if(newline){
    while(!(EUSCI_A0->IFG & 0x02));  /* wait for transmit buffer empty */
       EUSCI_A0->TXBUF = '\n';              /* send a newline char */
    while(!(EUSCI_A0->IFG & 0x02));  /* wait for transmit buffer empty */
        EUSCI_A0->TXBUF = '\r';              /* send a return char */
    }
}
void UART_send_char(char val){
    while(!(EUSCI_A0->IFG & 0x02)) { }  /* wait for transmit buffer empty */
            EUSCI_A0->TXBUF = val;              /* send a char */
}




