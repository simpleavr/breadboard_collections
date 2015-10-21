# Breadboard Collections

A collection of MSP430 mini projects (mostly LED) that requires no soldering and can be easily assembled w/ a 170 tie-point mini-breadboard

Tix Clock Gems (tix.c)
______________________

Tix Clock Implemented In a Gem Mine Diorama

Application Notes

Single button press shows time for a few seconds, TCG returns to sleep after that
Long press setup time, starting with the hours, related LEDs starts to blink
Short press advance the hour value. Another Long press confirms the entry
The tenths of the minutes and the minutes are subsequently entered like the hour
Once the hours and minutes (tenths and singles) have been entered, TCG returns to show mode
 
Breadboard Layout
_________________

```
   +=====================================================+
   |  .  o-----(W3)-----o  .  .  .  .  .  .1>.  .8>.  .  |
   |  . (-) .  .  .  .  .  .  .  .  .  .+>.  .4>.  .3>.  |
   |                \.  .  oXTo  .  .  .  .<5.  .<6.  .  | XT 32Khz Clock Crystal
   |                 :+-(W1)--------o  .<7.  .<2.  .<9.  | W1 +ve battery to IO (reset)
   | Battery Holder  :| ---+--+--+--+--+--+--+--+--+  o  | W2 b2 to led extension column
   |                 :||- b6 b7 CK IO a7 a6 b5 b4 b3| |  | W3, W4 battery power to MCU
   |                 :||+ a0 a1 a2 a3 a4 a5 b0 b1 b2|(W2)|
   |                 :| ---+--+--+--+--+--+--+--+--+  |  |
   |                 :| .  .  .1>.  .9>.  .1>.  .2>.  |  | > side of led is anode (longer leg)
   |                /.| .  .2>.  .3>.  .4>.  .3>.  o--+  |
   |  . (+) .  .  .  .| .  .  .<6.  .<5.  .<6.  .<5.  .  |
   |  .  o-----(W4)---+-o-[B]-o  .<4.  .<7.  .<8.  .  .  |
   +=====================================================+

```


Schematic
_________
```

                            MSP430G2xxx
                         -----------------
                        |                 |                   /|\
                     <--|TEST        RESET|--------------------+
                        |                 |                    | 
               ~~~~~~~~ |P2.0         P1.0|--> ~~~~~~~    _|_  |
              {         |P2.1         P1.1|-->        }--o   o-+ button
              { Magic   |P2.2         P1.2|-->        }
              {  LOL    |P2.3         P1.3|-->  Many  }
              {         |P2.4         P1.4|-->  LEDs  }
               ~~~~~~~~ |P2.5         P1.5|-->        }
      32Khz /-----------|P2.6(XIN)    P1.6|-->        }
    Crystal \-----------|P2.7(XOUT)   P1.7|--> ~~~~~~~ 
                        |                 |

```
