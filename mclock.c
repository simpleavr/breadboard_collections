/*

   MClock, multi-mode clock

   +=====================================================+
   |  .  .  o--(W4)--------o        o--(W1)-------------.| W1 (1.6) b2 to CK
   |  .  . (-) .  .  .-------(W2)------o--(W3)--------o || W2 (1.6) BTN to IO
   |  .  .  .  .  .  |  .  .  +--+--+--+--+--+--+--+  . || W3 (0.5) b3 to IO
   |  .  .  .  .  .  |  .  . C7 C6 R1 C0 R3 C5 C3 R0  . || W4 (0.6) battery to -ve
   |  .  .  .  .  .  |  .  .  |    b2 b3           |  . || W5 (0.6) battery to +ve
   |  .  .  .  .  .  |  .  +--+--+--+--+--+--+--+--+--+ ||
   |                 |    |G b6 b7 CK IO a7 a6 b5 b4 b3|||
   |                 |    |                            |||
   |                 |    |+ a0 a1 a2 a3 a4 a5 b0 b1 b2|||
   |  .  .  .  .  .  |  . .+--+--+--+--+--+--+--+--+--+ ||
   |  .  .  .  .  .  |  .  .  +--+--+--+--+--+--+--+  o-.|
   |  .  .  .  .  .  |  .  .  R4 R6 C1 C2 R7 C4 R5 R2 .  |
   |  .  .  .  .  .  |  .  .  x  x  o  o  .  .  .  .  .  |
   |  .  . (+) .  .  o=BTN=o  .  .  .  .  .  .  .  .  .  | BTN button
   |  .  .  o--(W5)--------o  .  .  .  .  .  .  .  .  .  | x-x, o-o easter egg mini-button
   +=====================================================+


*/

#include <msp430.h>
#include <stdlib.h>
#define G2412

#define NO_C7	1	// not connecting C7 of led matrix as it interferes w/ 32khz xtal

const uint8_t row_col_map[] = { 
	((8|4)<<4|(8|3)), ((8|2)<<4|(0|2)), ((8|1)<<4|(0|3)), ((0|7)<<4|(8|5)), 
	((0|0)<<4|(0|5)), ((8|0)<<4|(0|6)), ((0|1)<<4|(8|7)), ((0|4)<<4|(8|6)),
	// below is used when led matrix is turned around, i.e. 180 degrees
	//((0|0)<<4|(0|4)), ((0|5)<<4|(0|6)), ((8|6)<<4|(0|7)), ((0|3)<<4|(0|1)), 
	//((8|4)<<4|(8|2)), ((8|7)<<4|(0|2)), ((8|5)<<4|(8|0)), ((8|3)<<4|(8|1)),
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
#ifdef NO_C7
	{
	0b00000000,
	0b00111000,
	0b01001100,
	0b10010010,
	0b10100010,
	0b10010010,
	0b01000100,
	0b00111000,
	},
	{
	0b00000000,
	0b00010000,
	0b01010100,
	0b00111000,
	0b11101110,
	0b00111000,
	0b01010100,
	0b00010000,
	},
	{
	0b00000000,
	0b00000000,
	0b01000100,
	0b10000010,
	0b10000010,
	0b10000010,
	0b01000100,
	0b00111000,
	},
#else
	{
	0b00000000,
	0b00011100,
	0b00100110,
	0b01001001,
	0b01010001,
	0b01001001,
	0b00100010,
	0b00011100,
	},
	{
	0b00000000,
	0b00001000,
	0b00101010,
	0b00011100,
	0b01110111,
	0b00011100,
	0b00101010,
	0b00001000,
	},
	{
	0b00000000,
	0b00000000,
	0b00100010,
	0b01000001,
	0b01000001,
	0b01000001,
	0b00100010,
	0b00011100,
	},
#endif
};


#define MCHAR(XX) XX-'A'+12		// letter A start at position 12
const uint8_t menu_desc[] = { 
	MCHAR('S'),MCHAR('E'), 
	MCHAR('D'),MCHAR('M'), 
	MCHAR('A'),MCHAR('O'), 
	};

#define BUTTON_PIN 		BIT3
#define SECONDS_TO_SLEEP	8

const uint8_t dice[] = { 0x10, 0x28, 0x54, 0x45, 0x55, 0x6d, }; // dice matrix

uint16_t time = 4*60+29;			// we starts up at 4:29
volatile uint8_t fb[16];

// routine to load framebuffer
//___________________________________________________
void show_char(uint8_t data, uint8_t which) {

	static uint8_t idx = 0;
	uint8_t line = which & 0x3f;

	if (which & 0x80) {		// menu mode, load letters
		for (which=0;which<2;which++)
			show_char(menu_desc[data*2+which], line);
		return;
	}//if
	else {
		if (which & 0x40) {	// data is numeric
			show_char(data/10, line);
			data %= 10;
		}//if
	}//else

	uint8_t use_dots = 0;
	uint16_t dots = 0x00;
	if (line&0x30) {
		use_dots = 1;
		if (line&0x10) {
			//____________ tix mode
			while (data--) {
				idx += 
					//line + ticks/3;
					*(font + ((line ^ ticks)&0x3f));
				while (1) {
					idx %= 9;
					if (!(dots&(1<<idx))) {
						dots |= (1<<idx);
						break;
					}//if
					idx++;
				}//while
			}//while
			idx += 7;
		}//if
		else {
			//____________ dice mode
			if (data) {
				dots = dice[data-1];
				if (dots&0x01) dots |= 0x0100;
			}//if
		}//else
		line &= ~0x30;
#ifdef NO_C7
		if (line > 1) dots <<= 3;
#else
		if (line) dots <<= 3;
#endif
	}//if
	else {
		data <<= 1;
	}//else

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
#ifdef NO_C7
		fb[line] <<= 4;
		if (use_dots) {
			fb[line] |= (((uint8_t) dots) & 0x07)<<1;
			dots >>= 3;
		}//if
		else {
			// position 1 bit left, now that we don't have column 7
			fb[line] |= (fnt>>2) & 0x0e;
#else
		fb[line] <<= use_dots ? 5 : 4;
		if (use_dots) {
			fb[line] |= ((uint8_t) dots) & 0x07;
			dots >>= 3;
		}//if
		else {
			fb[line] |= (fnt>>3) & 0x07;
#endif
		}//else
		line++;
		fnt <<= 3;
		if (line&0x01) goto here;
	}//for
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
	uint8_t  i=0;
	uint8_t state=ST_SCROLL|ST_REFRESH|ST_AUTO;
	uint8_t sec=30;
	uint8_t menu=0;
	uint8_t mode=0;
	uint8_t setup=0;
	uint8_t dim=1;

	uint8_t  scan=0; 
	uint8_t  stays=0;

#define GAME_LEFT		BIT0
#define GAME_ROTATE		BIT1
#define GAME_RIGHT		BIT2
#define GAME_ADVANCE	BIT3

	uint8_t game_state=0;

	uint8_t sav_fb[8];
	uint8_t piece[5][4] = {		// 5 game piece and their rotations
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

	while (1) { 

		if (stacked && !(state&ST_BREATH) && (state&ST_AUTO) && !menu) { // auto sleep
			if (sleep_at) {
				sleep_at--;
			}//if
			else {
				last_mode = mode < 5 ? mode : 0;
				mode = 5;
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
						mode = 6;			// show score
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
						show_char(setup == 1 ? hr : min, 0x42);
					}//if
					else {
						// surpress leading zero
						if (hr<10) hr += 0x0b*10;
						show_char(hr, 0x42);
#ifdef NO_C7
						for (i=0;i<8;i++) fb[i] <<= 1;
						fb[3] |= 0x01;
						fb[5] |= 0x01;
						show_char(min, 0x4a);
						for (i=8;i<16;i++) fb[i] >>= 1;
#else
						for (i=0;i<8;i++) fb[i] <<= 2;
						fb[3] |= 0x01;
						fb[5] |= 0x01;
						show_char(min, 0x4a);
#endif
						state |= ST_SCROLL;
						//scroll = 0;
					}//else
					break;
				case 1:			// seconds
					show_char(sec, 0x42);
					break;
				case 2:			// tix
#ifdef NO_C7
					show_char(hr, 0x51);
#else
					show_char(hr, 0x50);
#endif
					show_char(min, 0x54);
					break;
				case 3:			// dice
					{
					static uint8_t idx = 0;
					min /= 5;
#ifdef NO_C7
					i = 0x61;
#else
					i = 0x60;
#endif
					do {
						uint8_t j = (hr <= 6) ? hr+1 : (12-hr)+1;
						j = idx%j;
						if (hr > 6) j += (hr - 6);
						hr = (j * 10) + (hr - j);
						show_char(hr, i);
						idx++;
						hr = min;
#ifdef NO_C7
						i += 3;
#else
						i += 4;
#endif
					} while (i<=0x64);
					}
					break;
				case 4:			// bcd
					/*
					fb[0] = hr/10;
					fb[1] = hr%10;
					fb[3] = min/10;
					fb[4] = min%10;
					fb[6] = sec/10;
					fb[7] = sec%10;
					for (i=0;i<8;i++) fb[i] <<= 2;
					{
					uint8_t j;
					fb[8+7] = hr/10;
					fb[8+6] = hr%10;
					fb[8+5] = 0;
					fb[8+4] = min/10;
					fb[8+3] = min%10;
					fb[8+2] = 0;
					fb[8+1] = sec/10;
					fb[8+0] = sec%10;
					for (i=0;i<8;i++) {
						fb[7-i] = 0x00;
						for (j=0;j<8;j++) {
							if (fb[8+j]&(1<<i))
								fb[7-i] |= 1<<j;
						}//for
					}//for
					}
					*/
					{
#ifdef NO_C7
					uint8_t tm[8] = { 0, sec%10, sec/10, min%10, min/10, hr%10, hr/10, 0, };
#else
  					uint8_t tm[8] = { sec%10, sec/10, 0, min%10, min/10, 0, hr%10, hr/10, };
#endif
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
				case 5:			// blank

					while (ticks%4) asm("nop");		// sync to 1/4 sec ticks
					P1DIR = P1OUT = 0;
					P2DIR = P2OUT = 0;
					P2REN = BUTTON_PIN;

					/* tried below to lower power, didn't see difference
					P1DIR = 0xff;
					P2DIR &= ~BUTTON_PIN;
					P1OUT = 0;
					P2OUT = 0;
					P1REN = P2REN = 0xff;
					*/

					P2IES &= ~BUTTON_PIN;	// low-high trigger
					P2IFG &= ~BUTTON_PIN;	// clear
					P2IE = BUTTON_PIN;	// pin interrupt enable

					BCSCTL3 = XCAP_3;
					P2SEL = BIT6|BIT7;		// need xtal
					CCTL0 = 0;		// stop timer a
					WDTCTL = WDT_ADLY_250 + WDTNMI + WDTNMIES;	// WDT at 250ms, NMI hi/lo
					IE1 |= WDTIE;                             // Enable WDT interrupt

					_BIS_SR(LPM3_bits + GIE);	// now i and deep sleep

					// we wake up here
					if (P2IFG & BIT3) {		// from keypress
						while (P2IN&BUTTON_PIN) __asm("nop"); 	// make sure key is not depressed
						if (last_mode && last_mode < 5 && !sleep_at) mode = last_mode - 1;
						else mode = 5;
						state |= ST_PRESSED;
					}//if

					IE1 &= ~WDTIE;		// Diable WDT interrupt
					WDTCTL = WDTPW + WDTHOLD + WDTNMI + WDTNMIES;
					CCTL0 = CCIE;		// CCR0 interrupt enabled, time keep w/ DCO from now on
					P2SEL = 0;			// turn xtal pins back to gio for led multiplexing
					state |= ST_REFRESH;
					break;
				case 6:			// report score
				case 7:
				case 8:
					for (i=0;i<16;i++) fb[i] = 0x00;
					if (mode == 8) {
						state |= ST_PRESSED;
					}//if
					else {
						show_char(score, 0x40 + (mode-6)*2);
						mode++;
					}//else
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
					if (mode > 5) mode = 0;
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
				mode = 8;
				menu++;
				if (menu >= 4) menu = 0;
			}//else
			state &= ~(ST_BUTTON | ST_SCROLL);
			//___________ show menu description
			if (menu && !setup) {
				// following 2 lines for 'letter' menu
				//for (i=0;i<8;i++) fb[i] = 0x00;
				//show_char(menu-1, 0x82);
				// following lines for 'icon' menu
				for (i=0;i<8;i++) fb[i] = menu_icon[menu-1][i];
				if (menu == 3 && (state&ST_AUTO)) {
#ifdef NO_C7
					fb[1] |= 0x10;
					fb[2] |= 0x10;
					fb[3] |= 0x10;
#else
					fb[1] |= 0x08;
					fb[2] |= 0x08;
					fb[3] |= 0x08;
#endif
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

		P2REN = BUTTON_PIN;
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
						P1DIR = BIT1|BIT2;
						P1OUT = BIT1;
					}//else
				}//if
			}//else
			wait++;
		} while (P2IN & BUTTON_PIN);
		P2REN = 0;

		//____ addn buttons test P1.3+p1.2, then P1.1+P1.0
		uint8_t button_test = BIT3;
		while (button_test) {
			P1DIR = P1OUT = button_test;
			button_test >>= 1;
			P1REN = button_test;
			if (P1IN&button_test) {
				while (P1IN&button_test) asm("nop");
				game_state |= button_test;
			}//if
			button_test >>= 1;
		}//while
		P1REN = P1DIR = P1OUT = 0;

		if (state&ST_HOLD && state&ST_BREATH) {
			state &= ~(ST_BUTTON|ST_BREATH);
		}//if
		if ((game_state&GAME_RIGHT) && !(state&ST_BREATH) && mode < 5) {
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
			}//swtich
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
#pragma vector=PORT2_VECTOR
__interrupt void port2_isr(void) {
	P2IE &= ~BUTTON_PIN;	// disable pin interrupt
	_BIC_SR_IRQ(LPM3_bits);	// wake up, got keypressed
}

