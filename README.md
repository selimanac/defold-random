![Defold - PCG Random](/.github/pcgrandom-hero.png?raw=true)

# Defold - PCG Random

PCG Random Number Generator native extension for the Defold Game Engine.

This extension generates random numbers with the minimal [C implementation of PCG](http://www.pcg-random.org/using-pcg-c-basic.html). Every random operation uses an explicit RNG context id, so deterministic systems can stay isolated from each other.

## Installation

Add this project as a [Defold library dependency](http://www.defold.com/manuals/libraries/):

	https://github.com/selimanac/defold-random/archive/master.zip

---

## Toss a Coin to Your Witcher

If you find my [Defold Extensions](https://github.com/selimanac) useful for your projects, please consider [supporting](https://github.com/sponsors/selimanac) it.
I'd love to hear about your projects! Please share your released projects that use my native extensions. It would be very motivating for me.

## Usage

### Contexts

Create an RNG context first. All random functions require the context id as the first argument.

```lua
local rng = rnd.new()
print(rnd.number(rng))
rnd.delete(rng)
```

The extension can keep up to 255 active RNG contexts at the same time. Delete contexts you no longer use with `rnd.delete(id)`, or clear all active contexts with `rnd.clear_contexts()`.

For deterministic map generation, create the context from a user-visible seed:

```lua
local map_seed = 12345
local map_rng = rnd.new(map_seed)

local tile = rnd.range(map_rng, 1, 30)
local noise = rnd.double(map_rng)

rnd.delete(map_rng)
```

`rnd.new(init_state)` uses `init_seq = 1`. Use `rnd.new(init_state, init_seq)` when you want to choose both unsigned 32-bit PCG seed inputs.

### rnd.new()

Creates an entropy-seeded RNG context and returns its id.

When no seed is provided, the extension uses the platform OS entropy source when available: `SecRandomCopyBytes` on Apple platforms, `BCryptGenRandom` on Windows, Web Crypto on HTML5, and `/dev/urandom` on Linux/Android. Unsupported platforms fall back to the bundled PCG time-based fallback entropy helper. The selected source is logged with `dmLogInfo`.

### rnd.new(`init_state`)

Creates a deterministic RNG context using an unsigned 32-bit `init_state` and `init_seq = 1`.

### rnd.new(`init_state`, `init_seq`)

Creates a deterministic RNG context using exact unsigned 32-bit seed values.

### rnd.reset(`id`)

Reseeds an existing context from entropy.

### rnd.reset(`id`, `init_state`)

Reseeds an existing context using an unsigned 32-bit `init_state` and `init_seq = 1`.

### rnd.reset(`id`, `init_state`, `init_seq`)

Reseeds an existing context using exact unsigned 32-bit seed values.

### rnd.delete(`id`)

Deletes one RNG context. The id becomes invalid.

### rnd.clear_contexts()

Deletes all RNG contexts. Existing ids become invalid.

## Exact 64-bit Seeds

Lua numbers cannot exactly represent every 64-bit integer, so exact 64-bit seed APIs use decimal strings.

```lua
local rng = rnd.new64("12345678901234567890", "42")
rnd.delete(rng)
```

### rnd.new64(`init_state`, `init_seq`)

Creates a context using exact unsigned 64-bit decimal string seed inputs.

### rnd.reset64(`id`, `init_state`, `init_seq`)

Reseeds a context using exact unsigned 64-bit decimal string seed inputs.

## Raw State Save And Restore

Raw state APIs use decimal strings so the full PCG state can be saved and restored exactly.

```lua
local state, increment = rnd.state(rng)
rnd.set_state(rng, state, increment)
```

### rnd.state(`id`)

Returns raw PCG `state` and `increment` as unsigned 64-bit decimal strings. `increment` controls the PCG stream.

### rnd.set_state(`id`, `state`, `increment`)

Restores raw PCG `state` and `increment` from unsigned 64-bit decimal strings.

## Random Values

### rnd.number(`id`)

Returns a 32-bit unsigned integer as a Lua number. Values can be in `[0, 4294967295]`.

### rnd.numbers(`id`, `count`)

Returns a table of 32-bit unsigned integers. `count` must be a non-negative integer. `rnd.numbers(id, 0)` returns an empty table.

### rnd.bound(`id`, `upper_bound`)

Returns a 32-bit unsigned integer in `[0, upper_bound)`. `upper_bound` must be greater than `0`.

### rnd.range(`id`, `min`, `max`)

Returns a 32-bit unsigned integer in `[min, max]`. Only unsigned 32-bit integer bounds are supported.

### rnd.ranges(`id`, `count`, `min`, `max`)

Returns a table of 32-bit unsigned integers in `[min, max]`.

### rnd.double(`id`)

Returns a floating point number in `[0, 1)`.

### rnd.doubles(`id`, `count`)

Returns a table of floating point numbers in `[0, 1)`. `count` must be a non-negative integer. `rnd.doubles(id, 0)` returns an empty table.

### rnd.double_range(`id`, `min`, `max`)

Returns a floating point number in `[min, max)`. Not as fast as `rnd.double(id)`.

### rnd.boolean(`id`)

Returns `true` or `false` with equal probability.

### rnd.chance(`id`, `probability`)

Returns `true` according to the given probability. Values less than or equal to `0` always return `false`; values greater than or equal to `1` always return `true`.

## Dice And Tables

### rnd.dice(`id`, `roll`, `type`)

DnD style dice roller.

**PARAMETERS**

* `id` (number) - RNG context id.
* `roll` (number) - Roll amount.
* `type` (number) - Dice type.

Dice constants:

* `rnd.d4` - D4: four-sided die.
* `rnd.d6` - D6: six-sided die.
* `rnd.d8` - D8: eight-sided die.
* `rnd.d10` - D10: ten-sided die, returns 0-9.
* `rnd.d12` - D12: twelve-sided die.
* `rnd.d20` - D20: twenty-sided die.
* `rnd.d100` - D%: percentile die, returns 0-90 in tens.

**RETURN**

* `result` (table) - Dice results.
* `total` (number) - Total amount of dice results.

```lua
local rng = rnd.new(123)
local result, total = rnd.dice(rng, 2, rnd.d10)
```

### rnd.toss(`id`)

Tosses a coin. Returns `0` or `1`.

### rnd.roll(`id`)

Rolls a six-sided die. Returns a number in `[1, 6]`.

### rnd.card(`id`)

Picks a standard deck card index. Returns a number in `[1, 52]`.

### rnd.card2(`id`)

Picks a standard deck suit and rank as two return values. Suit is in `[1, 4]`; card is in `[1, 13]`.

### rnd.shuffle(`id`, `array`)

Shuffles the array part of a table in place using Fisher-Yates and returns the same table.

```lua
local rng = rnd.new(123)
local items = { "a", "b", "c", "d" }
rnd.shuffle(rng, items)
```

### rnd.check(`id`)

Debug/sample method that prints PCG sample output and consumes the context RNG state.

## Release Notes

### 1.3.0

- **Breaking change:** removed global RNG calls. Every random operation now requires an RNG context id.
- Added context lifecycle APIs: `rnd.new`, `rnd.new64`, `rnd.reset`, `rnd.reset64`, `rnd.delete`, and `rnd.clear_contexts`.
- Added exact raw state APIs: `rnd.state(id)` and `rnd.set_state(id, state, increment)`.
- Added exact unsigned 64-bit decimal string support for context seeds and raw state save/restore.
- Added utility APIs: `rnd.bound(id, upper_bound)`, `rnd.card(id)`, and `rnd.card2(id)`.


### 1.2.8

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

