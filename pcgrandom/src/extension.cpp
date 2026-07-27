
#define LIB_NAME "pcgrandom"
#define MODULE_NAME "rnd"

#include <dmsdk/sdk.h>
#include <pcgrandom.h>

static const luaL_reg Module_methods[] = {
    { "new", rnd::new_context },         //
    { "new64", rnd::new_context64 },     //
    { "reset", rnd::reset_context },     //
    { "reset64", rnd::reset_context64 }, //
    { "delete", rnd::delete_context },   //
    { "clear_contexts", rnd::clear_contexts },
    { "state", rnd::state },               //
    { "set_state", rnd::set_state },       //
    { "double", rnd::double_num },         //
    { "double_range", rnd::double_range }, //
    { "doubles", rnd::doubles },           //
    { "roll", rnd::roll },                 //
    { "toss", rnd::toss },                 //
    { "boolean", rnd::rnd_boolean },       //
    { "chance", rnd::chance },             //
    { "bound", rnd::bound },               //
    { "range", rnd::range },
    { "ranges", rnd::ranges },
    { "number", rnd::number },   //
    { "numbers", rnd::numbers }, //
    { "card", rnd::card },       //
    { "card2", rnd::card2 },     //
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

#define SETCONSTANT(field, value) \
    lua_pushnumber(L, (lua_Number)value); \
    lua_setfield(L, -2, field);

    SETCONSTANT("d4", rnd::DiceType::d4);
    SETCONSTANT("d6", rnd::DiceType::d6);
    SETCONSTANT("d8", rnd::DiceType::d8);
    SETCONSTANT("d10", rnd::DiceType::d10);
    SETCONSTANT("d12", rnd::DiceType::d12);
    SETCONSTANT("d20", rnd::DiceType::d20);
    SETCONSTANT("d100", rnd::DiceType::d100);
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
