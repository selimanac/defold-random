
#define LIB_NAME "pcgrandom"
#define MODULE_NAME "rnd"

#include <dmsdk/sdk.h>
#include <pcgrandom.h>

static const luaL_reg Module_methods[] = {
    { "seed", rnd::seedgen },              //
    { "seed32", rnd::seed32 },             //
    { "double", rnd::double_num },         //
    { "double_range", rnd::double_range }, //
    { "doubles", rnd::doubles },           //
    { "roll", rnd::roll },                 //
    { "toss", rnd::toss },                 //
    { "boolean", rnd::rnd_boolean },       //
    { "chance", rnd::chance },             //
    { "range", rnd::range },
    { "ranges", rnd::ranges },
    { "number", rnd::number },   //
    { "numbers", rnd::numbers }, //
    { "check", rnd::check },     //
    { "dice", rnd::dice },       //
    { "shuffle", rnd::shuffle }, //
    { 0, 0 }                     //
};

static void LuaInit(lua_State* L)
{
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

#define SETCONSTANT(name) \
    lua_pushnumber(L, (lua_Number)name); \
    lua_setfield(L, -2, #name);

    SETCONSTANT(rnd::DiceType::d4);
    SETCONSTANT(rnd::DiceType::d6);
    SETCONSTANT(rnd::DiceType::d8);
    SETCONSTANT(rnd::DiceType::d10);
    SETCONSTANT(rnd::DiceType::d12);
    SETCONSTANT(rnd::DiceType::d20);
    SETCONSTANT(rnd::DiceType::d100);
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
    rnd::entropy_seed();

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
