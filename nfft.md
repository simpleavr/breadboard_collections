




## Breadboard Audio Spectrum Analyser

```[October 2015]``` Originally created.
<iframe width="560" height="315" src="https://www.youtube.com/embed/ODHvRk2EyS0" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>



Breadboard Audio Spectrum Analyzer.
<br><img src="images/nfft01.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/><img src="images/nfft02.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/><br><br>

### Description

This is a resurrected project from a breadboard prototyping exercise I did on 2013. The core logic is the same as my 8 Bit FFT Spectrum Analyzer, except I am using a slightly better op-amp and I have LED matrix for display instead of an LCD module. Also the number of bands shown is 8 instead of 16.

This project is microphone based and require minimal external components. 2 x LR44 coin cells are used so that I can have the whole structure working in the confines of a 170 tie-point mini breadboard.

ADC10, TimerA interrupt LPM wakeup, TimerA PWM like output, button use, integer arithmetic are used and demonstrated.

### Features

- 8 bit integer FFT
- 16 samples at 500Hz separation
- shows 8 amplitudes of 1K, 1.5K, 2K, 3K, 4K, 5K, 6K, 7.5K non-linear
- partial logarithm map to show amplitudes, limited as resolution has been reduced for 8 bit FFT
- TLC272 one stage mic pre-amp at 100x times 100x gain (you can experience w/ 2 stages)
- menu selectable optional Hamming window
- menu adjust 4 levels brightness
- menu adjust 8 levels sample rate / response time
- 2 x LR44 coin cell powered "on board"


### Parts / Bill of Materials

This is what's needed for this project

- MSP430G2452 (the extra chip from TI Launchpad G2, or any 4K 20 pin MSP430G series MCU)
- a 170 tie-point mini breadboard or perf board for pre-amp construction
- a TLC272 Dual op-amp
- mini electret microphone
- 47k (pull-up), 100k, 2 x 10k, 1k resistors
- 1 x 0.1uF
- jumper wires
- double row male pin header to be used for battery holder
- 2 x LR44 coin cell battery


### Schematic



```
     MSP430G2452 or similar, need 4K Flash
     TLC272 Dual Op-Amp, GBW @1.7Mhz, @x100 gain, bandwidth up to 17Khz

   * we are using one stage of the TLC272 only

                                              ._____________.
                                              | MSP430G2452 |  Vcc
                                              |             |   |
                     +-----------------------2|ADC0         |1--+
                     |                        |             |   |.
                     |                    Vcc |             |  | | pull-up (47k)
   Vcc     Vcc       |   ---------------   |  |             |  |_|
    |       |        +-1|----.       Vcc|8-+  |             |   |
    |.      |.       |. |    ^      .---|7    |             |16-+
   | |10k  | |10k   | | |   / \     ^   |     |             |
   |_|     |_|  100k|_| |  /__+\   / \  |     |            /|--- (see breadboard layout)
    | .1u   |        |  |   | |   /__+\ |     |           / |------_______
    +--||---|-[ 1k]--+-2|---+ |    | |  |     |    15 GPIO  |     |       |
    |       +----------3|-----+    +-|--|6    |    P1.1-P1.7|     | 8x8   |
    |       |        +-4|Gnd         +--|5    |    P2.0-P2.7|     | LED   |
    |+      |        |   ---------------      |             |     | matrix|
  ((O))     |.       |                        |           \ |     |_______|
    |MIC   | |10k    |                    +-20|Gnd         \|--------
    |      |_|       |                    |   |_____________|
   _|_     _|_      _|_                  _|_
   ///     ///      ///                  ///

```



### Application Notes
<br><img src="images/nfft_app01.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/><img src="images/nfft_app02.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/><br><br>

- Short key press in display mode cycles through no dot, one dot, 2 dots, and 3 dots display.
- Long press enters setup mode, subsequent long press rotates thru menu.
- Menu items cycles thru 'Hamming Window Option', 'Dimmer', 'Sampling / Refresh Rate'.
- In 'Hamming Window' setup mode, short presses cycles through no hamming, hamming 1, hamming 2, hamming 3, long press confirms setting.
- In 'Dimmer' setup mode, short presses cycles through available brightness levels from 0 to 3, long press confirms setting.
- In 'Sampling / Refresh rate' setup mode, short presses cycles through the available refresh rates from 0 to 7, 0 means no delay, long press confirms setting.
- Led segment multiplexing includes time delays to compensate for brightness differences for individual rows.


### LED Driving

The LED matrix is of 8 x 8 elements. They are driven by 15 GPIO pins. They are multiplexed w/ 8 rows and 8 column scheme. Since there are only 15 pins after we use 1 pin for ADC input, the multiplexing has row 1 and column 0 sharing a single pin. This means that the particular LED on row 1 and column 0 cannot be lit. This is a compromise as there are just not enough GPIO pins to drive all LED elements.

### Sound Capture

Sound is capture via the on board condenser microphone on the Educational BoosterPack. As microphone signals are small, we need to amplify it to a level that the msp430 ADC10 can use w/ a reasonable resolution. I had used a two-stage op-amp amplifier for this purpose.

The op-amp amplifier is consist of two stages, each w/ a about 100x gain. I had adopt the TLC272 as it is also a very common part and it works w/ 3V. The gain bandwidth being about 1.7Mhz means that for our gain of 100x, we can only guarantee it would work nicely (i.e. maintain the gain we want) under 17Khz. (1.7Mhz / 100).

Originally I intend to make this spectrum analyzer measure up to 16-20Khz, but in the end I found about 8Khz is good enough to show music. This can be changed by replacing the LM358 w/ something of audio-rated and changing the sampling rate. Just look for the gain bandwidth of the op-amps you choose.

### Sampling and FFT

The FFT function used is the "fix_fft.c" code that many projects had adopted, it has been floating around in the internet for some years. I had tried a 16 bit version and a 8 bit version. Eventually I settled for the 8 bit version as for my purpose, I did not see a major advance on the 16 bit version.

I do not have a good understanding of the FFT mechanism except that it's a time domain to frequency domain conversion. That means the rate (time) of the sound samples, after feeding to the FFT calculation function, will affect the frequency of the amplitude I am getting as a result. So by adjusting the rate to sample sound, I can determine the frequency band as the result.

TimerA 0 CCR0 is used to keep the sampling time. We first determine the counts we need to achieve the band frequency (corresponds to our DCO clock rate of 16Mhz). i.e. TA0CCR0 set to (8000/(BAND_FREQ_KHZ*2))-1; where BAND_FREQ_KHZ is 8 for me. It can be changed if you got a better op-amp and / or wants it be different.

Frequency Bands and Amplitude Scaling

The firmware process 16 bands at one sweep, and the capture timing produces 500Hz separation between these banks. The LED matrix is of 8 columns and will only display 8 bands / amplitudes. Instead of displaying one every two bands, a non-linear frequency band list is used to show the more dynamic frequency bands (in terms of music). The list is of 500Hz gaps at the low end, 1KHz gaps in the middle bands and 1.5Khz bands in the highs.

The amplitude of individual bands are scaled down to 8 levels, which are represented by the number of horizontal 'dots' on the LED matrix display.The amplitude levels are scaled down via a non-linear map that translates FFT results into one of the 8 dots. A sort-of logarithmic scaling is used as it best represent our perception of sound levels.

There is built-in AGC logic and the spectrum analyser will try to scale down the amplitude levels when there are multiple peak levels detected in the previous cycles. This is done with a sliding ruler comparing table.

### Breadboard Layout



```

         G  V+            Gnd              (1 stage layout) WE ARE USING THIS LAYOUT
   +=====================================================+          c0............c7
   |  MIC.  .  .  .  .  .  .  +-----+  +--+  .  .  .  .  |        r0 o o o o o o o o
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

   we are using one stage of the TLC272 only
```



### Building the project

You can start w/ the breadboard layout. And it's ASCII art so may not be very clear. You can pair w/ the photo below to identify all connections.
<br><img src="images/nfft_build01.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/><br><br>


The battery holder is made from a section of double male header pins. Use a pair of long nose pliers to pull out the middle pins, then bent the end pins to fit the two LR44 cells properly. Since there is no power switch, I use a thin piece of plastic tab to break the connection between the LR44s when not in use.
<br><img src="images/nfft_build02.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/><br><br>


I am using mps430-gcc to compile the firmware but it should go well with TI CCS. You can avoid all the trouble of installing IDEs or compilers by going to TI CCS cloud, which is a web based IDE. I will even download the firmware to your target device.



```
msp430--gcc -Os -Wall -ffunction-sections -fdata-sections -fno-inline-small-functions -Wl,-Map=nfft.map,--cref -Wl,--relax -Wl,--gc-sections -I/cygdrive/c/Users/chrisc/Desktop/energia-0101E0016/hardware/msp430/cores/msp430 -mmcu=MSP430G2553 -o nfft.elf nfft.c
```



I am using a TI Launchpad G2 as a programmer to program the MCU. I am not covering the installation / use of the tools as there are well written tutorials to cover that.

### Source code

Source code usually resides in my github repositories.

For this particular project, the single C source file nfft.c is bundled in my [breadboard collections repository](https://github.com/simpleavr/breadboard_collections). You just need nfft.c



