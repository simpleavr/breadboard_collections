/*

         G     V+ ADC   L  Gnd (1 stage layout) WE ARE USING THIS LAYOUT
   +=====================================================+          c0............c7
   |  MIC.  .  o  .  .  .  .  +-----+  +--+  .  .  .  .  |        r0 o o o o o o o o
   |  o||o  +-----[100k]---------------+  .  .  .  .  .  |        r1 X o o o o o o o
   |  .  +--------------+--+  . C7 C6 R1 C0 R3 C5 C3 R0  |         . o o o o o o o o
   |  .  .  .  .  .  .  |  .  .  | b6 a7              |  | c0 and r1 shares same pin and won't show
   |  +  .  +--+--+--+  |  +--+--+--+--+--+--+--+--+--+  | *possible application to have c6 + c0 + r1
   |  |    |V+        | | |G b6 b7  T  R a7 a6 b5 b4 b3| |  this will free up b6 for 32khz xtal clock
   |  |    |   TLC272 | | |                            | |
   |  |    out -  +  G| | |+ a0 a1 a2 a3 a4 a5 b0 b1 b2| |
   |  +  .  +--+--+--+  |  +--+--+--+--+--+--+--+--+--+  |
   |  o||o  o[]o  .  +--+  .  . R4 R6 C1 C2 R7 C4 R5 R2  |
   |  .  .  .  .  o-[10k]--o  .  .  .  .  .  .  .  .  .  |
   |  .  o-[1k]o  o[]o  .  .  .  .  .  .  .  .  .  .__.  |
   |  o----[10k]-----------o  .  .  .  .  .  .  .  o  o  |
   +=====================================================+
     .1uF   100k  10k        ADC                   Button
            +-----------------+

   TLC272 Dual Op-Amp, GBW @1.7Mhz, @x100 gain, bandwidth up to 17Khz

   * we are using one stage of the TLC272 only


                                               ._____________.
                                               | MSP430G2452 |  Vcc
                                               |             |   |
                      +-----------------------2|ADC0         |1--+
                      |                        |             |   |.
                      |                        |             |  | | pull-up (47k)
   Vcc                |   ---------------      |             |  |_|
    |                 +-1|----.       Vcc|8    |             |   |
    |.                |. |    ^      .---|7    |             |16-+
   | |10k        100k| | |   / \     ^   |     |             |
   |_|               |_| |  /__+\   / \  |     |            /|--- (see breadboard layout)
    |     .1u         |  |   | |   /__+\ |     |           / |------_______
    +------||--[ 1k]--+-2|---+ |    | |  |     |    15 GPIO  |     |       |
    |                 .-3|-----+    +-|--|6    |    P1.1-P1.7|     | 8x8   |
    |                 | 4|Gnd         +--|5    |    P2.0-P2.7|     | LED   |
    |+                |   ---------------      |             |     | matrix|
  ((O)) mic           |.                       |           \ |     |_______|
    |                | |10k                +-20|Gnd         \|--------
    |                |_|                   |   |_____________|
   _|_               _|_                  _|_
   ///               ///                  ///


   *alternate 2 stage pre-amp, not used

                                       +| | 10uF
                      +-----------------| |----------+
                      |                 | |          |
                      |                              |
   Vcc                |   ---------------            | *alternate 2nd stage
    |                 +-1|----.       Vcc|8          |
    |.                |. |    ^      .---|7--+-------|-----o (ADC)
   | |10k        100k| | |   / \     ^   |   |.      |.
   |_|               |_| |  /__+\   / \  |  | |100k | |1k
    |     .1u         |  |   | |   /__+\ |  |_|     |_|
    +------||--[ 1k]--+-2|---+ |    | |  |   |       |  Vcc
    |+                .-3|-----+    +-|--|6--+-------+   |.
  ((O)) mic           | 4|Gnd         +--|5----+        | |10k
    |                 |   ---------------      |        |_|
   _|_                |                        |         |
   ///                +------------------------+---------+
                                                         |.
                                                        | |10k
                                                        |_|
                                                        _|_ 
                                                        /// 


 Chris Chung Nov 2015
 . adopt integar square-root and hamm windowing from sq7bti.
 . relocate layout to drive 63 dots from 15 GPIO pins.
 . allow setup of hamming window 0, 1, 2, 3. 0 is not hamming window.
 . allow setup of dimmer values 0, 1, 2, 3. 0 being brightest.
 . allow setup of sampling delay from 0 to 7. 7 being longest delay.
 . implements automatic gain control.

 Chris Chung May 2014
 . init release.

 code provided as is, no warranty

 you cannot use code for commercial purpose w/o my permission

*/

#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>

#define G2553

//_____________ from fix_fft.c
#define N_WAVE      256    /* full length of Sinewave[] */
#define LOG2_N_WAVE 8      /* log2(N_WAVE) */


#include <stdint.h>
#include <stdlib.h>

const int8_t Sinewave[N_WAVE-N_WAVE/4] = {
0, 3, 6, 9, 12, 15, 18, 21, 
24, 28, 31, 34, 37, 40, 43, 46, 
48, 51, 54, 57, 60, 63, 65, 68, 
71, 73, 76, 78, 81, 83, 85, 88, 
90, 92, 94, 96, 98, 100, 102, 104, 
106, 108, 109, 111, 112, 114, 115, 117, 
118, 119, 120, 121, 122, 123, 124, 124, 
125, 126, 126, 127, 127, 127, 127, 127, 

127, 127, 127, 127, 127, 127, 126, 126, 
125, 124, 124, 123, 122, 121, 120, 119, 
118, 117, 115, 114, 112, 111, 109, 108, 
106, 104, 102, 100, 98, 96, 94, 92, 
90, 88, 85, 83, 81, 78, 76, 73, 
71, 68, 65, 63, 60, 57, 54, 51, 
48, 46, 43, 40, 37, 34, 31, 28, 
24, 21, 18, 15, 12, 9, 6, 3, 

0, -3, -6, -9, -12, -15, -18, -21, 
-24, -28, -31, -34, -37, -40, -43, -46, 
-48, -51, -54, -57, -60, -63, -65, -68, 
-71, -73, -76, -78, -81, -83, -85, -88, 
-90, -92, -94, -96, -98, -100, -102, -104, 
-106, -108, -109, -111, -112, -114, -115, -117, 
-118, -119, -120, -121, -122, -123, -124, -124, 
-125, -126, -126, -127, -127, -127, -127, -127, 

/*-127, -127, -127, -127, -127, -127, -126, -126, 
-125, -124, -124, -123, -122, -121, -120, -119, 
-118, -117, -115, -114, -112, -111, -109, -108, 
-106, -104, -102, -100, -98, -96, -94, -92, 
-90, -88, -85, -83, -81, -78, -76, -73, 
-71, -68, -65, -63, -60, -57, -54, -51, 
-48, -46, -43, -40, -37, -34, -31, -28, 
-24, -21, -18, -15, -12, -9, -6, -3, */
};

inline int8_t FIX_MPY(int8_t a, int8_t b) {
  
  //Serial.print16_tln(a);
 //Serial.print16_tln(b);
  
  
    /* shift right one less bit (i.e. 15-1) */
    int16_t c = ((int16_t)a * (int16_t)b) >> 6;
    /* last bit shifted out = rounding-bit */
    b = c & 0x01;
    /* last shift + rounding bit */
    a = (c >> 1) + b;

        /*
        Serial.print16_tln(Sinewave[3]);
        Serial.print16_tln(c);
        Serial.print16_tln(a);
        while(1);*/

    return a;
}

int16_t fix_fft(int8_t fr[], int8_t fi[], int16_t m, int16_t inverse) {
    int16_t mr, nn, i, j, l, k, istep, n, scale, shift;
    int8_t qr, qi, tr, ti, wr, wi;

    n = 1 << m;

    /* max FFT size = N_WAVE */
    if (n > N_WAVE)
        return -1;

    mr = 0;
    nn = n - 1;
    scale = 0;

    /* decimation in time - re-order data */
    for (m=1; m<=nn; ++m) {
        l = n;
        do {
            l >>= 1;
        } while (mr+l > nn);
        mr = (mr & (l-1)) + l;

        if (mr <= m)
            continue;
        tr = fr[m];
        fr[m] = fr[mr];
        fr[mr] = tr;
        ti = fi[m];
        fi[m] = fi[mr];
        fi[mr] = ti;
    }

    l = 1;
    k = LOG2_N_WAVE-1;
    while (l < n) {
        if (inverse) {
            /* variable scaling, depending upon data */
            shift = 0;
            for (i=0; i<n; ++i) {
                j = fr[i];
                if (j < 0)
                    j = -j;
                m = fi[i];
                if (m < 0)
                    m = -m;
                if (j > 16383 || m > 16383) {
                    shift = 1;
                    break;
                }
            }
            if (shift)
                ++scale;
        } else {
            /*
              fixed scaling, for proper normalization --
              there will be log2(n) passes, so this results
              in an overall factor of 1/n, distributed to
              maximize arithmetic accuracy.
            */
            shift = 1;
        }
        /*
          it may not be obvious, but the shift will be
          performed on each data point16_t exactly once,
          during this pass.
        */
        istep = l << 1;
        for (m=0; m<l; ++m) {
            j = m << k;
            /* 0 <= j < N_WAVE/2 */
            //wr =  pgm_read_word_near(Sinewave + j+N_WAVE/4);
            wr =  Sinewave[j+N_WAVE/4];

/*Serial.print16_tln("asdfasdf");
Serial.print16_tln(wr);
Serial.print16_tln(j+N_WAVE/4);
Serial.print16_tln(Sinewave[256]);
Serial.print16_tln("");*/


            //wi = -pgm_read_word_near(Sinewave + j);
            wi = -Sinewave[j];
            if (inverse)
                wi = -wi;
            if (shift) {
                wr >>= 1;
                wi >>= 1;
            }
            for (i=m; i<n; i+=istep) {
                j = i + l;
                tr = FIX_MPY(wr,fr[j]) - FIX_MPY(wi,fi[j]);
                ti = FIX_MPY(wr,fi[j]) + FIX_MPY(wi,fr[j]);
                qr = fr[i];
                qi = fi[i];
                if (shift) {
                    qr >>= 1;
                    qi >>= 1;
                }
                fr[j] = qr - tr;
                fi[j] = qi - ti;
                fr[i] = qr + tr;
                fi[i] = qi + ti;
            }
        }
        --k;
        l = istep;
    }
    return scale;
}


uint8_t sqrt16(uint16_t a) {
	uint16_t rem=0, root=0;
	uint8_t i;
	for (i=0;i<8;++i) {
		root <<= 1;
		rem = ((rem << 2) + (a >> 14));
		a <<= 2;
		++root;

		if(root <= rem) {
			rem -= root;
			++root;
		}//if
		else {
			--root;
		}//else
	}//for
	return (uint8_t) (root >> 1);
}

//______________________________________________________________________
volatile uint16_t scan_at = 800;		// @8Mhz..1Mhz/100..10Khz (0.1ms)
volatile uint8_t ticks=0;				// @8Mhz..8,000,000/65536 = 122Hz (8ms) per tick

int16_t fix_fft(int8_t fr[], int8_t fi[], int16_t m, int16_t inverse);

const uint8_t row_col_map[] = { 
	//((8|4)<<4|(8|3)), ((8|2)<<4|(0|2)), ((8|1)<<4|(0|3)), ((0|7)<<4|(8|5)), 
	//((0|0)<<4|(0|5)), ((8|0)<<4|(0|6)), ((0|1)<<4|(8|7)), ((0|4)<<4|(8|6)),
	((8|3)<<4|(0|7)), ((0|7)<<4|(0|3)), ((8|2)<<4|(0|4)), ((0|6)<<4|(8|4)), 
	((0|1)<<4|(8|0)), ((8|1)<<4|(8|5)), ((0|2)<<4|(8|6)), ((0|5)<<4|(8|7)),
};

volatile uint8_t fb[8];

volatile uint8_t scan=0; 
volatile uint8_t stays=0;


#define ST_BUSY		BIT0
#define ST_PRESSED	BIT1
#define ST_HOLD		BIT2
#define ST_PAINT	BIT3

volatile uint8_t state = 0;
volatile uint8_t bar = 1;
volatile uint8_t menu = 0;
volatile uint8_t pixel_test = 0;
volatile uint8_t menu_val[3] = { 2, 2, 2, };		// hamming, drop, sampling rate

void scan_led() {

	//_____________________ led multiplexing starts here
	//_____________________ stay longer, get brighter
	if (stays & 0x0f) { stays--; return; 	}//if

	//____________ refresh led matrix, disconnect all
	P1DIR = P2DIR = 0x00;
	P1OUT = P2OUT = 0x00;

	if (state&ST_BUSY) return;
	//_____________________ stay blank longer, get dimmer
	if (stays) { stays--; return; }

	//_____________________ check button
	static uint16_t wait_butt=0;
	P2DIR = P2OUT = BIT2;
	P2REN = BIT1;
	if (P2IN&BIT1) {	// button hit
		wait_butt++;
		if (wait_butt>4000) {
			if (wait_butt & 0x0f) {
				P2DIR = P2OUT = 0;	// off cycle, don't burn our led
			}//if
			else {
				P2DIR = BIT3|BIT7;
				P2OUT = BIT3;
			}//else
		}//if
		return;			// button down, come back later and check for release
	}//if
	else {				// button released
		if (wait_butt>100) {
			if (wait_butt>4000) {
				state |= ST_HOLD;
			}//if
			else {
				state |= ST_PRESSED;
			}//else
		}//if
	}//else
	wait_butt = 0;
	P2REN = P2DIR = P2OUT = 0;

	if (state & ST_PRESSED) {
		state &= ~ST_PRESSED;
		if (menu) {
			menu_val[menu-1]++;
			menu_val[0] &= 0x03;	// hamming has 4 settings
			menu_val[1] &= 0x03;	// dimmer has 4 settings
			menu_val[2] &= 0x07;	// delay had 8 settings
			state |= ST_PAINT;
		}//if
		else {
			bar++;
			bar &= 0x03;
		}//else
	}//if
	if (state & ST_HOLD) {
		state &= ~ST_HOLD;
		menu++;
		menu &= 0x03;
		state |= ST_PAINT;
		//pixel_test = 63;
	}//if

	//if (bar&BIT1) return;		// DEB
	//____________ get current row's port map
	//uint8_t row_map = row_col_map[scan] >> 4;
	uint8_t row_map = row_col_map[scan] &0x0f;
	uint8_t out1=0, out2=0, ddr1, ddr2;

	//____________ read frame buffer
	uint8_t set = 0;
	uint8_t bit = 0;
	uint8_t pos=0x80;
	stays = 0;
	set = fb[scan];


	while (pos) {
		if (set & pos) {
			//uint8_t col_map = row_col_map[bit]&0x0f;
			uint8_t col_map = row_col_map[bit] >> 4;
			if (col_map&8)
				out2 |= 1<<(col_map&7);
			else
				out1 |= 1<<(col_map);
			stays++;
		}//if
		bit++;
		pos >>= 1;
	}//while

	if (stays) {	// load dimming value
		stays <<= 1;
		stays |= menu_val[1]<<3;
	}//if

	ddr2 = out2;
	ddr1 = out1;

	out2 = ddr2;
	out1 = ddr1;
	//____________ turn on current row
	if (row_map & 0x08) {
		row_map &= 0x07;
		out2 &= ~(1<<row_map);
		ddr2 |= (1<<row_map);
	}//if
	else {
		out1 &= ~(1<<row_map);
		ddr1 |= (1<<row_map);
	}//else

	P2DIR = ddr2;
	P1DIR = ddr1;
	P2OUT = out2;
	P1OUT = out1;

	scan++;
	scan &= 0x07;
}//scan

#define ST_BUSY	BIT0

/*
	fb[8]
	fb[0].....fb[7]
	0 0 0 0 0 0 0 0 bit7
	0 0 0 0 0 0 0 0
	0 .............
	............. 0
	0 0 0 0 0 0 0 0 bit0
*/
const uint8_t menu_icon[3][8] = {
	{		// hamming on/off
	0b00111110,
	0b00100010,
	0b00111110,
	0b00101010,
	0b00111110,
	0b00100010,
	0b00111110,
	0b00000000,
	},
	{		// dimmer
	0b00010000,
	0b01010100,
	0b00111000,
	0b11101110,
	0b00111000,
	0b01010100,
	0b00010000,
	0b00000000,
	},
	{		// sampling rate
	0b00010000,
	0b00111000,
	0b01010000,
	0b10000000,
	0b10000000,
	0b01000100,
	0b00111000,
	0b00000000,
	},
};
//______________________________________________________________________
void main(void) {
	//WDTCTL = WDTPW + WDTHOLD;		// Stop WDT
	WDTCTL = WDTPW|WDTHOLD|WDTNMI|WDTNMIES;	// stop WDT, enable NMI hi/lo
	BCSCTL1 = CALBC1_8MHZ;			// 16MHz clock
	DCOCTL = CALDCO_8MHZ;

	P1SEL = P2SEL = 0;
	P1DIR = P2DIR = 0;
	P1OUT = P2OUT = 0;

	//while (1) { __asm(" nop"); }
	//______________ adc setting, use via microphone jumper on educational boost
	ADC10CTL0 = SREF_1 + ADC10SHT_2 + REFON + ADC10ON;// + ADC10IE;
	ADC10CTL0 |= REF2_5V;

	ADC10CTL1 = INCH_0;                       // input A0
	//ADC10AE0 |= BIT0;                         // P1.0 ADC microphone if we use interrupt


	//______________ setup timer
	TA0CCR0 = TA0CCR1 = 0;
	TA0CCTL1 = CCIE;			// our led scan
	TA0CCR1 = scan_at;

	TA0CTL = TASSEL_2 + MC_2 + TAIE;	// smclk, continous mode
	_BIS_SR(GIE); 						// now

	//state |= ST_HOLD;

	uint8_t i=0;


#define log2FFT   4
#define FFT_SIZE  (1<<log2FFT)
#define Nx        (2 * FFT_SIZE)
#define log2N     (log2FFT + 1)
#define BAND_FREQ_KHZ	8



	int16_t sample[Nx];
	int8_t data[Nx], im[Nx];			// 32+32 bytes
	uint8_t plot[8] = {  0, 0, 0, 0, 0, 0, 0, 0, };

	const unsigned char hamming[3][16] = { 
		{ 112, 126, 141, 155, 168, 181, 193, 205, 215, 225, 233, 240, 246, 250, 253, 255, },
		{  23,  35,  50,  67,  85, 105, 126, 146, 167, 186, 204, 220, 233, 244, 251, 255, },
		{   4,   9,  18,  29,  44,  61,  82, 105, 129, 154, 179, 202, 221, 237, 249, 254, },
	};
	const uint16_t lvls[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 16, 22, 32, 45, 63, 89,  };
	//                       { 1k .. 2k 3k 4k 5k  6k  7.5k
	const  uint8_t pick[8] = { 2, 3, 4, 6, 8, 10, 12, 15, };

	uint8_t agc = 0;		// automatic gain control?
	while (1) {
		state |= ST_BUSY;
		TA0CCR0 = TA0R;
		TA0CCTL0 |= CCIE;

		// round(255 * window('kr',32,2))
		// round(255 * window('kr',32,4))
		// round(255 * window('kr',32,6))

		int16_t offset=0;

		for (i=0;i<Nx;i++) {

			// time delay between adc samples
			// this will become the band frequency after time - frequency conversion
			
			TA0CCTL0 &= ~CCIFG;
			TA0CCR0 += (8000/(BAND_FREQ_KHZ*2))-1;	// begin counting for next period
			ADC10CTL0 |= ENC + ADC10SC;			// sampling and conversion start
		//_BIS_SR(LPM0_bits + GIE);			// wake me up when done, not used, need led running
			while (ADC10CTL1 & ADC10BUSY);		// stay and wait for it
			ADC10CTL0 &= ~ENC;
			uint16_t adc = ADC10MEM;

			sample[i] = adc;
			offset += adc;

			//data[i] = (adc>>2);		// high gain?
			//data[i] = adc-512;		// 3.6v supply
			//data[i] = adc-512+64;		// 3V supply

			//data[i] = adc>>2;
			//data[i] = adc-512+64;
			data[i] = adc;
			im[i] = 0;

			_BIS_SR(LPM0_bits + GIE);			// wake me up when timeup

		}//for
		state &= ~ST_BUSY;
		TA0CCTL0 &= ~CCIE;

		// averaging
		offset >>= (log2FFT+1);

		for (i=0;i<Nx;i++)
			data[i] = (sample[i] - offset) >> 2;

		if (menu_val[0]) {		// hamming window requested? we have 3 lists to use
			for (i=0;i<Nx;i++) {
				int16_t hamm = hamming[menu_val[0]-1][i<(FFT_SIZE-1)?i:(Nx-1)-i] * data[i];
				data[i] = (hamm >> 8);
			}//for
		}//if

		fix_fft(data, im, log2N, 0);

		uint8_t peaks = 0;

		for (i=0;i<8;i++) {
			uint8_t idx = pick[i];
			data[idx] = sqrt16(data[idx]*data[idx] + im[idx]*im[idx]);
			//_______ logarithm scale mapping
			uint8_t c = 8;
			while ((data[idx] < lvls[c+agc]) && (--c));
			data[idx] = c;

			if (data[idx] < 0) data[idx] = 0;
			if (data[idx] > plot[i]) {		// advance is immediate
				plot[i] = data[idx];
			}//if
			else { 		// decline is not
				if (data[idx] < plot[i])
					plot[i]--;
			}//else
			if (plot[i] >= 7) peaks++;

			if (bar) {
				uint8_t dot=1;
				if (bar == 3 && plot[i] >= 2) {
					dot = 0x07;
				}//if
				else {
					if (bar >= 2 && plot[i] >= 1)
						dot = 0x03;
				}//else
				fb[i] = dot<<plot[i];
			}//if
			else {
				fb[i] = 0;
			}//else

			//fb[i] = dot<<plot[i];
		}//for
		if (peaks >= 2 && agc < 10) agc++;		// too many band peaked, turn down sensitivity
		if (!peaks && agc) agc--;				// none of the banks peaked, turn up sensitivity
		//____________ delay between each sampling
		ticks = menu_val[2]; while (ticks) __asm(" nop");

		while (pixel_test) {
			for (i=0;i<8;i++) fb[i] = 0;
			fb[pixel_test>>3] = 1<<(pixel_test&0x07);
			ticks = 61; while (ticks) __asm(" nop");
			pixel_test--;
		}//while
		//___________ show menu description
		while (menu) {
			if (state & ST_PAINT) {
				state &= ~ST_PAINT;
				for (i=0;i<8;i++) fb[i] = menu_icon[menu-1][i];
				//if (menu_val[menu-1])
					fb[7] = 1<<menu_val[menu-1];
			}//if
		}//while
	}//while

} 
 
// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void) {
	__bic_SR_register_on_exit(CPUOFF);
}

// Timer A0 interrupt service routine
//
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer0_A0 (void) {
	//P1OUT ^= BIT0;
	__bic_SR_register_on_exit(CPUOFF);
	//state &= ~ST_BUSY;
}
//
//
//________________________________________________________________________________
//interrupt(TIMERA1_VECTOR) Timer_A1(void) {
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer0_A1 (void) {
	switch(TAIV) {
		case  2: 
			CCR1 += scan_at;
			scan_led();
			break;
		case 10: 
			if (ticks) ticks--;
			break;
	}//switch
}
//
