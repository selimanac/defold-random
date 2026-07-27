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

---Creates an isolated RNG context. Calling without arguments uses entropy. Calling with one seed uses init_seq = 1.
---@param init_state? number Starting state for the RNG. Must be an unsigned 32-bit integer when provided.
---@param init_seq? number Output sequence for the RNG. Must be an unsigned 32-bit integer when provided.
---@return number id RNG context id.
function rnd.new(init_state, init_seq) end

---Creates an isolated RNG context with exact unsigned 64-bit decimal string seeds.
---@param init_state string Starting state for the RNG.
---@param init_seq string Output sequence for the RNG.
---@return number id RNG context id.
function rnd.new64(init_state, init_seq) end

---Resets an RNG context. Calling with only id uses entropy. Calling with one seed uses init_seq = 1.
---@param id number RNG context id.
---@param init_state? number Starting state for the RNG. Must be an unsigned 32-bit integer when provided.
---@param init_seq? number Output sequence for the RNG. Must be an unsigned 32-bit integer when provided.
function rnd.reset(id, init_state, init_seq) end

---Resets an RNG context with exact unsigned 64-bit decimal string seeds.
---@param id number RNG context id.
---@param init_state string Starting state for the RNG.
---@param init_seq string Output sequence for the RNG.
function rnd.reset64(id, init_state, init_seq) end

---Deletes an RNG context id.
---@param id number RNG context id.
function rnd.delete(id) end

---Deletes all RNG contexts.
function rnd.clear_contexts() end

---Returns raw PCG state and increment as unsigned 64-bit decimal strings.
---@param id number RNG context id.
---@return string state Raw PCG state.
---@return string increment Raw PCG increment. Controls the PCG stream.
function rnd.state(id) end

---Restores raw PCG state and increment from unsigned 64-bit decimal strings.
---@param id number RNG context id.
---@param state string Raw PCG state.
---@param increment string Raw PCG increment. Controls the PCG stream.
function rnd.set_state(id, state, increment) end

---Returns a 32-bit unsigned integer.
---@param id number RNG context id.
---@return number random_number 32-bit unsigned integer.
function rnd.number(id) end

---Returns a table of 32-bit unsigned integers.
---@param id number RNG context id.
---@param count number Number of values to generate. Must be a non-negative integer.
---@return number[] random_numbers 32-bit unsigned integers.
function rnd.numbers(id, count) end

---Returns an integer between 0 and upper_bound, exclusive.
---@param id number RNG context id.
---@param upper_bound number Exclusive upper bound. Must be an unsigned 32-bit integer greater than 0.
---@return number random_number Integer in [0, upper_bound).
function rnd.bound(id, upper_bound) end

---Returns an integer between min and max, inclusive.
---Only unsigned 32-bit integer bounds are supported.
---@param id number RNG context id.
---@param min number Minimum unsigned integer.
---@param max number Maximum unsigned integer.
---@return number random_number Integer in [min, max].
function rnd.range(id, min, max) end

---Returns a table of integers between min and max, inclusive.
---Only unsigned 32-bit integer bounds are supported.
---@param id number RNG context id.
---@param count number Number of values to generate. Must be a non-negative integer.
---@param min number Minimum unsigned integer.
---@param max number Maximum unsigned integer.
---@return number[] random_numbers Integers in [min, max].
function rnd.ranges(id, count, min, max) end

---Returns a floating point number in [0, 1).
---@param id number RNG context id.
---@return number random_number Floating point number in [0, 1).
function rnd.double(id) end

---Returns a table of floating point numbers in [0, 1).
---@param id number RNG context id.
---@param count number Number of values to generate. Must be a non-negative integer.
---@return number[] random_numbers Floating point numbers in [0, 1).
function rnd.doubles(id, count) end

---Returns a floating point number in [min, max).
---@param id number RNG context id.
---@param min number Minimum number.
---@param max number Maximum number.
---@return number random_number Floating point number in [min, max).
function rnd.double_range(id, min, max) end

---Returns true or false with equal probability.
---@param id number RNG context id.
---@return boolean result Random boolean value.
function rnd.boolean(id) end

---Returns true according to the given probability.
---@param id number RNG context id.
---@param probability number Probability from 0.0 to 1.0. Values <= 0 are always false and values >= 1 are always true.
---@return boolean success True if the probability check succeeds.
function rnd.chance(id, probability) end

---Tosses a coin.
---@param id number RNG context id.
---@return number result 0 or 1. 0 = H, 1 = T.
function rnd.toss(id) end

---Rolls a six-sided die.
---@param id number RNG context id.
---@return number result Integer in [1, 6].
function rnd.roll(id) end

---Picks a standard deck card index.
---@param id number RNG context id.
---@return number card Card index in [1, 52].
function rnd.card(id) end

---Picks a standard deck suit and rank.
---@param id number RNG context id.
---@return number suit Suit in [1, 4].
---@return number card Card rank in [1, 13].
function rnd.card2(id) end

---Rolls dice using one of the exported dice constants.
---d10 returns 0-9, d100 returns 0-90 in tens, and the other dice return 1 through the dice type.
---@param id number RNG context id.
---@param roll number Roll count. Must be greater than 0.
---@param type number Dice type. Use rnd.d4, rnd.d6, rnd.d8, rnd.d10, rnd.d12, rnd.d20, or rnd.d100.
---@return number[] results Dice results.
---@return number total Total amount of dice results.
function rnd.dice(id, roll, type) end

---Shuffles the array part of a table in place using Fisher-Yates.
---@generic T
---@param id number RNG context id.
---@param array T[] Table to shuffle.
---@return T[] array The same table, shuffled in place.
function rnd.shuffle(id, array) end

---Runs the PCG sample output check and prints results. This is a debug/sample method and consumes RNG state.
---@param id number RNG context id.
function rnd.check(id) end
