




## Mini Simon Game

```[October 2015]``` Originally created.

```UNDER CONSTRUCTION and source code not yet avaliable```

Minimalist's Simon Game on a Breadboard

### MSG Mini Simon Game



### Description

This is a re-creation of a 70s game that tests memory. There are four different coloured LEDs and four buttons. The game play starts with blinks of randomized LEDs starting from one blinks. The player is to response and press the corresponding button. After that additional steps / LED blinks are added to the current sequence and repeats until the player fails to play the machine generated sequence.

A buzzer provides four different tone of sound when individual LEDs lights up.

This project does not have a power button, upon in-activity the MCU goes into deep sleep (LPM4) mode that consume nA levels of power.

The MSG (Mini Simon Game) demonstrates a lot of IO techniques and advance use of timer, for period capture and pwm tone generation.

### Features

- Minimal jumper wires, 4 on a mini breadboard
- Personalization via LED placements and leg lengths
- Battery operated from 3V
- Use of 32Khz clock crystal to keep time, power-down sleep mode (LPM3) takes less than 1uA power
- Parts list
- 170 tie point mini breadboard
- MSP430F2011 (or other G/F series dip 14 pin devices w/ 2k flash)
- 4 LEDs (blue, green, red and yellow)
- 4 Tactile button
- 9mm Buzzer
- CR2032 button cell holder (battery not included)


### Application Notes

- To start game, press blue button, blue LED will blink three times
- Game play starts w/ the first blink
- Player response by pressing the corresponding button
- Player is considered failed if player press wrong button
- If there are no button press within 5 secounds, player is considered failed
- Game advances by introducing one more blink for each completed response
- Once failed, game is placed in deep sleep mode to conserve energy


### Breadboard Layout



```
   +=====================================================+
   |  .  o-----(W1)--------o(W1a)o  .  .  .  .  .  .  .  | W1, W1a battery to -ve, buzzer (-)
   |  . (-) .  .  .  .  . BZR .  o=BTN=o  .  o=BTN=o  .  | W2 battery to +ve
   |                \   .  +  .+--(W3)-------+  .  .  .  | W3 +ve to IO (reset)
   |                 :  .  o  .| .  o>o  .  | W4 a2 to buzzer (+)
   |                 :  .  |  .| +--+--+--+--+--+--+     |
   | Battery Holder  :     |   ||- b6 b7 CK IO a7 a6| .  | D   A
   |                 :    (W4) ||+ a0 a1 a2 a3 a4 a5|    | C   B
   |                 :  .  |  .| +--+--+--+--+--+--+  .  |
   |                 :  .  |  .| .  o>>o  .  .  o>>o  .  |
   |                /.  .  +---|----------o  .  .  .  .  |
   |  . (+) .  .  .  .  .  .  .| o=BTN=o  .  o=BTN=o  .  |
   |  .  o-----(W1)------------+-o  .  .  .  .  .  .  .  |
   +=====================================================+
```



### Schematic



```
                            MSP430G2xxx
                         -----------------
                        |                 |                   /|\
                N.c. <--|TEST        RESET|--------------------+
                        |                 |                    |
                        |             P1.0|----|>|--+          |
                        |                 |         |     _|_  |
                        |             P1.1|---------+----o   o-+
                        |                 |                    |
                        |             P1.7|----|>|--+          |
                        |                 |         |     _|_  |
                        |             P1.6|---------+----o   o-+
                        |                 |
                        |             P1.3|--------------------+
                        |             P1.4|----|>|--+          |
                        |                 |         |     _|_  |
                        |             P1.5|---------+----o   o-+
                        |                 |
                        |             P2.6|---|<|---+
                        |                 |         |     _|_
                        |             P2.7|---------+----o   o-+
                        |                 |                   _|_
                        |                 |                   ///
```



### Assembling

- Follow breadboard layout and place jumper wire on mini breadboard
- Place MSP430F2011 on breadboard
- Place LEDs
- Place Tactile Buttons
- Place Battery Holder


### Source code

Source code usually resides in my github repositories.

For this particular project, the single C source file simon.c is bundled in my [breadboard collections repository](https://github.com/simpleavr/breadboard_collections). You just need simon.c



