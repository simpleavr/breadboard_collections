/*
   Joule Thief Christmas Tree

	Breadbroad Layout

                +=====================================================+
                |                                                     |
                |  .  o-----------------o  .  .  .  .  .  .  .  .  .  |
		        |                                                     |
     -||-  10pF |  o  o---||---o  o |.  o  .  .  .  .  .  .  .  .  .  |
		        |   \_______o     +-||--+  (1.5V button cell)         |
     -[]-  100k |  o[]o  o  o  o  o |.  o  .  .  .  .  .  .  .  .  .  |
    /CBE\ BC547 |   /cBE\ \EBC/                                       |
     -uu- 100uH |  o  o  o  .  ouuo  o--------------o  .  .  .  .  .  |
	     	    |                   ----------o                       |
                |  o  .  o  .  o  ./ o  ---+--+--+--+--+--+  .  .  .  |
      | |   47K | | |     \    |   | | |- b6 b7 CK IO a7 a6|          |
      | |       | | |      \   |   | | |  MSP430G2231      |          |
       |        |  |  +     |  |   | | |+ a0 a1 a2 a3 a4 a5|          |
     .     47uF |  o  o  .  o  o  .| |  ---+--+--+--+--+--+  .  .  .  |
      //        |      //          | \                                |
        .       |  .  .  o  .  .  .| .\-o  .  (<2o  (<3)  .  .  .  .  |
		        |                  |                                  |
     __n__  SW1 |  o__n__o  .  .  .| .  .  (<7)  (<0)  (<8)  .  .  .  |
		        |                  |                                  |
     ->|- Diode |  .  o>|o  .  .  .| .  o  .  (1>)  (4>)  .  .  .  .  |
		        |     o------------|---/                              |
     -|<- Zener |  o  o-|<--o  .  .| .  .  (6>)  (5>)  (9>)  .  .  .  |
                |   \--------------/                                  |
                +=====================================================+
                                           (?>), (<?) 10 x LEDs
                                               


	Schematic

                                           _|_ 
                       o------------------o   o            MSP430G2231
                       |                  |   |            ._________.
      o----------------|---uuuuu--o----o  |   |            |         |
      |                |.         |    |  o---|-< PWM ---12|P2.7     | 10xLEDs
      |               | |4.7k     |    |      |            |     P1.0|2--o--o
      |               |_|         |    V      |            |         |   ^  V
      |                |  ||10pF  |    -1N5819|            |     P1.1|3--o--o
      |      o---------|--||------o    |      |            |         |   ^  V
      |      |         |  ||      |    |      |            |     P1.2|4--o--o
      |      |         |          |    |      |            |         |   ^  V
      |      |  o------o-----o    |    o------o-< PWR -o--1|Vcc  P1.3|5--o--o
      |      |  |.     |     |    |    |      |        |   |         |   ^  V
    __|__    | | |100k |     |    |    |      |        o-10|RST  P1.4|7--o--o
     ---     | |_|     |     |    |    |      |            |         |   ^  V
      |1.5V  |  |      |     |    |   ---47uF -1N5228  o-14|Vss  P1.5|7--o--o
      |      |  |    |/      |  |/    ---     ^3.3v    |   |_________|
      |      o--o----|       o--|      |      |        |
      |              |> BC547s  |>     |      |        |
      |               |          |     |      |        |
     _|_             _|_        _|_   _|_    _|_      _|_
     ///             ///        ///   ///    ///      ///



Summary:
A minimalist 1.5V coin cell led tree with all components and power source confined within a mini breadboard.

Description:
This is a holiday project targeting the 2017 December HackaDay coin cell challenge.
The objective is to employ a 1.5V button cell to drive microcontroller based holiday led display.
A minimalist design with all components and power source confined within a mini breadboard.

Detail Description:
A joule thief circuit is used to step up the voltage from the 1.5V button cell to power the microcontrollers and leds.
A push button instead of a switch is used to complete the joule thief circuit.
Initial push button depress starts the joule thief and supply start up voltage to the microcontroller (which operates at 1.8V to 3.6V).
A zener diode limits the maximum voltage to 3.3V. This can be replaced by a 3.6V zener if higher voltage is needed
Once the microcontroller boots up, and the push button releases, the microcontroller takes control of the joule thief.
Microcontroller samples the supply voltage via internal ADC and switch on the joule thief (mimicking a push button depress) when voltage is less than 3.2V and switch it off when voltage is below 3.2V. This provide certain efficiency benefits to the otherwise inefficient joule thief circuit. The voltage threshold can be altered via source code change.
The led christmas tree cycles in a fixed lighting sequence.
There are 4 + 1 different display modes; breathing, solid, throbing, and twinkle, plus an automatic cycling of the four modes.
The display starts w/ a breathing display mode when initially turned on, and cycles between the above-mentioned 4+1 modes upon keypress.
The same I/O pin is used for switching the joule thief and for push button reading. The firmware switches the I/O pin periodically to input and reads the button state.
A long keypress will shut_down the display via switching off the joule thief, leading to collapse of the voltage supplying to the microcontroller.
The entire build incluing power is on a 170 tie-point mini breadbroad.

This project demonstrates an switching joule thief design, timers and interrupts, analog digital conversion usage, and LED multiplexing / charlieplexing.

Application Notes

Single button press turns on led tree
Subsequent single button press (indicate by top led on) rotates these display modes; breathing, solid, throbing, and twinkle, and cycling
Long press (indicate by 2 led on) turns off led tree


Assembling

Follow breadboard layout and place jumper wire on mini breadboard
Place joule thief components with larger components go first
Place MSP430G2231 on breadboard
Cut your LEDs into different lengths, *be sure to maintain one leg longer
The top most LED need extra long legs via soldering cut legs from a sibling
Place LEDs accordingly, you can substitute different color LEDs that works up to 3.2V
Battery Holder was made by reshaping pins in a double row male pin header section

Source code

Source code resides on my github repository, breadboard collections project.  https://github.com/simpleavr/breadboard_collections. Single c source file jt_ledtree.c is needed for build.

Compile under gnu msp430-gcc, build requirement is simple enough, other msp430 compilers should all work.

msp430--gcc -Os -Wall -std=c99 -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,-Map=jt_ledtree.map,--cref -Wl,--relax -Wl,--gc-sections -I/cygdrive/c/Users/chrisc/Desktop/energia-0101E0016/hardware/msp430/cores/msp430 -mmcu=msp430g2231 -o jt_ledtree.elf jt_ledtree.c




Parts / Bill of Materials

This is what's needed for this project

1 x MSP430G2311 (can be substitute by MSP430F2012
1 x 170 tie-point mini breadboard
1 x 4.7k, 1 x 100k resistors
1 x 10pF ceramic capacitor
1 x 47uF electrolytic capacitor
1 x 100uH inductor
2 x BC547 or any small signal NPN transistor
1 x 1N5819 or similar schottly diode
1 x 1N5228 or any 3.3V zener diode (500mW)
1 x tactile switch
Jumper wires
Double row male pin header to be used for battery holder
1 x LR44 1.5V coin cell battery

*/
#include  <msp430.h>
#include  <stdio.h>
#define G2231
/*
*/

#define VOLTS	32

volatile uint8_t secs=0;
volatile uint16_t ticks=0, button=0;
volatile uint8_t dim=0;
volatile uint8_t curIdx=0;

volatile uint8_t advance=0;
volatile uint8_t tick_keep=0, dim_keep=0;

const uint8_t duty[] = {
	0, 0, 250, 244, 200, 146, 114, 100, 88, 66, 48, 40, 34, 23, 12, 7, 5, 4, 3, 2, 1, 1, 1,
	1, 1, 1, 2, 3, 4, 5, 7, 12, 23, 34, 40, 48, 66, 88, 100, 114, 146, 200, 244, 250, 0, 0,
};

void set_mode(uint8_t mode) {
	switch (mode) {
		case 0:	tick_keep = 32; dim_keep = sizeof(duty); break; 		// breath
		case 1:	tick_keep = 16; dim_keep = sizeof(duty); break;		// full
		case 2:	tick_keep = 32; dim_keep = sizeof(duty)/2; break;	// throbe
		case 3:	tick_keep = 1; dim_keep = sizeof(duty); break;					// blink
		default: break;
	}//switch
}

int main(void) {
	WDTCTL = WDTPW|WDTHOLD;	// Stop WDT

	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL  = CALDCO_1MHZ;

	ADC10CTL1 = INCH_11;		// select Vcc/2 against 2.5V reference
	ADC10CTL0 = SREF_1|ADC10SHT_3|REFON|REF2_5V|ADC10ON;

	CCTL0 = CCIE;			// CCR0 interrupt enabled
	CCR0 = 15625/8;			// 
	TACTL = TASSEL_2 + MC_1;	// SMCLK, upmode


	_BIS_SR(GIE);

   /*
   |                   |+ a0 a1 a2 a3 a4 a5|             |
   |                    ---+--+--+--+--+--+              |
   |  .  .  .  .  .  .  .  .  .<2.  .<3.  .  .  .  .  .  |
   |  .  .  .  .  .  .  .  .<7.  .<0.  .<8.  .  .  .  .  |
   |  .  .  .  .  .  .  .  .  .1>.  .4>.  .  .  .  .  .  |
   |  .  .  .  .  .  .  .  .6>.  .5>.  .9>.  .  .  .  .  |
	*
	* 3,7,9
	* 4,6,8
	* 0,1,7
	* 5,2,6
	* */
	const uint8_t ledOut[] = {
		BIT0|BIT3,
		BIT1|BIT4,
		BIT2|BIT5,

		BIT4|BIT0,
		BIT5|BIT1,
		BIT3|BIT2,

		BIT5|BIT0,
		BIT3|BIT1,
		BIT4|BIT2,
	};

	P1DIR = 0x3f;
	P2SEL = 0x00;
	P2DIR = BIT6;
	P2OUT = BIT6|BIT7;
	//P2REN = BIT7;	// pull down for button

	uint8_t key_mode=4, auto_mode=0;

	set_mode(key_mode);

	while (1) {
		uint8_t iMax = duty[dim];
		for (uint8_t iCnt=0;iCnt<iMax;iCnt++) {
			if (iCnt < 2) {
				if (!iCnt) {
					P1OUT = ledOut[curIdx%9];
					if (!button) {
						for (uint8_t d=0;d<16;d++) __asm(" nop");
					}//if
				}//if
				if (!(tick_keep==16)) P1OUT = 0;	// skip off-cycles to achieve constant brightness
			}//if
			if (button) break;
		}//for

		//______________________ check button
		if (advance == 2) {
			P1OUT = BIT3|BIT4;
			P1DIR = BIT2|BIT3|BIT4;
			while (advance == 2) __asm(" nop");
			if (advance == 3) {
				P1OUT = BIT3;
				while (advance == 3) __asm(" nop");
				P1OUT = P1DIR = 0;
				//while (1) __asm(" nop");			// should turn myself off
				//WDTCTL = 0;		// power off, reset will stop joule thief
				TA0CCTL0 &= ~CCIE;
				P2OUT &= ~BIT7;		// stop interrupt, turn off JT switch
			}//if
			P1OUT = 0;
			P1DIR = 0x3f;
			//_________ breath, full, throbe, blink, auto cycle
			key_mode++;
			key_mode %= 5;
			if (key_mode == 4) {		// auto cycle
				auto_mode = 0; advance = 1;
				secs = 0;
			}//if
			else {
				set_mode(key_mode);
				advance = 0; secs = 0;
			}//else
		}//if
		if (advance == 1) {
			if (key_mode == 4) {
				auto_mode++;
				auto_mode %= 4;
				set_mode(auto_mode);
			}//if
			advance = 0;
		}//if
	}//while
}

//______________________________________________________________________
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void) {
	ticks++;
	if (!(ticks%tick_keep)) {
		if (++dim >= dim_keep) {
			dim = 0;	
			curIdx++;
		}//if
	}//if
	//____________ second passed
	if (!(ticks%(1024))) {
		secs++;		
		if (!(secs%4)) advance = 1;
	}//if
	if (!(ticks%16)) {
		//___________ change pin to input and read button
		P2OUT &= ~BIT7;
		P2DIR &= ~BIT7;
		//______________ button read w/ debounce
		if (P2IN&BIT7) {
			button++;
			if (button > 512/16) {
				advance = 3;
			}//if
			else {
				if (button > 64/16)  advance = 2;
			}//else
		}//if
		else {
			if (button) {
				advance = 0;
				button = 0;
			}//if
		}//else
		P2DIR |= BIT7;
		//______________________ maintain joule thief
		ADC10CTL0 |= (ENC|ADC10SC);
		while (ADC10CTL1 & ADC10BUSY);
		if (ADC10MEM > (1024 * VOLTS) / 50) {	
			// don't signal when voltage reached
			P2OUT &= ~BIT7;
		}//if
		else {
			P2OUT |= BIT7;
		}//else
	}//if
}

