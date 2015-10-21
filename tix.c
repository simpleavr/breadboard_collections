/*
   Tix Clock Gems, Gem-like Tix Clock
   +=====================================================+
   |  .  o-----(W3)-----o  .  .  .  .  .  .1>.  .8>.  .  |
   |  . (-) .  .  .  .  .  .  .  .  .  .+>.  .4>.  .3>.  |
   |                \.  .  oXTo  .  .  .  .<5.  .<6.  .  | XT 32Khz Clock Crystal
   |                 :+-(W1)--------o  .<7.  .<2.  .<9.  | W1 +ve battery to IO (reset)
   | Battery Holder  :| ---+--+--+--+--+--+--+--+--+  o  | W2 b2 to led extension column
   |                 :||- b6 b7 CK IO a7 a6 b5 b4 b3| |  | W3, W4 battery power to MCU
   |                 :||+ a0 a1 a2 a3 a4 a5 b0 b1 b2|(W2)|
   |                 :| ---+--+--+--+--+--+--+--+--+  |  |
   |                 :| .  .  .1>.  .9>.  .1>.  .2>.  |  |
   |                /.| .  .2>.  .3>.  .4>.  .3>.  o--+  |
   |  . (+) .  .  .  .| .  .  .<6.  .<5.  .<6.  .<5.  .  |
   |  .  o-----(W4)---+-o-[B]-o  .<4.  .<7.  .<8.  .  .  |
   +=====================================================+

*/



#include  <msp430.h>
#include  <stdio.h>
#define G2412

const uint8_t v_hr[] = {
	//P1    P2    D1    D2
	//0, 0, 0, 0,
	BIT6, 0x00, BIT6, BIT5,
	0x00, BIT4, 0x00, BIT5|BIT4,
	0x00, BIT3, 0x00, BIT3|BIT2,
	0x00, BIT5, 0x00, BIT5|BIT4,

	0x00, BIT3, 0x00, BIT4|BIT3,
	0x00, BIT5, BIT6, BIT5,
	BIT6, 0x00, BIT7|BIT6, 0x00,
	0x00, BIT4, 0x00, BIT4|BIT3,

	0x00, BIT2, 0x00, BIT3|BIT2,
};
const uint8_t v_min1[] = {
	//P1    P2    D1    D2
	//0, 0, 0, 0,
	BIT1, 0x00, BIT1|BIT2, 0x00,
	BIT0, 0x00, BIT0|BIT1, 0x00,
	BIT2, 0x00, BIT2|BIT3, 0x00,

	BIT3, 0x00, BIT2|BIT3, 0x00,
	BIT4, 0x00, BIT3|BIT4, 0x00,
	BIT2, 0x00, BIT1|BIT2, 0x00,
};
const uint8_t v_min[] = {
	//P1    P2    D1    D2
	//0, 0, 0, 0,
	BIT5, 0x00, BIT5, BIT0,
	0x00, BIT1, 0x00, BIT1|BIT2,
	0x00, BIT0, 0x00, BIT0|BIT1,
	BIT4, 0x00, BIT4|BIT5, 0x00,

	0x00, BIT0, BIT5, BIT0,
	0x00, BIT2, 0x00, BIT1|BIT2,
	BIT5, 0x00, BIT4|BIT5, 0x00,
	0x00, BIT1, 0x00, BIT0|BIT1,

	BIT3, 0x00, BIT3|BIT4, 0x00,
};

#define AUTO_SLEEP 10

volatile uint8_t stacked=0;
volatile uint8_t ticks=1;
volatile uint16_t time = 4*60+53;			// starts up time

int main(void) {
	WDTCTL = WDTPW|WDTHOLD;	// Stop WDT

	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL  = CALDCO_1MHZ;

	//P2SEL = 0x00;
	CCTL0 = CCIE;				// CCR0 interrupt enabled
	CCR0 = 62500;
	TACTL = TASSEL_2 + MC_1;	// SMCLK, upmode

	_BIS_SR(GIE);

	uint8_t mode = 0; 
	uint8_t calc = 1;
	uint8_t sec=30;

	uint16_t bit_hr = 0;
	uint16_t bit_min = 0;
	uint8_t  bit_hr1 = 0;
	uint8_t  bit_min1 = 0;
	uint8_t  sleep_in = AUTO_SLEEP;

	while (1) {
		while (stacked) {
			sec++;
			stacked--;
			//________ around one minute passed
			if (sec >= 60) {
				sec = 0;
				time++;
				calc = 1;
			}//if
		}//while


		if (!mode) {		// rotate leds every second
			bit_hr <<= 1; if (bit_hr >= 0x0200) bit_hr |= 1; bit_hr &= 0x01ff;
			bit_min1 <<= 1; if (bit_min1 >= 0x0040) bit_min1 |= 1; bit_min1 &= 0x003f;
			bit_min <<= 1; if (bit_min >= 0x0200) bit_min |= 1; bit_min &= 0x01ff;
			if (sleep_in) {
				sleep_in--;
			}//if
			else {
				while (ticks%4) asm("nop");		// sync to 1/4 sec ticks
				P1DIR = P1OUT = 0;
				P2DIR = P2OUT = 0;
				P1REN = BIT1;
				P1IES &= ~BIT1;	// low-high trigger
				P1IFG &= ~BIT1;	// clear
				P1IE = BIT1;	// pin interrupt enable

				BCSCTL3 = XCAP_3;
				//P2SEL = BIT6|BIT7;		// need xtal
				CCTL0 = 0;		// stop timer a
				WDTCTL = WDT_ADLY_250 + WDTNMI + WDTNMIES;	// WDT at 250ms, NMI hi/lo
				IE1 |= WDTIE;                             // Enable WDT interrupt

				_BIS_SR(LPM3_bits + GIE);	// now i and deep sleep

				// we wake up here
				while (P1IN&BIT1) __asm("nop"); 	// make sure key is not depressed

				P1IE &= ~BIT1;
				IE1 &= ~WDTIE;		// Diable WDT interrupt
				//WDTCTL = WDTPW + WDTHOLD + WDTNMI + WDTNMIES;
				WDTCTL = WDTPW|WDTHOLD;	// Stop WDT
				CCTL0 = CCIE;		// CCR0 interrupt enabled, time keep w/ DCO from now on
				sleep_in = AUTO_SLEEP;
				calc = 1;
				//continue;
			}//else

		}//if

		while (!stacked) {
			if (calc) {
				bit_hr = bit_min = 0;
				bit_hr1 = bit_min1 = 0;
				uint8_t hour = time / 60;
				uint8_t min  = time % 60;
				uint8_t min1 = min / 10;
				min %= 10;
				bit_hr1 = (hour >= 10 || !hour) ? 1 : 0;
				if (!hour) hour = 2;	// as in 12:00
				else hour %= 10;
				while (hour) { bit_hr <<= 1; bit_hr |= 1; hour--; }
				while (min1) { bit_min1 <<= 1; bit_min1 |= 1; min1--; }
				while (min)  { bit_min <<= 1; bit_min |= 1; min--; }
				calc = 0;
			}//if

			uint8_t i;
			const uint8_t *h=v_hr, *p=v_min, *t=v_min1;
			for (i=0;i<9;i++) {
				uint8_t stay=9, d1=0, d2=0, p1=0, p2=0;
				if (((ticks&0x02 && mode==1) || !mode) && bit_hr1 && i<3) {
					p1 = BIT7; d1 = BIT7|BIT6;
				}//if
				if (((ticks&0x02 && mode==1) || !mode) && bit_hr & (1<<i)) {
					p1 |= *h++; p2 |= *h++;
					d1 |= *h++; d2 |= *h++;
				}//if
				else {
					h += 4;
					stay -= 3;
				}//else

				if (((ticks&0x02 && mode==2) || !mode) && bit_min1 & (1<<i)) {
					p1 |= *t++; p2 |= *t++;
					d1 |= *t++; d2 |= *t++;
				}//if
				else {
					t += 4;
					stay -= 3;
				}//else

				if (((ticks&0x02 && mode==3) || !mode) && bit_min & (1<<i)) {
					p1 |= *p++; p2 |= *p++;
					d1 |= *p++; d2 |= *p++;
				}//if
				else {
					p += 4;
					stay -= 3;
				}//else
				P1DIR = P2DIR = 0;
				P1OUT = p1; P2OUT = p2;
				P1DIR = d1; P2DIR = d2;
				while (stay--) __asm("nop");
				//P1DIR &= ~d1; P2DIR &= ~d2;
				//P1OUT &= ~p1; P2OUT &= ~p2;
				//__delay_cycles(10);

			}//for
			//_________ check button
			P1OUT = P2OUT = 0x00;
			P1DIR = 0xc0;
			P2DIR = 0x00;
			P1REN = BIT1;
			if (P1IN&BIT1) {
				uint8_t hit = 0;
				while (P1IN&BIT1) {
					if (hit > 200) {
						if (!(ticks&0x01)) P1OUT = 0x40;
						P1OUT = 0x00;
					}//if
					else {
						hit++;
					}//else
					__delay_cycles(1600);
				}//while
				if (hit > 5) {
					if (hit > 200) {
						mode++;
						mode &= 0x03;
						sleep_in = AUTO_SLEEP;
					}//if
					else {
						if (mode) {
							switch (mode) {
								case 1:
									time += 60;
									if (time > 12*60) time %= 60;
									break;
								case 2:
									if ((time%60)>=50) time -= 50;
									else time += 10;
									break;
								case 3:
									if ((time%10)==9) time -= 9;
									else time++;
									break;
								default:
									break;
							}//switch
							P1REN = 0x00;
							calc = 1; break;
						}//if
						else {
							sleep_in += AUTO_SLEEP;
						}//else
					}//else
					stacked = 0;
					ticks = 0;
				}//if
			}//if
			P1REN = 0x00;

		}//while

	}//while
}

//______________________________________________________________________
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void) {
	ticks++;
	//____________ second passed
	if (!(ticks%16)) stacked++;		
}
//______________________________________________________________________
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer (void) {
	ticks += 4;
	//____________ second passed
	if (!(ticks%16)) stacked++;		
	if (stacked >= 60) {
		stacked = 0;
		time++;
		time %= (12*60);
	}//if
	//_BIC_SR_IRQ(LPM3_bits);
	//WDTCTL = WDTPW + WDTHOLD + WDTNMI + WDTNMIES;
}
//______________________________________________________________________
#pragma vector=PORT1_VECTOR
__interrupt void port1_isr(void) {
	P1IE &= ~BIT1;
	_BIC_SR_IRQ(LPM3_bits);	// wake up, got keypressed
}

