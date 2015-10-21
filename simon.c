/*
   MSG, Mini Simon Game
   +=====================================================+
   |  .  o-----(W1)--------o(W1a)o  .  .  .  .  .  .  .  | W1, W1a battery to -ve, buzzer (-)
   |  . (-) .  .  .  .  . BZR .  o=BTN=o  .  o=BTN=o  .  | W2 battery to +ve
   |                \   .  +  .+--(W3)-------+  .  .  .  | W3 +ve to IO (reset)
   |                 :  .  o  .| .  o<<o  .  .  o>>o  .  | W4 a2 to buzzer (+)
   |                 :  .  |  .| +--+--+--+--+--+--+     |
   | Battery Holder  :     |   ||- b6 b7 CK IO a7 a6| .  | D   A
   |                 :    (W4) ||+ a0 a1 a2 a3 a4 a5|    | C   B
   |                 :  .  |  .| +--+--+--+--+--+--+  .  |
   |                 :  .  |  .| .  o>>o  .  .  o>>o  .  |
   |                /.  .  +---|----------o  .  .  .  .  |
   |  . (+) .  .  .  .  .  .  .| o=BTN=o  .  o=BTN=o  .  |
   |  .  o-----(W1)------------+-o  .  .  .  .  .  .  .  |
   +=====================================================+


*/

#include  <msp430.h>
#include  <stdio.h>
#define F2011

#define AUTO_SLEEP 10

#define A_ON	(P1OUT |= BIT6)
#define A_OFF	(P1OUT &= ~BIT6)
#define B_ON	(P1OUT &= ~BIT4)
#define B_OFF	(P1OUT |= BIT4)
#define C_ON	(P1OUT |= BIT1)
#define C_OFF	(P1OUT &= ~BIT1)
#define D_ON	(P2OUT |= BIT6)
#define D_OFF	(P2OUT &= ~BIT6)

#define AaOFF	(P1OUT |= BIT7)
#define AaON	(P1OUT &= ~BIT7)
#define CaOFF	(P1OUT |= BIT0)
#define CaON	(P1OUT &= ~BIT0)

#define ALL_ON	A_ON;B_ON;C_ON;D_ON
#define ALL_OFF	A_OFF;B_OFF;C_OFF;D_OFF

/*
#define c 3824
#define cSH 3608
#define d 3405
#define dSH 3214
#define e 3034
#define f 2863
#define fSH 2703
#define g 2551
#define gSH 2408
#define a 2273
#define aSH 2145
#define b 2025
#define cH 1911
*/

volatile uint8_t wait=0;
volatile uint8_t tenths=0;
volatile uint8_t led_state=0x00;

#define	STEP_WAIT	4
//______________________________________________________________________
void flash(uint8_t leds, uint8_t times, uint8_t wait_time) {
	//_________________ blink a few times to get ready
	uint8_t i=0;
	for (i=0;i<times;i++) {
		led_state = leds;
		wait = wait_time; while (wait);		// wait for 0.5 sec
		led_state = 0x00;
		wait = wait_time; while (wait);
	}//for
}//flash

//______________________________________________________________________
int main(void) {
	WDTCTL = WDTPW|WDTHOLD;	// Stop WDT

	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL  = CALDCO_1MHZ;

	P2SEL = 0x00;
	CCTL0 = CCIE;
	CCR0  = 1000;
	TACTL = TASSEL_2 + MC_2 + TAIE;                  // SMCLK, continous

	//_BIS_SR(GIE);


	while (1) {

		uint8_t seq[32];			// this would make 128 steps

		CCTL0 &= ~CCIE;
		P1DIR = P2DIR = 0x00;
		P1OUT = ~(BIT1|BIT2);
		P2OUT = 0xbf;
		P1REN = P2REN = 0xff;
		P1IE = BIT1;
		P1IES = BIT1;		// hi-low trip
		P1IFG &= ~BIT1;

		seq[0] = 0xf800 + TAR;
		_BIS_SR(LPM4_bits + GIE);

		P1IE = 0;
		CCTL0 |= CCIE;
		_BIS_SR(GIE);

   /*
   +=====================================================+
   |  .  o-----(W1)--------o(W1a)o  .  .  .  .  .  .  .  | W1, W1a battery to -ve, buzzer (-)
   |  . (-) .  .  .  .  . BZR .  o=BTN=o  .  o=BTN=o  .  | W2 battery to +ve
   |                \   .  +  .+--(W3)-------+  .  .  .  | W3 +ve to IO (reset)
   |                 :  .  o  .| .  o<<o  .  .  o>>o  .  | W4 a2 to buzzer (+)
   |                 :  .  |  .| +--+--+--+--+--+--+     |
   | Battery Holder  :     |   ||- b6 b7 CK IO a7 a6| .  | D   A
   |                 :    (W4) ||+ a0 a1 a2 a3 a4 a5|    | C   B
   |                 :  .  |  .| +--+--+--+--+--+--+  .  |
   |                 :  .  |  .| .  o>>o  .  .  o>>o  .  |
   |                /.  .  +---|----------o  .  .  .  .  |
   |  . (+) .  .  .  .  .  .  .| o=BTN=o  .  o=BTN=o  .  |
   |  .  o-----(W1)------------+-o  .  .  .  .  .  .  .  |
   +=====================================================+

   */
		P1REN = BIT3;
		P2REN = 0x00;

		P1DIR = ~BIT3;
		P1OUT = BIT4|BIT5;
		P2DIR = BIT6|BIT7; 
		P2OUT = 0x00;

		flash(0x0c, 2, 1);

		//const uint8_t *seq = (uint8_t*) 0xf800;
		uint8_t steps = 0;
		uint8_t failed = 0;

		//_________________ game play
		while (!failed) {

			if (steps && !(seq[steps]&0x03)) seq[steps>>2] = 0xf800 + TAR;
			++steps;

			//_________________ flash appropriate led up to number of steps in play
			uint8_t i;
			for (i=0;i<steps;i++) {
				wait = 3; while (wait);
				//led_state = 1<<(*(seq+i)&0x03);
				uint8_t v = *(seq+i/4);
				v >>= (i&0x03)<<1;
				led_state = 1<<(v&0x03);
				wait = 4; while (wait);
				led_state = 0x00;
				wait = 1; while (wait);
			}//for
			
#define A_DOWN	(P1IN&BIT6)
#define B_DOWN	(P1IN&BIT3)
#define C_DOWN	(P1IN&BIT1)
#define D_DOWN	(!(P2IN&BIT7))

			failed = 0;
			uint8_t cnt = 0;
			wait = 50;	// set a deadline for response
			//_________________ now player's turn to key in the right buttons
			while (!failed && cnt<steps) {
				//__________ capture keys, and wait for release
				P1DIR &= ~(BIT1|BIT6);
				P2OUT |= BIT7; P2DIR &= ~BIT7; P2REN |= BIT7; 

				led_state = 0;
				while (!failed) {
					uint16_t pressed = 0;
					if (A_DOWN) {
						led_state = 0x11;
						while (A_DOWN) pressed++;
					}//if
					else if (B_DOWN) {
						led_state = 0x11<<1;
						while (B_DOWN) pressed++;
					}//if
					else if (C_DOWN) {
						led_state = 0x11<<2;
						while (C_DOWN) pressed++;
						P1OUT &= ~BIT0;
					}//if
					else if (D_DOWN) {
						led_state = 0x11<<3;
						while (D_DOWN) pressed++;
					}//if
					else {
						led_state = 0;
					}//if
					led_state &= 0x0f;		// mask out button down bits

					if (pressed > 100) {
						uint8_t v = *(seq+cnt/4);
						v >>= (cnt&0x03)<<1;
						v = 1<<(v&0x03);
						if (!(led_state & v))
							failed = v;
						//if (!(led_state & (1<<(*(seq+cnt)&0x03)))) 
						//	failed = 1<<(*(seq+cnt)&0x03);
						led_state = 0x00;
						wait = 50;		// refresh deadline
						cnt++;
						break;
					}//if
					if (!wait) failed = 0x0f;
				}//while
			}//while
			//______________ setup for blinking, no buttons
			P1DIR |= (BIT1|BIT6);
			P2OUT &= ~BIT7; P2DIR |= BIT7; P2REN &= ~BIT7; 
			wait = 4; while (wait);
		}//while
		flash(failed, 3, 1);		// failed, flash green 3 times

	}//while
}

const uint16_t notes[] = { 1276, 3034, 2552, 3822, };
//Gh E G C
//783.99,329.63,392,261.63
volatile uint16_t _ccr1=0;
//______________________________________________________________________
#pragma vector=TIMER0_A0_VECTOR
__interrupt void timer0_a0 (void) {

	//__________________ control led on/off time
	static uint8_t cycles=0, ticks=0;
	if (cycles == 0) {		// check which color is on
		if (led_state&BIT0) {
			if (led_state&BIT4)
				AaON;
			else
				A_ON;
		}//if
		if (led_state&BIT1) B_ON;
		if (led_state&BIT2) {
			if (led_state&BIT6)
				CaON;
			else
				C_ON;
		}//if
		if (led_state&BIT3) D_ON;

		if (led_state && !_ccr1) {	// leds on and sound off, turn it on
			if (led_state&BIT0) _ccr1 = notes[0];
			if (led_state&BIT1) _ccr1 = notes[1];
			if (led_state&BIT2) _ccr1 = notes[2];
			if (led_state&BIT3) _ccr1 = notes[3];
			CCR1  = _ccr1;
			CCTL1 = OUTMOD_4|CCIE;
			P1REN &= ~BIT2;
			P1SEL |= BIT2;
		}//if
		else {
			if (!led_state && _ccr1) {	// sound on and no leds, turn it off
				_ccr1 = 0;
				CCR1  = 0;
				CCTL1 = 0x00;
				P1REN |= BIT2;
				P1SEL &= ~BIT2;
			}//if
		}//else
	}//if
	else {
		if (cycles == 1) {	// time to stop leds as we don't have resistors on them
			if (led_state&BIT4)
				AaOFF;
			else
				A_OFF;
			B_OFF;
			if (led_state&BIT6)
				CaOFF;
			else
				C_OFF;
			D_OFF;
		}//if
	}//else
	cycles++;
	if (cycles >= 16) cycles = 0;

	ticks++;
	if (ticks >= 100) {		// 1/10 of a second passed
		ticks = 0;
		tenths++;
		if (wait) wait--;
	}//if

	CCR0 += 1000;		// at 1Mhz, interrupt at 1ms
}
//______________________________________________________________________
#pragma vector=TIMER0_A1_VECTOR
__interrupt void timer0_a1(void) {
	switch(TAIV) {
		case  2: 
			CCR1 += _ccr1;
			break;
	}//switch
}
//______________________________________________________________________
#pragma vector=PORT1_VECTOR
__interrupt void port1_isr(void) {
	P1IFG &= ~BIT1;
	_BIC_SR_IRQ(LPM4_bits);	// wake up, got keypressed
}

