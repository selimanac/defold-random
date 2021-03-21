# Defold - PCG Random

PCG Random Number Generator Native Extension for the Defold Game Engine

This extension allow you to generate random numbers using minimal [C implementation of PCG](http://www.pcg-random.org/using-pcg-c-basic.html).

It uses [entropy](https://github.com/imneme/pcg-c/blob/master/extras/entropy.c) seed internally with fallback to time based seed. You can switch to Time based seed and remove the entropy by uncommenting/commenting a few lines on the source code, but I don't think it is necessary. 

## Installation
You can use PCG Random in your own project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:

	https://github.com/selimanac/defold-random/archive/master.zip
	
---

## Usage

### rnd.seed(`init_state`, `init_seq`)

Seeds the random number generator.   
Random number generator is always initialized by using entropy seed. You don't need to call this method unless you want to control the seed.


`init_state` is the starting state for the RNG, you can pass any 64-bit value.  
`init_seq` selects the output sequence for the RNG, you can pass any 64-bit value, although only the low 63 bits are significant.
 
**Caution:** I don't recommend using of 64-bit integers. Consider using 32-bit integers instead. 

### rnd.seed()

Re-seed the random number generator by using entropy seed.  
Random number generator is always initialized by using entropy seed. You don’t need to call this method unless you want to re-seed.

### rnd.number()

Returns a 32 bit unsigned integer.

### rnd.range(`min`, `max`)

Returns a 32 bit unsigned integer between min and max values. Only for positive numbers(unsigned integers).   
Same as **math.random(3,20)**  
**math.random(90)** == rnd.range(1, 90)

###  rnd.double()

Returns a floating point between 0-1.  
Same as **math.random()**

###  rnd.double_range(`min`, `max`)

Returns a floating point between min - max.  
Not fast as `rnd.double()`

###  rnd.toss()

Toss a coin. Returns 0 or 1 (0 = 'H', 1 = 'T')

###  rnd.roll()

Roll the dice. Returns between 1-6

###  rnd.check()

Testing entropy.



## Release Notes  

1.2.4

- `rnd.double_range(min, max)` added.

1.2.3

- `rnd.range` returns min if min == max.

1.2.2

- `rnd.range` was causing a crash when MIN is bigger than MAX. Error message added.

1.2.1

- Added static seed
- Small fix.

1.1

- Fixed integers.

1.0

Initial release.