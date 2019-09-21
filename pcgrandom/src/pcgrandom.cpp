
#define LIB_NAME "pcgrandom"
#define MODULE_NAME "rnd"

#include <dmsdk/sdk.h>
#include <dmsdk/dlib/log.h>
#include "pcg_basic.h"
#include <math.h>
#include "entropy.h"

//#include <time.h>

static pcg32_random_t rng;
static uint32_t num;
static uint32_t min;
static uint32_t max;
static uint32_t seed;
static uint64_t seeds[2];
static double d;

static void entropy_seed()
{
    // Entropy seed with Time based fallback:
    entropy_getbytes((void *)seeds, sizeof(seeds));
    pcg32_srandom_r(&rng, seeds[0], seeds[1]);

    // ---------------

    // Time based seed:
    //pcg32_srandom_r(&rng, time(NULL) ^ (intptr_t)&printf, (intptr_t)&rng);
}

static int double_num(lua_State *L)
{
    int top = lua_gettop(L);
    d = ldexp(pcg32_random_r(&rng), -32);
    lua_pushnumber(L, d);
    assert(top + 1 == lua_gettop(L));
    return 1;
}

static int roll(lua_State *L)
{
    int top = lua_gettop(L);
    num = (int)pcg32_boundedrand_r(&rng, 6) + 1;
    lua_pushinteger(L, num);
    assert(top + 1 == lua_gettop(L));
    return 1;
}

static int toss(lua_State *L)
{
    int top = lua_gettop(L);
    num = pcg32_boundedrand_r(&rng, 2) ? 0 : 1;
    lua_pushinteger(L, num);
    assert(top + 1 == lua_gettop(L));
    return 1;
}

static int range(lua_State *L)
{
    int top = lua_gettop(L);
    min = luaL_checknumber(L, 1);
    max = luaL_checknumber(L, 2);
    max++;
    num = pcg32_boundedrand_r(&rng, (max - min)) + min;
    lua_pushnumber(L, num);
    assert(top + 1 == lua_gettop(L));
    return 1;
}

static int seedgen(lua_State *L)
{
    seed = luaL_optnumber(L, 1, 0);
    if (seed == 0)
    {
        entropy_seed();
    }
    else
    {
        seed = luaL_checknumber(L, 1);
        pcg32_srandom_r(&rng, seed, sizeof(seed));
    }

    return 0;
}

static int number(lua_State *L)
{
    int top = lua_gettop(L);
    num = pcg32_random_r(&rng);
    lua_pushnumber(L, num);
    assert(top + 1 == lua_gettop(L));
    return 1;
}

static int check(lua_State *L)
{
    int rounds = 1;
    int round, i;

    uint32_t t = pcg32_random_r(&rng);
    printf("uint: %u", t);
    printf("\n");

    printf("pcg32_random_r:\n"
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
            int chosen = pcg32_boundedrand_r(&rng, i);
            char card = cards[chosen];
            cards[chosen] = cards[i - 1];
            cards[i - 1] = card;
        }

        printf("  Cards:");
        static const char number[] = {'A', '2', '3', '4', '5', '6', '7',
                                      '8', '9', 'T', 'J', 'Q', 'K'};
        static const char suit[] = {'h', 'c', 'd', 's'};
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

static const luaL_reg Module_methods[] =
    {

        {"seed", seedgen},
        {"double", double_num},
        {"roll", roll},
        {"toss", toss},
        {"range", range},
        {"number", number},
        {"check", check},
        {0, 0}};

static void LuaInit(lua_State *L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result app_init_pcgrandom(dmExtension::AppParams *params)
{

    return dmExtension::RESULT_OK;
}

dmExtension::Result init_pcgrandom(dmExtension::Params *params)
{
    LuaInit(params->m_L);
    printf("Registered %s Extension\n", MODULE_NAME);

    entropy_seed();

    return dmExtension::RESULT_OK;
}

dmExtension::Result app_final_pcgrandom(dmExtension::AppParams *params)
{
    return dmExtension::RESULT_OK;
}

dmExtension::Result final_pcgrandom(dmExtension::Params *params)
{
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(pcgrandom, LIB_NAME, app_init_pcgrandom, app_final_pcgrandom, init_pcgrandom, 0, 0, final_pcgrandom)
