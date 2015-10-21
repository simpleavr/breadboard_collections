/*
   K Timer / One Digit Kitchen Timer

   +=====================================================+
   |  .  o-----------------------o  .  .  .  .  .  .  .  |
   |  . (-) .  .  .  .  .  o+BZR=o  .  .  .  .  .  .  .  |
   |                \   .  |  .  .  .  G  F  +  A  B  .  |
   |                 :  .  |  .  .  o-----o  .  .  .  .  |
   | CR 2032         :  .  |  .  +--+--+--+--+--+--+     |
   | Battery Holder  :     |    |- b6 b7 CK IO a7 a6| .  |
   |                 :     |    |+ a0 a1 a2 a3 a4 a5|    |
   |                 :  .  |  .  +--+--+--+--+--+--+  .  |
   |                 :  .  |  .  .  .  E  D  +  C  d  .  |
   |                /.  .  +--------o  .  .  .  .  .  .  |
   |  . (+) .  .  .  .  .  .  .  .  o-[Btn]--o  .  .  .  |
   |  .  o-----------------------o-----------o  .  .  .  |
   +=====================================================+


*/

#include  <msp430.h>
#include  <stdio.h>
#define F2011

//                6  7
//            AB.CFDEG   ABCDEFG.
#define _0_	0b11011110 //11111100
#define _1_	0b01010000 //01100000
#define _2_	0b11000111 //11011010
#define _3_	0b11010101 //11110010
#define _4_	0b01011001 //01100110
#define _5_	0b10011101 //10110110
#define _6_	0b10011111 //10111110
#define _7_	0b11010000 //11100000
#define _8_	0b11011111 //11111110
#define _9_	0b11011101 //11110110

#define _0d	0b11111110 //11111100
#define _1d	0b01110000 //01100000
#define _2d	0b11100111 //11011010
#define _3d	0b11110101 //11110010
#define _4d	0b01111001 //01100110
#define _5d	0b10111101 //10110110
#define _6d	0b10111111 //10111110
#define _7d	0b11110000 //11100000
#define _8d	0b11111111 //11111110
#define _9d	0b11111101 //11110110

#define _X_	0b10000101 //10010010

#define SEG_DOT	BIT5

#define E_BUTTON	BIT0
#define E_30SEC		BIT1
#define E_BTDOWN	BIT2
#define E_ALARM		BIT3
#define E_SLEEP		BIT4
#define E_INPUT		BIT5

//const uint8_t chrs[] = { _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, };
const uint8_t chrs[] = { _0_, _0d, _1_, _1d, _2_, _2d, _3_, _3d, _4_, _4d,
	_5_, _5d, _6_, _6d, _7_, _7d, _8_, _8d, _9_, _9d, _X_ };

volatile uint16_t clicks = 0;
volatile uint8_t seconds = 0;
volatile uint8_t half_minutes = 0;
volatile uint8_t event = E_SLEEP;

int main(void) {
	WDTCTL = WDTPW|WDTHOLD;	// Stop WDT
	//WDTCTL = WDTPW|WDTHOLD|WDTNMI|WDTNMIES;		// stop WDT, enable NMI hi/lo

	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL  = CALDCO_1MHZ;

	CCTL0 = CCIE;				// CCR0 interrupt enabled
	//CCR0 = 10000;				// 10ms @ 1Mhz
	CCR0 = 1000;				// 10ms @ 1Mhz
	TACTL = TASSEL_2 + MC_1;	// SMCLK, upmode

	P2SEL = 0x00;


	uint8_t last_value_set = 6;		// startup is 3 minute for timer

	while (1) {
		//__________ time off, deep sleep
		if (event&E_SLEEP) {
			CCTL0 &= ~CCIE;
			P1REN = ~BIT3;
			P2REN = BIT6|BIT7;
			P1DIR = BIT3;
			P1OUT = ~BIT0;
			P2DIR = 0x00;
			P2OUT = BIT6|BIT7;
			P1IE  |= BIT0;
			P1IES |= BIT0;		// hi-low trip
			P1IFG &= ~BIT0;
			_BIS_SR(LPM4_bits + GIE);
			P1IE  &= ~BIT0;
			P1DIR = BIT3;
			P1REN = BIT0;
			P2REN = 0;

			clicks = 0;
			seconds = 0;
			event = E_INPUT;
			//if (!half_minutes) half_minutes = 6;
			if (!last_value_set) half_minutes = 6;
			else half_minutes = last_value_set;

			CCTL0 |= CCIE;
			//__delay_cycles(1000);
		}//if

		uint8_t segments = chrs[(event&E_ALARM) ? 20 : half_minutes];
		P1OUT = ~(BIT0|BIT3);
		P1OUT &= ~(segments&0xf6);
		P2OUT = BIT6|BIT7;
		if (segments&0x01) P2OUT &= ~BIT7;
		if (segments&0x08) P2OUT &= ~BIT6;

		_BIS_SR(LPM0_bits + GIE);

		if (event&E_BUTTON) {
			if (event&E_ALARM) {
				event &= ~E_ALARM;
				event |= E_SLEEP;
			}//if
			else {
				half_minutes++;
				half_minutes %= 20;
				if (half_minutes) last_value_set = half_minutes;
				clicks = 0;
				seconds = 0;
			}//else
			event |= E_INPUT;
			event &= ~E_BUTTON;
		}//if
		if (event&E_30SEC) {
			if (half_minutes) half_minutes--;
			if (!half_minutes) event |= E_ALARM;
			event &= ~(E_30SEC|E_INPUT);
		}//if

	}//while
}

volatile uint8_t ticks=0;
//______________________________________________________________________
// Timer A0 interrupt service routine
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void) {
	if (event&E_BTDOWN) {
		if (!(P1IN&BIT0)) {
			event &= ~E_BTDOWN;
			event |= E_BUTTON;
			_BIC_SR_IRQ(LPM0_bits);
		}//if
	}//if
	else {
		switch (ticks++) {
			case 0:		// set all pins but button be output and let there be light
				//if (seconds || event&E_INPUT || clicks&0x80) {
				//if ((event&E_ALARM || (!seconds && !(event&E_INPUT))) && clicks&0x80) {
				if ((event&E_ALARM && clicks >= 0x200) || ((!seconds && !(event&E_INPUT)) && clicks&0x80)) {
				}//if
				else {
					P1DIR = ~BIT0;
					P2DIR = BIT6|BIT7;
				}//else
				break;
			/*
			case 1:		// all pins now become input and we black out
				P1DIR = BIT3;
				P2DIR = 0x00;
				break;
			*/
			case 3:		// check for button press
				P1DIR = 0;
				P2DIR = 0x00;
				P1REN = BIT0;
				P1OUT &= ~BIT0;
				if (P1IN&BIT0) event |= E_BTDOWN;
				break;
			case 4:		// buzz buzz
				if (event&E_ALARM && clicks < 0x200 && clicks&0x80) {
					P1REN = 0;
					P1DIR |= BIT0;
					P1OUT |= BIT0;
				}//if
				break;
			case 8:	// reset tick counter so that next time we cycle again
				ticks = 0;
				break;
			default:
				break;
		}//switch

		if (++clicks>=1000) {		// that's one second
			clicks = 0;
			if (event&E_INPUT) {
				event &= ~E_INPUT;		// exit input
				seconds = 0;
				if (!half_minutes) {
					event = E_SLEEP;
					_BIC_SR_IRQ(LPM0_bits);
				}//if
			}//if
			else {
				seconds++;
			}//else
			if (seconds >= 30) {
				seconds = 0;
				event |= E_30SEC;
				_BIC_SR_IRQ(LPM0_bits);
			}//if
		}//if
	}//else
}
//______________________________________________________________________
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
	P1IFG &= ~BIT0;
	_BIC_SR_IRQ(LPM4_bits);
}

