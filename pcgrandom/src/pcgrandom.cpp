
#define LIB_NAME "pcgrandom"
#define MODULE_NAME "rnd"

#include <dmsdk/dlib/log.h>
#include <dmsdk/sdk.h>
#include <entropy.h>
#include <limits.h>
#include <math.h>
#include <pcg_basic.h>

// #define __STDC_FORMAT_MACROS
// #include <inttypes.h>

// #include <time.h>

static pcg32_random_t rng;

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

static void entropy_seed()
{
    uint64_t seeds[2];
    bool     used_entropy = entropy_getbytes((void*)seeds, sizeof(seeds));
    if (!used_entropy)
    {
        fallback_entropy_getbytes((void*)seeds, sizeof(seeds));
    }

    pcg32_srandom_r(&rng, seeds[0], seeds[1]);
    dmLogInfo("Seeded %s using %s", MODULE_NAME, used_entropy ? "OS entropy" : "fallback entropy");
}

static double random_double()
{
    return ldexp(pcg32_random_r(&rng), -32);
}

static bool check_uint32(lua_State* L, int index, const char* name, uint32_t* out)
{
    double value = luaL_checknumber(L, index);
    if (value < 0 || value > UINT32_MAX || floor(value) != value)
    {
        dmLogError("%s(%f) must be an unsigned 32-bit integer", name, value);
        return false;
    }

    *out = (uint32_t)value;
    return true;
}

static bool check_count(lua_State* L, int index, const char* name, int* out)
{
    double value = luaL_checknumber(L, index);
    if (value < 0 || value > INT_MAX || floor(value) != value)
    {
        dmLogError("%s(%f) must be a non-negative integer", name, value);
        return false;
    }

    *out = (int)value;
    return true;
}

static uint32_t random_range_uint32(uint32_t min, uint32_t max)
{
    uint64_t span = (uint64_t)max - (uint64_t)min + 1;
    return span == 4294967296ULL ? pcg32_random_r(&rng) : pcg32_boundedrand_r(&rng, (uint32_t)span) + min;
}

static int double_range(lua_State* L)
{
    double dmin = luaL_checknumber(L, 1);
    double dmax = luaL_checknumber(L, 2);

    if (dmin == dmax)
    {
        DM_LUA_STACK_CHECK(L, 1);
        lua_pushnumber(L, dmin);
        return 1;
    }

    if (dmin > dmax)
    {
        DM_LUA_STACK_CHECK(L, 0);
        dmLogError("rnd.double_range: MAX(%f) must be bigger than MIN(%f)", dmax, dmin);
        return 0;
    }

    DM_LUA_STACK_CHECK(L, 1);
    double d = random_double() * (dmax - dmin) + dmin;

    lua_pushnumber(L, d);
    return 1;
}

static int double_num(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    double d = random_double();

    lua_pushnumber(L, d);
    return 1;
}

static int roll(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    uint32_t num = pcg32_boundedrand_r(&rng, 6) + 1;
    lua_pushinteger(L, num);
    return 1;
}

static int dice(lua_State* L)
{
    int roll_count = luaL_checkint(L, 1);
    if (roll_count <= 0)
    {
        DM_LUA_STACK_CHECK(L, 0);
        dmLogError("Roll must be bigger then 0");
        return 0;
    }

    DiceType type = (DiceType)luaL_checkint(L, 2);
    if (type != d4 && type != d6 && type != d8 && type != d10 && type != d12 && type != d20 && type != d100)
    {
        DM_LUA_STACK_CHECK(L, 0);
        dmLogError("Invalid dice type: %d", type);
        return 0;
    }

    DM_LUA_STACK_CHECK(L, 2);
    lua_createtable(L, roll_count, 0);
    int newTable = lua_gettop(L);

    int total = 0;

    for (int i = 0; i < roll_count; ++i)
    {
        uint32_t num = 0;
        if (type == d100)
        {
            num = (int)pcg32_boundedrand_r(&rng, 10) * 10;
        }
        else if (type == d10)
        {
            num = (int)pcg32_boundedrand_r(&rng, 10);
        }
        else
        {
            num = (int)pcg32_boundedrand_r(&rng, type) + 1;
        }

        total = total + num;

        lua_pushinteger(L, num);
        lua_rawseti(L, newTable, i + 1);
    }

    lua_pushinteger(L, total);
    return 2;
}

static int toss(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    uint32_t num = pcg32_boundedrand_r(&rng, 2) ? 0 : 1;
    lua_pushinteger(L, num);
    return 1;
}

static int range(lua_State* L)
{
    uint32_t min = 0;
    if (!check_uint32(L, 1, "rnd.range: MIN", &min))
    {
        DM_LUA_STACK_CHECK(L, 0);
        return 0;
    }

    uint32_t max = 0;
    if (!check_uint32(L, 2, "rnd.range: MAX", &max))
    {
        DM_LUA_STACK_CHECK(L, 0);
        return 0;
    }

    if (min == max)
    {
        DM_LUA_STACK_CHECK(L, 1);
        lua_pushinteger(L, min);
        return 1;
    }

    if (min > max)
    {
        DM_LUA_STACK_CHECK(L, 0);
        dmLogError("rnd.range: MAX(%u) must be bigger than MIN(%u)", max, min);
        return 0;
    }

    DM_LUA_STACK_CHECK(L, 1);
    uint32_t num = random_range_uint32(min, max);
    lua_pushinteger(L, num);
    return 1;
}

static int numbers(lua_State* L)
{
    int count = 0;
    if (!check_count(L, 1, "rnd.numbers: count", &count))
    {
        DM_LUA_STACK_CHECK(L, 0);
        return 0;
    }

    DM_LUA_STACK_CHECK(L, 1);
    lua_createtable(L, count, 0);
    int table = lua_gettop(L);

    for (int i = 0; i < count; ++i)
    {
        lua_pushinteger(L, pcg32_random_r(&rng));
        lua_rawseti(L, table, i + 1);
    }

    return 1;
}

static int ranges(lua_State* L)
{
    int count = 0;
    if (!check_count(L, 1, "rnd.ranges: count", &count))
    {
        DM_LUA_STACK_CHECK(L, 0);
        return 0;
    }

    uint32_t min = 0;
    if (!check_uint32(L, 2, "rnd.ranges: MIN", &min))
    {
        DM_LUA_STACK_CHECK(L, 0);
        return 0;
    }

    uint32_t max = 0;
    if (!check_uint32(L, 3, "rnd.ranges: MAX", &max))
    {
        DM_LUA_STACK_CHECK(L, 0);
        return 0;
    }

    if (min > max)
    {
        DM_LUA_STACK_CHECK(L, 0);
        dmLogError("rnd.ranges: MAX(%u) must be bigger than MIN(%u)", max, min);
        return 0;
    }

    DM_LUA_STACK_CHECK(L, 1);
    lua_createtable(L, count, 0);
    int table = lua_gettop(L);

    for (int i = 0; i < count; ++i)
    {
        lua_pushinteger(L, min == max ? min : random_range_uint32(min, max));
        lua_rawseti(L, table, i + 1);
    }

    return 1;
}

static int doubles(lua_State* L)
{
    int count = 0;
    if (!check_count(L, 1, "rnd.doubles: count", &count))
    {
        DM_LUA_STACK_CHECK(L, 0);
        return 0;
    }

    DM_LUA_STACK_CHECK(L, 1);
    lua_createtable(L, count, 0);
    int table = lua_gettop(L);

    for (int i = 0; i < count; ++i)
    {
        lua_pushnumber(L, random_double());
        lua_rawseti(L, table, i + 1);
    }

    return 1;
}

static int seedgen(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    if (lua_gettop(L) == 0)
    {
        entropy_seed();
    }
    else
    {
        uint64_t seedinitstate = luaL_checknumber(L, 1);
        uint64_t seedinitseq = luaL_checknumber(L, 2);
        // printf("seedinitstate: %" PRIu64 "\n", seedinitstate);
        // printf("seedinitseq: %" PRIu64 "\n", seedinitseq);
        pcg32_srandom_r(&rng, seedinitstate, seedinitseq);
    }

    return 0;
}

static int seed32(lua_State* L)
{
    uint32_t seedinitstate = 0;
    if (!check_uint32(L, 1, "rnd.seed32: init_state", &seedinitstate))
    {
        DM_LUA_STACK_CHECK(L, 0);
        return 0;
    }

    uint32_t seedinitseq = 0;
    if (!check_uint32(L, 2, "rnd.seed32: init_seq", &seedinitseq))
    {
        DM_LUA_STACK_CHECK(L, 0);
        return 0;
    }

    DM_LUA_STACK_CHECK(L, 0);
    pcg32_srandom_r(&rng, seedinitstate, seedinitseq);
    return 0;
}

static int number(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    uint32_t num = pcg32_random_r(&rng);
    lua_pushinteger(L, num);
    return 1;
}

static int rnd_boolean(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 1);
    lua_pushboolean(L, pcg32_boundedrand_r(&rng, 2) != 0);
    return 1;
}

static int chance(lua_State* L)
{
    double probability = luaL_checknumber(L, 1);

    if (probability != probability)
    {
        DM_LUA_STACK_CHECK(L, 0);
        dmLogError("rnd.chance: probability must be a number");
        return 0;
    }

    DM_LUA_STACK_CHECK(L, 1);
    if (probability <= 0.0)
    {
        lua_pushboolean(L, 0);
    }
    else if (probability >= 1.0)
    {
        lua_pushboolean(L, 1);
    }
    else
    {
        lua_pushboolean(L, random_double() < probability);
    }

    return 1;
}

static int shuffle(lua_State* L)
{
    if (!lua_istable(L, 1))
    {
        DM_LUA_STACK_CHECK(L, 0);
        dmLogError("rnd.shuffle: array must be a table");
        return 0;
    }

    size_t count = lua_objlen(L, 1);
    if (count > UINT32_MAX)
    {
        DM_LUA_STACK_CHECK(L, 0);
        dmLogError("rnd.shuffle: array length(%zu) must fit in an unsigned 32-bit integer", count);
        return 0;
    }

    DM_LUA_STACK_CHECK(L, 1);
    for (size_t i = count; i > 1; --i)
    {
        uint32_t j = pcg32_boundedrand_r(&rng, (uint32_t)i) + 1;

        lua_rawgeti(L, 1, i);
        lua_rawgeti(L, 1, j);
        lua_rawseti(L, 1, i);
        lua_rawseti(L, 1, j);
    }

    lua_pushvalue(L, 1);
    return 1;
}

static int check(lua_State* L)
{
    DM_LUA_STACK_CHECK(L, 0);
    int      rounds = 1;
    int      round, i;

    uint32_t t = pcg32_random_r(&rng);
    printf("uint: %u", t);
    printf("\n");

    printf(
    "pcg32_random_r:\n"
    "      -  result:      32-bit unsigned int (uint32_t)\n"
    "      -  period:      2^64   (* 2^63 streams)\n"
    "      -  state type:  pcg32_random_t (%zu bytes)\n"
    "      -  output func: XSH-RR\n"
    "\n",
    sizeof(pcg32_random_t));

    for (round = 1; round <= rounds; ++round)
    {
        printf("Round %d:\n", round);
        /* Make some 32-bit numbers */
        printf("  32bit:");
        for (i = 0; i < 6; ++i)
            printf(" 0x%08x", pcg32_random_r(&rng));
        printf("\n");

        /* Toss some coins */
        printf("  Coins: ");
        for (i = 0; i < 65; ++i)
            printf("%c", pcg32_boundedrand_r(&rng, 2) ? 'H' : 'T');
        printf("\n");

        /* Roll some dice */
        printf("  Rolls:");
        for (i = 0; i < 33; ++i)
        {
            printf(" %d", (int)pcg32_boundedrand_r(&rng, 6) + 1);
        }
        printf("\n");

        /* Deal some cards */
        enum
        {
            SUITS = 4,
            NUMBERS = 13,
            CARDS = 52
        };
        char cards[CARDS];

        for (i = 0; i < CARDS; ++i)
            cards[i] = i;

        for (i = CARDS; i > 1; --i)
        {
            int  chosen = pcg32_boundedrand_r(&rng, i);
            char card = cards[chosen];
            cards[chosen] = cards[i - 1];
            cards[i - 1] = card;
        }

        printf("  Cards:");
        static const char number[] = { 'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T', 'J', 'Q', 'K' };
        static const char suit[] = { 'h', 'c', 'd', 's' };
        for (i = 0; i < CARDS; ++i)
        {
            printf(" %c%c", number[cards[i] / SUITS], suit[cards[i] % SUITS]);
            if ((i + 1) % 22 == 0)
                printf("\n\t");
        }
        printf("\n");
    }

    return 0;
}

static const luaL_reg Module_methods[] = {
    { "seed", seedgen },              //
    { "seed32", seed32 },             //
    { "double", double_num },         //
    { "double_range", double_range }, //
    { "doubles", doubles },           //
    { "roll", roll },                 //
    { "toss", toss },                 //
    { "boolean", rnd_boolean },       //
    { "chance", chance },             //
    { "range", range },
    { "ranges", ranges },
    { "number", number },   //
    { "numbers", numbers }, //
    { "check", check },     //
    { "dice", dice },       //
    { "shuffle", shuffle }, //
    { 0, 0 }                //
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name) \
    lua_pushnumber(L, (lua_Number)name); \
    lua_setfield(L, -2, #name);

    SETCONSTANT(d4);
    SETCONSTANT(d6);
    SETCONSTANT(d8);
    SETCONSTANT(d10);
    SETCONSTANT(d12);
    SETCONSTANT(d20);
    SETCONSTANT(d100);
#undef SETCONSTANT

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result app_init_pcgrandom(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result init_pcgrandom(dmExtension::Params* params)
{
    LuaInit(params->m_L);
    dmLogInfo("Registered %s Extension\n", MODULE_NAME);
    entropy_seed();

    return dmExtension::RESULT_OK;
}

dmExtension::Result app_final_pcgrandom(dmExtension::AppParams* params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result final_pcgrandom(dmExtension::Params* params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(pcgrandom, LIB_NAME, app_init_pcgrandom, app_final_pcgrandom, init_pcgrandom, 0, 0, final_pcgrandom)
