# Defold - PCG Random

PCG Random Number Generator Native Extension for the Defold Game Engine

This extension allow you to generate random numbers using minimal [C implementation of PCG](http://www.pcg-random.org/using-pcg-c-basic.html).

It uses [entropy](https://github.com/imneme/pcg-c/blob/master/extras/entropy.c) seed internally with fallback to time based seed. You can switch to Time based seed and remove the entropy by uncommenting/commenting a few lines on the source code, but I don't think it is necessary. 

## Installation
You can use PCG Random in your own project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:

	https://github.com/selimanac/defold-random/archive/master.zip
	
---

## Usage

### rnd.number()

Returns a 32 bit integer

### rnd.range(min, max)

Returns a 32 bit integer between min and max values.  
Same as **math.random(3,20)**  
**math.random(90)** => rnd.range(1, 90)

###  rnd.double()

Returns a floating point between 0-1.  
Same as **math.random()**

###  rnd.toss()

Toss a coin. Returns 0 or 1 (0 = 'H', 1 = 'T')

###  rnd.roll()

Roll the dice. Returns between 1-6

###  rnd.check()

Testing entropy.



