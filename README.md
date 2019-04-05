# Defold - PCG Random

PCG Random Number Generator Native Extension for the Defold Game Engine

This extension allow you to generate random numbers using minimal [C implementation of PCG](http://www.pcg-random.org/using-pcg-c-basic.html).

It uses entropy seed internally. You can switch to Time based seed by uncommenting lines(Check the source code). 

## Installation
You can use PCG Random in your own project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:

	https://github.com/selimanac/defold-random/archive/master.zip
	
---

## Usage

### rnd.number()

Returns a 32 bit integer

### rnd.range(min, max)

Returns a 32 bit integer between min and max values.

###  rnd.double()

Returns a floating point between 0-1

###  rnd.toss()

Toss a coin. Returns 0 or 1 (0 = 'H', 1 = 'T')

###  rnd.roll()

Roll the dice. Returns between 1-6

###  rnd.check()

Testing entropy.



