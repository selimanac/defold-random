#pragma once

#include <dmsdk/sdk.h>
#include <entropy.h>
#include <limits.h>
#include <math.h>
#include <pcg_basic.h>

namespace rnd
{

    enum DiceType
    {
        d4 = 4,
        d6 = 6,
        d8 = 8,
        d10 = 10,
        d12 = 12,
        d20 = 20,
        d100 = 100
    };

    void     entropy_seed();
    double   random_double();
    bool     check_uint32(lua_State* L, int index, const char* name, uint32_t* out);
    bool     check_count(lua_State* L, int index, const char* name, int* out);
    uint32_t random_range_uint32(uint32_t min, uint32_t max);
    int      double_range(lua_State* L);
    int      double_num(lua_State* L);
    int      roll(lua_State* L);
    int      dice(lua_State* L);
    int      toss(lua_State* L);
    int      range(lua_State* L);
    int      numbers(lua_State* L);
    int      ranges(lua_State* L);
    int      doubles(lua_State* L);
    int      seedgen(lua_State* L);
    int      seed32(lua_State* L);
    int      number(lua_State* L);
    int      rnd_boolean(lua_State* L);
    int      chance(lua_State* L);
    int      shuffle(lua_State* L);
    int      check(lua_State* L);
} // namespace rnd