#pragma once

#define __STDC_FORMAT_MACROS
#define __STDC_LIMIT_MACROS

#include <dmsdk/sdk.h>
#include <entropy.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <pcg_basic.h>
#include <stdint.h>
#include <stdio.h>

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

    bool     check_uint32(lua_State* L, int arg_index, const char* name, uint32_t* out);
    bool     check_count(lua_State* L, int arg_index, const char* name, int* out);
    int      new_context(lua_State* L);
    int      new_context64(lua_State* L);
    int      reset_context(lua_State* L);
    int      reset_context64(lua_State* L);
    int      delete_context(lua_State* L);
    int      clear_contexts(lua_State* L);
    int      state(lua_State* L);
    int      set_state(lua_State* L);
    int      double_range(lua_State* L);
    int      double_num(lua_State* L);
    int      roll(lua_State* L);
    int      dice(lua_State* L);
    int      toss(lua_State* L);
    int      bound(lua_State* L);
    int      range(lua_State* L);
    int      numbers(lua_State* L);
    int      ranges(lua_State* L);
    int      doubles(lua_State* L);
    int      number(lua_State* L);
    int      card(lua_State* L);
    int      card2(lua_State* L);
    int      rnd_boolean(lua_State* L);
    int      chance(lua_State* L);
    int      shuffle(lua_State* L);
    int      check(lua_State* L);
} // namespace rnd
