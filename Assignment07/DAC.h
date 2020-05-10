#include "msp.h"
#include "delay.h"

#define BIT_MASK        0x00FF
#define UPPER4_3        0x3000
#define VREF            3300

void initSPI(void);
void writeCommand(uint16_t input);
void writeHalfWord(uint8_t input);
void outputVoltage(uint16_t amplitude);

void main(void)
{
    setup_DCO(FREQ_6);
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	initSPI();
	while(1)
	{
	    outputVoltage(620); // Output 2.0 V
	}
}

/* This function initializes the pins for interfacing with the MCP4912 DAC
 * P1.6 MOSI
 * P1.5 SCLK
 * P5.0 ~CS
 * */
void initSPI(void)
{
    EUSCI_B0->CTLW0 |=  UCSWRST;    // Set UCSWRST

    //Configure CTLW0 register
    EUSCI_B0->CTLW0 &=  ~UCCKPH;     // Data captured on first edge changed following
    EUSCI_B0->CTLW0 |=  UCCKPL;     // Clock polarity high
    EUSCI_B0->CTLW0 |=  UCMSB;      // MSB first
    EUSCI_B0->CTLW0 &=  ~UC7BIT;    // 8-bit mode
    EUSCI_B0->CTLW0 |=  UCMST;      // Master mode
    EUSCI_B0->CTLW0 |=  EUSCI_B_CTLW0_MODE_0;   // 3 pin SPI mode
    EUSCI_B0->CTLW0 |=  UCSYNC;                 // Synchronous mode
    EUSCI_B0->CTLW0 |=  EUSCI_B_CTLW0_UCSSEL_2; // Choose SMCLK as SCLK source
    EUSCI_B0->CTLW0 |=  0x01;   // Divide clk by 16

    //Configure Ports
    P1->SEL1 &= ~BIT5;  // P1.5 SCLK
    P1->SEL0 |= BIT5;

    P1->SEL1 &= ~BIT6;  // P1.6 MOSI/SDI
    P1->SEL0 |= BIT6;

    P5->SEL1 &= ~BIT0;  // P5.0 ~CS intialized to off
    P5->SEL0 &= ~BIT0;
    P5->DIR |= BIT0;

    EUSCI_B0->CTLW0 &= ~UCSWRST;
    EUSCI_B0->IFG |= EUSCI_B_IFG_TXIFG;  // Clear TXIFG flag
}

void writeCommand(uint16_t input)
{
    P5->OUT &= ~BIT0;
    writeHalfWord((input >> 8) & BIT_MASK);  // Write upper 8 bits
    writeHalfWord(input & BIT_MASK);         // Write lower 8 bits
    P5->OUT |= BIT0;
}

void writeHalfWord(uint8_t input)
{
    EUSCI_B0->TXBUF = input;
    while (EUSCI_B0->STATW & UCBUSY) // Wait until data transmittal is done Use flag bit or IFG
    {
        ;
    }
}

/* This function takes in the numerator of the ratio
 * x/1024. This function also takes in on offset which is scaled by a factor of 1000
 * to avoid floating point calculations. It then sets the corresponding output voltage.
 * Gain setting function is 0-1023/1024 * VREF. The amplitude
 * cannot exceed 1023.
 */
void outputVoltage(uint16_t amplitude)
{
    uint16_t command;   // Command is the 16-bit input we send to writeCommand
    uint8_t i, j;

    // Modify numerator to correct voltage by adding offset
    command |= UPPER4_3;    // Set 3 into the 4 MSBs.
    command |= (amplitude << 2) & 0xFFC; // Puts input into D9-D0

    writeCommand(command);
}
