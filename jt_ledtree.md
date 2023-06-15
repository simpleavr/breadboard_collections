




## Joule Thief Mini Christmas Tree

```[December 2017]``` Originally created.

A minimalist 1.5V coin cell led tree with all components and power source confined within a mini breadboard.
<iframe width="560" height="315" src="https://www.youtube.com/embed/r7xlT_eYZ-I" frameborder="0" allow="autoplay; encrypted-media" allowfullscreen></iframe>


<br><img src="images/jt_ledtree_01.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/><br><br>


### Description

This is a holiday project targeting the 2017 December [HackaDay](https://hackaday.io/project/28627-joule-thief-mini-christmas-tree) coin cell challenge.

The objective is to employ a 1.5V button cell to drive microcontroller based holiday led display.

A minimalist design with all components and power source confined within a mini breadboard.

A joule thief circuit is used to step up the voltage from the 1.5V button cell to power the microcontrollers and leds.

### Details
<br><img src="images/jt_ledtree_02.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/><br><br>


- A joule thief circuit is used to step up the voltage from the 1.5V button cell to power the microcontrollers and leds.
- A push button instead of a switch is used to complete the joule thief circuit.
- Initial push button depress starts the joule thief and supply start up voltage to the microcontroller (which operates at 1.8V to 3.6V).
- A zener diode limits the maximum voltage to 3.3V. This can be replaced by a 3.6V zener if higher voltage is needed
- Once the microcontroller boots up, and the push button releases, the microcontroller takes control of the joule thief.
- Microcontroller samples the supply voltage via internal ADC and switch on the joule thief (mimicking a push button depress) when voltage is less than 3.2V and switch it off when voltage is below 3.2V. This provide certain efficiency benefits to the otherwise inefficient joule thief circuit. The voltage threshold can be altered via source code change.
- The led christmas tree cycles in a fixed lighting sequence.
- There are 4 + 1 different display modes; breathing, solid, throbing, and twinkle, plus an automatic cycling of the four modes.
- The display starts w/ a breathing display mode when initially turned on, and cycles between the above-mentioned 4+1 modes upon keypress.
- The same I/O pin is used for switching the joule thief and for push button reading. The firmware switches the I/O pin periodically to input and reads the button state.
- A long keypress will shut-down the display via switching off the joule thief, leading to collapse of the voltage supplying to the microcontroller.
- The entire build incluing power is on a 170 tie-point mini breadbroad.
- This project demonstrates an switching joule thief design, timers and interrupts, analog digital conversion usage, and LED multiplexing / charlieplexing.


### Application Notes
<br><img src="images/jt_ledtree_03.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/><br><br>


- Single button press turns on led tree
- Subsequent single button press (indicate by top led on) rotates these display modes; breathing, solid, throbing, and twinkle, and cycling
- Long press (indicate by 2 led on) turns off led tree


### Breadbroad Layout



```
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
                                           (.>), (<.) 10 x LEDs
                                               
                                               
```



### Schematic



```
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

```



### Assembling
<br><img src="images/jt_ledtree_04.jpg" alt="IMG" style="border:2px solid #555;margin-right:10px" width="360"/><br><br>


- Follow breadboard layout and place jumper wire on mini breadboard
- Place joule thief components with larger components go first
- Place MSP430G2231 on breadboard
- Cut your LEDs into different lengths, *be sure to maintain one leg longer
- The top most LED need extra long legs via soldering cut legs from a sibling
- Place LEDs accordingly, you can substitute different color LEDs that works up to 3.2V
- Battery Holder was made by reshaping pins in a double row male pin header section
- Parts / Bill of Materials


This is what's needed for this project

- 1 x MSP430G2311 (can be substitute by MSP430F2012
- 1 x 170 tie-point mini breadboard
- 1 x 4.7k, 1 x 100k resistors
- 1 x 10pF ceramic capacitor
- 1 x 47uF electrolytic capacitor
- 1 x 100uH inductor
- 2 x BC547 or any small signal NPN transistor
- 1 x 1N5819 or similar schottly diode
- 1 x 1N5228 or any 3.3V zener diode (500mW)
- 1 x tactile switch
- Jumper wires
- Double row male pin header to be used for battery holder
- 1 x LR44 1.5V coin cell battery


### Source code

Source code usually resides in my github repositories.

For this particular project, the single C source file jt_ledtree.c is bundled in my [breadboard collections repository](https://github.com/simpleavr/breadboard_collections). You just need jt_ledtree.c



