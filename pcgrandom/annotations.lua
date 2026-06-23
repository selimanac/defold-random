---PCG Random Number Generator Native Extension for the Defold Game Engine.
---@class rnd
rnd = {
    d4 = 4,     -- D4: four-sided die.
    d6 = 6,     -- D6: six-sided die.
    d8 = 8,     -- D8: eight-sided die.
    d10 = 10,   -- D10: ten-sided die. Returns 0-9.
    d12 = 12,   -- D12: twelve-sided die.
    d20 = 20,   -- D20: twenty-sided die.
    d100 = 100  -- D%: percentile die. Returns 0-90 in tens.
}

---Seeds the random number generator.
---Call without arguments to reseed using entropy. For deterministic repeatability from Lua, use 32-bit unsigned integer seed values.
---@param init_state? number Starting state for the RNG.
---@param init_seq? number Output sequence for the RNG.
function rnd.seed(init_state, init_seq) end

---Seeds the random number generator with exact unsigned 32-bit values.
---@param init_state number Starting state for the RNG. Must be an unsigned 32-bit integer.
---@param init_seq number Output sequence for the RNG. Must be an unsigned 32-bit integer.
function rnd.seed32(init_state, init_seq) end

---Returns a 32-bit unsigned integer.
---@return number random_number 32-bit unsigned integer.
function rnd.number() end

---Returns a table of 32-bit unsigned integers.
---@param count number Number of values to generate. Must be a non-negative integer.
---@return number[] random_numbers 32-bit unsigned integers.
function rnd.numbers(count) end

---Returns an integer between min and max, inclusive.
---Only unsigned 32-bit integer bounds are supported.
---@param min number Minimum unsigned integer.
---@param max number Maximum unsigned integer.
---@return number random_number Integer in [min, max].
function rnd.range(min, max) end

---Returns a table of integers between min and max, inclusive.
---Only unsigned 32-bit integer bounds are supported.
---@param count number Number of values to generate. Must be a non-negative integer.
---@param min number Minimum unsigned integer.
---@param max number Maximum unsigned integer.
---@return number[] random_numbers Integers in [min, max].
function rnd.ranges(count, min, max) end

---Returns a floating point number in [0, 1).
---@return number random_number Floating point number in [0, 1).
function rnd.double() end

---Returns a table of floating point numbers in [0, 1).
---@param count number Number of values to generate. Must be a non-negative integer.
---@return number[] random_numbers Floating point numbers in [0, 1).
function rnd.doubles(count) end

---Returns a floating point number in [min, max).
---@param min number Minimum number.
---@param max number Maximum number.
---@return number random_number Floating point number in [min, max).
function rnd.double_range(min, max) end

---Returns true or false with equal probability.
---@return boolean result Random boolean value.
function rnd.boolean() end

---Returns true according to the given probability.
---@param probability number Probability from 0.0 to 1.0. Values <= 0 are always false and values >= 1 are always true.
---@return boolean success True if the probability check succeeds.
function rnd.chance(probability) end

---Tosses a coin.
---@return number result 0 or 1. 0 = H, 1 = T.
function rnd.toss() end

---Rolls a six-sided die.
---@return number result Integer in [1, 6].
function rnd.roll() end

---Rolls dice using one of the exported dice constants.
---d10 returns 0-9, d100 returns 0-90 in tens, and the other dice return 1 through the dice type.
---@param roll number Roll count. Must be greater than 0.
---@param type number Dice type. Use rnd.d4, rnd.d6, rnd.d8, rnd.d10, rnd.d12, rnd.d20, or rnd.d100.
---@return number[] results Dice results.
---@return number total Total amount of dice results.
function rnd.dice(roll, type) end

---Shuffles the array part of a table in place using Fisher-Yates.
---@generic T
---@param array T[] Table to shuffle.
---@return T[] array The same table, shuffled in place.
function rnd.shuffle(array) end

---Runs the PCG sample output check and prints results. This is a debug/sample method and consumes RNG state.
function rnd.check() end
