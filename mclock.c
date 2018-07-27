/*

   MClock, multi-mode clock


   +=====================================================+
   |  .  . (-)------------(-) o||o (1)---------------(1) | 32khz clock crystal
   |  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  | 
   |  .  . (-) .  .  .  .  .  .  +--+--+--+--+--+--+--+  | CR2032 button cell
   |  .  .  .  .  .  .  .  .  . C7 C6 R1 C0 R3 C5 C3 R0  | 
   |  .  .  .  .  .  .  .+------------(2)             |  | 
   |  .  .  .  .  .  .  .| +--+--+--+--+--+--+--+--+--+  |
   |                     ||G b6 b7 CK IO a7 a6 b5 b4 b3| |
   |                     ||                            | | msp430 mcu
   |                     ||+ a0 a1 a2 a3 a4 a5 b0 b1 b2| |
   |  .  .  .  .  .  .  .|.+--+--+--+--+--+--+--+--+--+  |
   |  .  .  .  .  .  .  .+---(2) +--+--+--+--+--+--+--+  |
   |  .  .  .  .  .  .  .  .  . R4 R6 C1 C2 R7 C4 R5 R2  | 8x8 red led matrix
   |  .  .  .  .  .  .  .  .__.  .__.  .  .  .  .  .__.  |
   |  .  . (+) .  .  .  .  o  o  o  o  .  .  .  .  o  o  | tactile buttons (main, left, right)
   |  .  . (+)------------(+) .  .  .  .  .  .  .  .  .  | 
   +=====================================================+

   . all (1)s, (2)s, (-) and (+) points are electrically connected
   . left and right buttons are optional and only used for tetris game


     C0 1 2 3 4 5 6 7
   R0 o o o . . o x o R0-C6
    1 o o o . . o o x R1-C7 (to be implemented for PCB / always display version)
    2 o o o . . o o o
    3 . . . . . . . .
    4 . . . . . . . .
    5 o o o . . o o o
    6 o o o . . o o o
    7 o o o . . o o o

                            MSP430G2xxx
                         -----------------
          --------------|RESET            |
          | ------------|TEST             |
          | |           |                 |
          | |   COL4 <--|P2.0         P1.0|--> ROW1 (of LED Matrix)
 /|\      | |   ROW5 <--|P2.1         P1.1|--> ROW4
  |  _|_  | --- ROW2 <--|P2.2         P1.2|--> ROW6
  --o   o--COL7+ROW0 <--|P2.3         P1.3|--> COL1
    Button      COL3 <--|P2.4         P1.4|--> COL2
                COL5 <--|P2.5         P1.5|--> ROW7
      32Khz /-- N.C. <--|P2.6(XIN)    P1.6|--> ROW3
    Crystal \-- COL7 <--|P2.7(XOUT)   P1.7|--> COL0
                        |                 |

    Row 0 and Column 6 uses shared pin to avoid clock crystal conflicts, and led dot at R0+C6 will not light up
    Optional buttons for Tetris game

	|        |
	|        |     _|_
	|    P1.1|----o   o--+	(left game button)
	|    P1.2|-----------+
	|        |
	|        |     _|_
	|    P2.1|----o   o--+  (right game button)
	|    P2.2|-----------+

	
	2018.07.25
	TODO:
	. provide option for full crystal clock by retiring use of P2.7 to drive LED
*/

#include <msp430.h>
#include <stdlib.h>
#include <stdint.h>
#define G2412

const uint8_t row_col_map[] = { 
	// newest layout to avoid crystal failure, i.e. P2.6 not used for LED multiplexing
	((8|3)<<4|(0|7)), ((0|0)<<4|(0|3)), ((8|2)<<4|(0|4)), ((0|6)<<4|(8|4)), 
	((0|1)<<4|(8|0)), ((8|1)<<4|(8|5)), ((0|2)<<4|(8|3)), ((0|5)<<4|(8|7)),
};

// font matrix, i don't need the letters, but useful for other projects
const uint8_t font[] = {
0xfd, 0x6d, 0x92, 0x12, 0xf9, 0x7c, 0xf9, 0x79, 0x6d, 0x39, 0xfc, 0x79, 
0xe4, 0x7d, 0x79, 0x09, 0xfd, 0x7d, 0x7d, 0x39, 0x02, 0x02, 0x00, 0x00, 
0x55, 0x7d, 0xb5, 0x75, 0xdc, 0x24, 0xb5, 0x6d, 0xfc, 0x74, 0x3c, 0x74, 
0xdc, 0x2d, 0x6d, 0x7d, 0xfa, 0x52, 0x89, 0x49, 0x6d, 0x75, 0xe4, 0x64, 
0x6f, 0x7d, 0x7d, 0x6d, 0xfd, 0x6d, 0x35, 0x74, 0x7d, 0x2e, 0x75, 0x75, 
0x9c, 0x51, 0xba, 0x12, 0xed, 0x6d, 0xad, 0x2d, 0x6d, 0x7f, 0x6d, 0x55, 
0xad, 0x12, 0xf9, 0x54, 

};

#define ST_HOLD		0x80
#define ST_PRESSED	0x40
#define ST_BUTTON   (ST_HOLD|ST_PRESSED)
#define ST_SCROLL	0x20
#define ST_AUTO 	0x10
#define ST_REFRESH	0x08
#define ST_BREATH   0x04
#define ST_SETUP   	0x03

volatile uint8_t stacked=0;

volatile uint8_t ticks=1;
volatile uint8_t scroll=0;
volatile uint8_t game_ticks=0;

const uint8_t menu_icon[3][8] = {
	{
	0b00011100,
	0b00100110,
	0b01001001,
	0b01010001,
	0b01001001,
	0b00100010,
	0b00011100,
	0b00000000,
	},
	{
	0b00001000,
	0b00101010,
	0b00011100,
	0b01110111,
	0b00011100,
	0b00101010,
	0b00001000,
	0b00000000,
	},
	{
	0b00000000,
	0b00100010,
	0b01000001,
	0b01000001,
	0b01000001,
	0b00100010,
	0b00011100,
	0b00000000,
	},
};


#define MCHAR(XX) XX-'A'+12		// letter A start at position 12
#define BUTTON_PIN 		BIT0

#define SECONDS_TO_SLEEP	12

const uint8_t dice[] = { 0x10, 0x28, 0x54, 0x45, 0x55, 0x6d, }; // dice matrix
const uint16_t sfnt[] = { 
	0b111101111, 
	0b111010110,
	0b011010110, 
	0b111011111, 
	0b001111101, 
	0b110010011, 
	0b111111100, 
	0b001001111, 
	0b111111011, 
	0b001111111, 
};	// 3x3 numeric font

uint16_t time = 4*60+29;			// we starts up at 4:29
volatile uint8_t fb[16];

// routine to load framebuffer
//___________________________________________________
void show_char(uint8_t data, uint8_t which) {
	static uint8_t idx = 0;

	// which = RZMMPPPP
	// R    re-entrant call, do not go again
	// Z    blank for leading zero
	// MM   mode: 0-HHMM, 1-tix, 2-dice, 3-hhmm
	// PPPP position, 0..15

	if (which&0x80) {	
		//________ re-entrant call for 1st digit
		which &= 0x7f;
	}//if
	else {
		//________ orginal call, this is 2nd digit
		show_char(data/10, which|0x80);
		data %= 10;
	}//if

	uint8_t use_dots=1, pre_shift=5;
	uint16_t dots=0x00;


	switch (which>>4) {
		case 0:		// scrolling HHMM
			use_dots = 0;
			pre_shift = 4;
			data <<= 1;
			break;
		case 1:		// tix
			//____________ tix mode
			while (data--) {
				//_____ randomly pick 1 to 9 dot position
				idx += *(font + ((which ^ ticks)&0x3f));
				while (1) {
					idx %= 9;
					//__________ R0-C6 is blind, avoid
					if (!(dots&(1<<idx))) {
						dots |= (1<<idx);
						break;
					}//if
					idx++;
				}//while
			}//while
			idx += 7;
			break;
		case 2:		// dice
			if (data) {
				dots = dice[data-1];
				if (dots&0x01) dots |= 0x0100;
			}//if
			break;
		case 3:		// small font
			dots = sfnt[data];
			pre_shift = 4;
			break;
	}//switch
	which &= 0x0f;
	if (which > 1) dots <<= 3;

	uint8_t i=3;
	uint8_t fnt=0, f5=0;

	while (i--) {
		if (!use_dots) {
			if (i) {
				f5 >>= 2;
				fnt = *(font + data++);
				f5 |= fnt & 0xc0;
			}//if
			else {
				fnt = f5>>1;
			}//else
		}//if
here:
		fb[which] <<= pre_shift;
		if (use_dots) {
			fb[which] |= (((uint8_t) dots) & 0x07);
			dots >>= 3;
		}//if
		else {
			fb[which] |= (fnt>>3) & 0x07;
		}//else
		which++;
		fnt <<= 3;
		if (which&0x01) goto here;
	}//while
}
//______________________________________________________________________
void main(void) {
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL  = CALDCO_1MHZ;
	WDTCTL = WDTPW|WDTHOLD|WDTNMI|WDTNMIES;	// stop WDT, enable NMI hi/lo
	//P1DIR = BIT4|BIT5;
	//P1OUT = 0;
	P2SEL = 0;
	CCTL0 = CCIE;                             // CCR0 interrupt enabled
	CCR0 = 62500;
	TACTL = TASSEL_2 + MC_1;           // SMCLK, upmode

	_BIS_SR(GIE); 
	//_BIS_SR(LPM0_bits + GIE);                 // Enter LPM0 w/ interrupt

	//_________________________________ breach line
	uint8_t i=0;
	uint8_t state=ST_SCROLL|ST_REFRESH|ST_AUTO;
	uint8_t sec=30;
	uint8_t menu=0;
	uint8_t mode=6;
	uint8_t setup=0;
	uint8_t dim=1;

	uint8_t  scan=0; 
	uint8_t  stays=0;

#define GAME_LEFT		BIT1
#define GAME_ROTATE		BIT2
#define GAME_RIGHT		BIT3
#define GAME_ADVANCE	BIT4		// tile drop

	uint8_t game_state=0;
	uint8_t sav_fb[8];

	const uint8_t piece[5][4] = {		// 5 game piece and their rotations
		// x  x   x  x  x
		// xx  x x  x x x
		//     x x      x
		{ 0b10011000, 0b01011000, 0b11001000, 0b11010000, },
		{ 0b10001001, 0b01110000, 0b10010001, 0b00111000, },
		{ 0b01010010, 0b10001100, 0b01001010, 0b11000100, },
		{ 0b01010100, 0b01010001, 0b10101000, 0b10001010, },
		{ 0b10010010, 0b11100000, 0b10010010, 0b11100000, },
	};

	uint8_t last_mode=0, sleep_at=SECONDS_TO_SLEEP;
	uint8_t score=0, cidx=0, pidx=0;
	uint8_t hidx=1, vidx=0;
	uint8_t _port = 0x20;		// this is P1

	while (1) { 

		if (stacked && !(state&ST_BREATH) && (state&ST_AUTO) && !menu) { // auto sleep
			if (sleep_at) {
				sleep_at--;
			}//if
			else {
				last_mode = mode < 6 ? mode : 0;
				mode = 6;
			}//else
		}//if

		while (stacked) {
			sec++;
			stacked--;
			//________ around one minute passed
			if (sec >= 60) {
				sec = 0;
				time++;
				state |= ST_REFRESH;
			}//if
			if (mode) {
				state |= ST_REFRESH;
				scroll = 0;
			}//if
		}//while

		if (state&ST_BREATH) {
			if (state&ST_PRESSED) game_state |= GAME_ROTATE;
			if (!game_ticks && !game_state) {
				game_state = GAME_ADVANCE;
				game_ticks = 12;
			}//if
			state &= ~(ST_REFRESH|ST_SCROLL|ST_PRESSED);
		}//if

		if (game_state) {
			uint8_t prev_hidx = hidx;
			uint8_t prev_vidx = vidx;
			if (game_state&GAME_ROTATE) pidx++;
			if (game_state&GAME_ADVANCE) vidx++;
			pidx &= 0x03;
			uint8_t block = piece[cidx][pidx];
			uint8_t failed = 0;
			if (game_state&GAME_LEFT) hidx--;
			if (game_state&GAME_RIGHT) hidx++;

			for (i=0;i<8;i++) fb[i] = sav_fb[i];
			//_________ collision detect piece against frame buffer
			for (i=0;i<3;i++) {
				if (fb[vidx+i] & ((block&0xe0) >> hidx)) {
					hidx = prev_hidx;
					vidx = prev_vidx;
					failed = 1;
					break;
				}//if
				block <<= 3;
			}//for
			block = piece[cidx][pidx];
			//_________ super-impose piece onto frame buffer for display
			for (i=0;i<3;i++) {
				fb[vidx+i] |= ((block&0xe0) >> hidx);
				block <<= 3;
			}//for
			if (failed) {
				//_________ piece fused onto current frame buffer
				if (game_state&GAME_ADVANCE) {
					uint8_t j=8;
					i = 8;
					while (--i) {
						if (fb[i] != 0xff)
							sav_fb[--j] = fb[i];
						else
							score++;
					}//while
					while (j) sav_fb[--j] = 0x81;
					if (fb[0] != 0x81) {	// game over
						mode = 7;			// show score
						state &= ~ST_BREATH;
					}//if
				cidx += ticks; cidx %= 5;		// new piece
				vidx = 0; hidx = 2;
				}//if
			}//if
			else {
				//_____ check for hits (row fullfillment) and advance quicker
				for (i=0;i<8;i++)
					if (fb[i] == 0xff) {
						game_ticks = 4;
						break;
					}//if
			}//else
			game_state = 0;
		}//if

		if ((state&ST_REFRESH) && !stays && !scan && (!menu || setup)) {
			state &= ~(ST_REFRESH|ST_SCROLL);

			if (time >= (12*60))
				time -= (12*60);
			uint8_t hr  = time/60;
			uint8_t min = time%60;
			if (!hr) hr = 12;

			switch (mode) {
				case 0:		// hour + min
					if (setup) {
						show_char(setup == 1 ? hr : min, 0x02);
					}//if
					else {
						// surpress leading zero
						if (hr<10) hr += 0x0b*10;
						show_char(hr, 0x02);
						for (i=0;i<8;i++) fb[i] <<= 2;
						fb[3] |= 0x01; fb[5] |= 0x01;	// create colon
						show_char(min, 0x0a);
						state |= ST_SCROLL;
						//scroll = 0;
					}//else
					break;
				case 1:			// seconds
					show_char(sec, 0x02);
					break;
				case 2:			// tix
					show_char(hr, 0x11);
					show_char(min, 0x14);
					break;
				case 3:			// dice
					{
						static uint8_t idx = 0;
						min /= 5;
						i = 0x20;
						do {
							uint8_t j = (hr <= 6) ? hr+1 : (12-hr)+1;
							j = idx%j;
							if (hr > 6) j += (hr - 6);
							hr = (j * 10) + (hr - j);
							show_char(hr, i);
							idx++;
							hr = min;
							i += 4;
						} while (i<=0x24);
					}
					break;
				case 4:			// HH:MM 3x3 small font
					if (hr < 10) {
						for (i=1;i<4;i++) fb[i] = 0x00;
						show_char(hr, 0x80|0x31);
						for (i=1;i<4;i++) fb[i] <<= (sec/10);
					}//if
					else {
						show_char(hr, 0x31);
						if (sec >= 30)
							for (i=1;i<4;i++) fb[i] <<= 1;
					}//else
					show_char(min, 0x34);
					if (sec&0x01) {
						for (i=5;i<8;i++) fb[i] <<= 1;
					}//if
					break;
				case 5:			// bcd
					{
						uint8_t tm[8] = { sec%10, sec/10, 0, min%10, min/10, 0, hr%10, hr/10, };
						uint8_t j=0x80, *p=(uint8_t*) fb;
						while (j) {
							*p = 0;
							for (i=0;i<8;i++)
								if ((tm[i]<<2)&j) *p |= (1<<i);
							j >>= 1;
							p++;
						}//while
					}
					break;
				case 6:			// blank

					while (ticks%4) __asm(" nop");		// sync to 1/4 sec ticks
					P1DIR = P1OUT = 0;
					P2DIR = P2OUT = 0;
					P1REN = BUTTON_PIN;

					/* tried below to lower power, didn't see difference
					P1DIR = 0xff;
					P1DIR &= ~BUTTON_PIN;
					P1OUT = 0;
					P2OUT = 0;
					P1REN = P2REN = 0xff;
					*/

					P1IES &= ~BUTTON_PIN;	// low-high trigger
					P1IFG &= ~BUTTON_PIN;	// clear
					P1IE = BUTTON_PIN;	// pin interrupt enable

					BCSCTL3 = XCAP_3;
					P2SEL |= BIT6|BIT7;		// need xtal
					CCTL0 = 0;		// stop timer a
					WDTCTL = WDT_ADLY_250 + WDTNMI + WDTNMIES;	// WDT at 250ms, NMI hi/lo
					IE1 |= WDTIE;                             // Enable WDT interrupt

					_BIS_SR(LPM3_bits + GIE);	// now i and deep sleep

					// we wake up here
					if (P1IFG&BUTTON_PIN) {		// from keypress
						while (P1IN&BUTTON_PIN) __asm(" nop"); 	// make sure key is not depressed
						if (last_mode && last_mode < 6 && !sleep_at) mode = last_mode - 1;
						else mode = 6;
						state |= ST_PRESSED;
					}//if

					IE1 &= ~WDTIE;		// Disable WDT interrupt
					WDTCTL = WDTPW + WDTHOLD + WDTNMI + WDTNMIES;
					CCTL0 = CCIE;		// CCR0 interrupt enabled, time keep w/ DCO from now on
					P2SEL &= ~(BIT6|BIT7);			// turn xtal pins back to gio for led multiplexing
					state |= ST_REFRESH;
					break;
				case 7:			// report score
					for (i=0;i<16;i++) fb[i] = 0x00;
					show_char(score, 0x02);
					setup = 1;
				case 8:
					mode++;
					break;
				case 9:
					setup = 0;
					state |= ST_PRESSED;
					/*
					for (i=0;i<16;i++) fb[i] = 0x00;
					if (mode == 9) {
						state |= ST_PRESSED;
					}//if
					else {
						show_char(score, (mode-5)*2);
						mode++;
					}//else
					*/
				default:
					break;
			}//switch
		}//if

		//_____________________________________ check input
		if (state & ST_BUTTON) {	// needs attention
			for (i=0;i<0x10;i++) fb[i] = 0x00;
			if ((state & ST_PRESSED) || setup) {
				//_______________ editing edits or selecting options
				if (!menu) {
					mode++; 
					if (mode > 6) mode = 0;
					sleep_at = SECONDS_TO_SLEEP;
				}//if
				else {
					if (state & ST_PRESSED) {
						if (menu == 3) {		// auto-off
							state ^= ST_AUTO;
							fb[0] = (state&ST_AUTO) ? 0x7f : 0x00;
						}//if
						else {
							if (menu == 2) {
								dim++;
								if (dim > 3) dim = 0;
							}//if
							else {
								if (setup) {
									if (setup == 1) {
										time += 60;
									}//if
									else {
										(time)++;
										if (!(time%60))
											time -= 60;
									}//else
								}//if
								else {
									mode = 0;
									setup++;
								}//else
							}//else
						}//else
					}//if
					else {
						//_______________ inside menu mode
						if (setup) {
							//___________ editing digits
							if (setup == 2) {	
								//________ done here, write eeprom
								// may want to write to flash in the future
								menu = 0;
								setup = 0;
							}//if
							else {
								//_______ edit next digit
								setup++;
							}//else
						}//if
						else {
						}//else
					}//else
				}//else
			}//if
			else {
				//_________ not option, not edit, advance menu
				mode = 9;
				menu++;
				if (menu >= 4) menu = 0;
			}//else
			state &= ~(ST_BUTTON | ST_SCROLL);
			//___________ show menu description
			if (menu && !setup) {
				// following lines for 'icon' menu
				for (i=0;i<8;i++) fb[i] = menu_icon[menu-1][i];
				if (menu == 3 && (state&ST_AUTO)) {
					fb[0] |= 0x08;
					fb[1] |= 0x08;
					fb[2] |= 0x08;
					fb[3] |= 0x08;
				}//for
			}//if
			else {
				state |= ST_REFRESH;
			}//else
			if (!menu && !mode) scroll = 4;
		}//if

		//_____________________ led multiplexing starts here
		//_____________________ stay longer, get brighter
		if (stays & 0x0f) { stays--; continue; 	}//if

		//____________ refresh led matrix, disconnect all
		P1DIR = P2DIR = 0x00;
		P1OUT = P2OUT = 0x00;

		//_____________________ stay blank longer, get dimmer
		if (stays) { stays--; continue; }

		//___________ check button
		uint16_t wait=0;

		P1REN = BUTTON_PIN;
		do {
			if (wait == 0x0040) {
				state |= ST_PRESSED;
			}//if
			else {
				if (wait++ > 0x6000) {
					state &= ~ST_PRESSED;
					state |= ST_HOLD;
					//_____ flash a dot to show long press accepted
					if (wait & 0x0f) {
						P1DIR = P1OUT = 0;
					}//if
					else {
						P1DIR = BIT2|BIT3;
						P1OUT = BIT2;
					}//else
				}//if
			}//else
			wait++;
		} while (P1IN&BUTTON_PIN);

		//____ test game buttons
		game_state = 0;
		P1REN = P1DIR = P1OUT = 0;
		P2REN = P2DIR = P2OUT = 0;

		/*
		P1REN = BIT1;
		P1DIR = P1OUT = BIT2;
		if (P1IN&BIT1) {
			while (P1IN&BIT1) __asm(" nop");
			game_state = GAME_LEFT;
		}//if
		P1REN = P1DIR = P1OUT = 0;

		P2REN = BIT1;
		P2DIR = P2OUT = BIT2;
		if (P2IN&BIT1) {
			while (P2IN&BIT1) __asm(" nop");
			game_state = GAME_RIGHT;
		}//if
		P2REN = P2DIR = P2OUT = 0;
		*/
		//static uint16_t _port = 0x0020;		// this is P1

		_port ^= 0x08; 	// toggle between P1 and P2
		uint8_t volatile *pp = (uint8_t*) _port;

		*(pp+7) = BIT1;				// PxREN
		*(pp+2) = *(pp+1) = BIT2;	// PxDIR, PxOUT
		if ((*pp)&BIT1) {
			while ((*pp)&BIT1) __asm(" nop");
			game_state = _port&0x08 ? GAME_RIGHT : GAME_LEFT;
		}//if
		*(pp+7) = *(pp+2) = *(pp+1) = 0;

		if (state&ST_HOLD && state&ST_BREATH) {
			state &= ~(ST_BUTTON|ST_BREATH);
		}//if

		//if ((game_state&GAME_RIGHT) && !(state&ST_BREATH) && mode < 5 && !setup) {
		if (game_state && !(state&ST_BREATH) && mode < 5 && !setup) {
			for (i=0;i<8;i++) sav_fb[i] = 0x81;
			fb[8] = 0xff;
			state |= ST_BREATH; // new game
			hidx=1; vidx=0; score=0;
		}//if

		if (state&ST_BUTTON) continue;

		//____________ get current row's port map
		uint8_t row_map = row_col_map[scan] >> 4;
		uint8_t out1=0, out2=0, ddr1, ddr2;

		//____________ read frame buffer
		uint8_t set = 0;
		uint8_t bit = 0;
		uint8_t pos=0x80;
		stays = 0;
		scroll &= 0x1f;

		if ((state&ST_SCROLL)) {		// need to scroll
			uint8_t act = 0;
			switch (scroll>>3) {
				case 0:
					act = fb[scan + 0];
					break;
				case 1:
					set = fb[scan + 0];
					act = fb[scan + 8];
					break;
				case 2:
					set = fb[scan + 8];
				default:
					break;
			}//switch
			set <<= (scroll&0x07);
			set |= act>>(8-(scroll&0x07));
		}//if
		else {
			set = fb[scan];
		}//else

		while (pos) {
			if (set & pos) {
				uint8_t col_map = row_col_map[bit];
				if (col_map&8)
					out2 |= 1<<(col_map&7);
				else
					out1 |= 1<<(col_map&7);
				stays++;
			}//if
			bit++;
			pos >>= 1;
		}//while

		if (stays) {	// load dimming value
			stays <<= 1;
			stays |= dim<<4;
		}//if

		if (setup && !(ticks & 0x02)) { 	// blinks during setup
			out2 = 0;
			out1 = 0;
		}//if

		ddr2 = out2;
		ddr1 = out1;

		out2 = ~ddr2;
		out1 = ~ddr1;
		//____________ turn on current row
		if (row_map & 0x08) {
			row_map &= 0x07;
			out2 |= (1<<row_map);
			ddr2 |= (1<<row_map);
		}//if
		else {
			out1 |= (1<<row_map);
			ddr1 |= (1<<row_map);
		}//else

		P2DIR = ddr2;
		P1DIR = ddr1;
		P2OUT = out2;
		P1OUT = out1;

		scan++;
		//if (scan > 5) state &= ~ST_SCROLL;		// for photo
		scan &= 0x07;
	}//while

}

//______________________________________________________________________
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void) {
	ticks++;
	//____________ second passed
	if (!(ticks%16)) stacked++;		
	scroll++;
	if (game_ticks) game_ticks--;
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
	P1IE &= ~BUTTON_PIN;	// disable pin interrupt
	_BIC_SR_IRQ(LPM3_bits);	// wake up, got keypressed
}

