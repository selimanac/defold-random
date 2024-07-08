![Defold - PCG Random](/.github/pcgrandom-hero.png?raw=true)

# Defold - PCG Random

PCG Random Number Generator Native Extension for the Defold Game Engine

This extension allow you to generate random numbers using minimal [C implementation of PCG](http://www.pcg-random.org/using-pcg-c-basic.html).

It uses [entropy](https://github.com/imneme/pcg-c/blob/master/extras/entropy.c) seed internally with fallback to time based seed. You can switch to Time based seed and remove the entropy by uncommenting/commenting a few lines on the source code, but I don't think it is necessary. 

## Installation
You can use PCG Random in your own project by adding this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/). Open your game.project file and in the dependencies field under project add:

	https://github.com/selimanac/defold-random/archive/master.zip
	
---

## Toss a Coin to Your Witcher
If you find my [Defold Extensions](https://github.com/selimanac) useful for your projects, please consider [supporting](https://github.com/sponsors/selimanac) it.  
I'd love to hear about your projects! Please share your released projects that use my native extensions. It would be very motivating for me.



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

### rnd.double()

Returns a floating point between 0-1.  
Same as **math.random()**

### rnd.double_range(`min`, `max`)

Returns a floating point between min - max.  
Not fast as `rnd.double()`

### rnd.dice(`roll`, `type`)

DnD style dice roller.   

**PARAMETERS**

* ```roll``` (int) - Roll amount
* ```type``` (enum) - Type of the dice

	**rnd.d4** : D4: four-sided die.       
	**rnd.d6** : D6: six-sided die.    
	**rnd.d8** : D8: eight-sided die.    
	**rnd.d10** : D10: ten-sided die (0-9).   
	**rnd.d12** : D12: twelve-sided die.    
	**rnd.d20** : D20: twenty-sided die.    
	**rnd.d100** : D%: percentile die (0-90).    
	
**RETURN**

* ```result``` (table) - Dice results
* ```total``` (int) - Total amout of dice results

**EXAMPLE**

```lua

local result, total = rnd.dice(2, rnd.d10)

```


### rnd.toss()

Toss a coin. Returns 0 or 1 (0 = 'H', 1 = 'T')

### rnd.roll()

Roll the dice. Returns between 1-6

### rnd.check()

Testing entropy.



## Release Notes

1.2.7

- rnd.dice added. 

1.2.6

- Fix for [#6](https://github.com/selimanac/defold-random/issues/6#issue-951284950)
- Fix for [#7](https://github.com/selimanac/defold-random/issues/7#issue-951982666)
- Auto-complete for native extensions 

1.2.5

- Fix for [#4](https://github.com/selimanac/defold-random/issues/4#issue-837151758)

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