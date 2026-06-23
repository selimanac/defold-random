![Defold - PCG Random](/.github/pcgrandom-hero.png?raw=true)

# Defold - PCG Random

PCG Random Number Generator Native Extension for the Defold Game Engine

This extension allow you to generate random numbers using minimal [C implementation of PCG](http://www.pcg-random.org/using-pcg-c-basic.html).

It uses the upstream PCG [entropy](https://github.com/imneme/pcg-c/blob/master/extras/entropy.c) helper internally when `rnd.seed()` is called without arguments or when the extension initializes. The vendored `entropy.c` file is intentionally kept close to upstream PCG. Project-specific behavior, such as checking whether OS entropy succeeded, falling back to the PCG fallback seed source, and logging the selected seed source, is handled in the Defold binding layer.

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


`init_state` is the starting state for the RNG.
`init_seq` selects the output sequence for the RNG, although only the low 63 bits are significant.

Lua numbers cannot exactly represent every 64-bit integer. For deterministic repeatability, use 32-bit unsigned integer seed values.

### rnd.seed32(`init_state`, `init_seq`)

Seeds the random number generator with exact unsigned 32-bit integer values.

Use this method when you need deterministic repeatability from Lua without relying on 64-bit number precision.

### rnd.seed()

Re-seed the random number generator by using entropy seed.  
Random number generator is always initialized by using entropy seed. You don’t need to call this method unless you want to re-seed.

### rnd.number()

Returns a 32 bit unsigned integer.

### rnd.numbers(`count`)

Returns a table of 32 bit unsigned integers.

`count` must be a non-negative integer. `rnd.numbers(0)` returns an empty table.

### rnd.range(`min`, `max`)

Returns a 32 bit unsigned integer between min and max values. Only for positive numbers(unsigned integers).   
Same as **math.random(3,20)**  
**math.random(90)** == rnd.range(1, 90)

### rnd.ranges(`count`, `min`, `max`)

Returns a table of 32 bit unsigned integers between min and max values.

`count` must be a non-negative integer. `min` and `max` must be unsigned 32-bit integers.

### rnd.double()

Returns a floating point between 0-1.  
Same as **math.random()**

### rnd.doubles(`count`)

Returns a table of floating point numbers in `[0, 1)`.

`count` must be a non-negative integer. `rnd.doubles(0)` returns an empty table.

### rnd.double_range(`min`, `max`)

Returns a floating point between min - max, using the range `[min, max)`.
Not fast as `rnd.double()`

### rnd.boolean()

Returns `true` or `false` with equal probability.

### rnd.chance(`probability`)

Returns `true` according to the given probability.

Values less than or equal to `0` always return `false`. Values greater than or equal to `1` always return `true`.

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

### rnd.shuffle(`array`)

Shuffles the array part of a table in place using Fisher-Yates and returns the same table.

```lua

local items = { "a", "b", "c", "d" }
rnd.shuffle(items)

```

### rnd.check()

Debug/sample method that prints PCG sample output and consumes RNG state.



## Release Notes


1.2.8

- Added `rnd.seed32(init_state, init_seq)` for exact deterministic unsigned 32-bit seeding from Lua.
- Added batch generators:
  - `rnd.numbers(count)`
  - `rnd.ranges(count, min, max)`
  - `rnd.doubles(count)`
- Added probability helpers:
  - `rnd.boolean()`
  - `rnd.chance(probability)`
- Added `rnd.shuffle(array)` for in-place Fisher-Yates table shuffling.
- Added `pcgrandom/annotations.lua` 
- Added missing `rnd.dice()` documentation to `pcgrandom.script_api`.
- Added entropy reseed logging showing whether OS entropy or fallback entropy was used.
- `rnd.seed()` now reseeds from entropy only when called with no arguments.
- `rnd.seed(0, seq)` is now valid deterministic seeding behavior.
- `rnd.double_range(min, max)` now generates values in `[min, max)`, matching `rnd.double()` semantics.





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
