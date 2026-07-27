#include <pcgrandom.h>

#if defined(__APPLE__)
#include <Security/Security.h>
#elif defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <bcrypt.h>
#elif defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#elif defined(__ANDROID__) || defined(__linux__)
#include <fcntl.h>
#include <unistd.h>
#endif

namespace rnd
{
    static const uint8_t  MAX_CONTEXTS = 255;
    static const uint64_t MAX_CONTEXT_ID = (uint64_t)UINT32_MAX * (uint64_t)MAX_CONTEXTS + (uint64_t)MAX_CONTEXTS - 1ULL;
    static const uint64_t UINT64_LIMIT_DIV_10 = UINT64_MAX / 10ULL;
    static const uint64_t UINT64_LIMIT_MOD_10 = UINT64_MAX % 10ULL;

    struct RngContext
    {
        pcg32_random_t rng;
        uint32_t       generation;
        bool           active;
    };

    static RngContext contexts[MAX_CONTEXTS];

    static bool       read_dev_urandom(void* dest, size_t size)
    {
#if defined(__ANDROID__) || defined(__linux__)
        int fd = open("/dev/urandom", O_RDONLY);
        if (fd < 0)
        {
            return false;
        }

        char*  output = (char*)dest;
        size_t remaining = size;
        while (remaining > 0)
        {
            ssize_t bytes_read = read(fd, output, remaining);
            if (bytes_read <= 0)
            {
                close(fd);
                return false;
            }

            output += bytes_read;
            remaining -= (size_t)bytes_read;
        }

        return close(fd) == 0;
#else
        (void)dest;
        (void)size;
        return false;
#endif
    }

    static bool system_entropy_getbytes(void* dest, size_t size, const char** source_name)
    {
#if defined(__APPLE__)
        *source_name = "SecRandomCopyBytes";
        return SecRandomCopyBytes(kSecRandomDefault, size, dest) == errSecSuccess;
#elif defined(_WIN32)
        *source_name = "BCryptGenRandom";
        if (size > ULONG_MAX)
        {
            return false;
        }
        return BCryptGenRandom(NULL, (PUCHAR)dest, (ULONG)size, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#elif defined(__EMSCRIPTEN__)
        *source_name = "Web Crypto";
        return EM_ASM_INT({
            var crypto_object = null;
            if (typeof globalThis !== 'undefined' && globalThis.crypto) {
                crypto_object = globalThis.crypto;
            } else if (typeof self !== 'undefined' && self.crypto) {
                crypto_object = self.crypto;
            }

            if (!crypto_object || !crypto_object.getRandomValues) {
                return 0;
            }

            crypto_object.getRandomValues(HEAPU8.subarray($0, $0 + $1));
            return 1; }, dest, size) != 0;
#elif defined(__ANDROID__) || defined(__linux__)
        *source_name = "/dev/urandom";
        return read_dev_urandom(dest, size);
#else
        *source_name = "time-based fallback";
        return false;
#endif
    }

    static void seed_from_entropy(pcg32_random_t* target)
    {
        uint64_t    seeds[2];
        const char* source_name = "time-based fallback";
        bool        used_entropy = system_entropy_getbytes((void*)seeds, sizeof(seeds), &source_name);
        if (!used_entropy)
        {
            fallback_entropy_getbytes((void*)seeds, sizeof(seeds));
            source_name = "time-based fallback";
        }

        pcg32_srandom_r(target, seeds[0], seeds[1]);
        dmLogInfo("Seeded rnd context using %s", source_name);
    }

    bool check_uint32(lua_State* L, int arg_index, const char* name, uint32_t* out)
    {
        double value = luaL_checknumber(L, arg_index);
        if (value < 0 || value > UINT32_MAX || floor(value) != value)
        {
            dmLogError("%s(%f) must be an unsigned 32-bit integer", name, value);
            return false;
        }

        *out = (uint32_t)value;
        return true;
    }

    bool check_count(lua_State* L, int arg_index, const char* name, int* out)
    {
        double value = luaL_checknumber(L, arg_index);
        if (value < 0 || value > INT_MAX || floor(value) != value)
        {
            dmLogError("%s(%f) must be a non-negative integer", name, value);
            return false;
        }

        *out = (int)value;
        return true;
    }

    static bool check_uint64_string(lua_State* L, int arg_index, const char* name, uint64_t* out)
    {
        if (lua_type(L, arg_index) != LUA_TSTRING)
        {
            dmLogError("%s must be an unsigned 64-bit decimal string", name);
            return false;
        }

        size_t      length = 0;
        const char* text = lua_tolstring(L, arg_index, &length);
        if (length == 0)
        {
            dmLogError("%s must not be empty", name);
            return false;
        }

        uint64_t value = 0;
        for (size_t i = 0; i < length; ++i)
        {
            char c = text[i];
            if (c < '0' || c > '9')
            {
                dmLogError("%s must be an unsigned 64-bit decimal string", name);
                return false;
            }

            uint64_t digit = (uint64_t)(c - '0');
            if (value > UINT64_LIMIT_DIV_10 || (value == UINT64_LIMIT_DIV_10 && digit > UINT64_LIMIT_MOD_10))
            {
                dmLogError("%s is larger than 18446744073709551615", name);
                return false;
            }

            value = value * 10ULL + digit;
        }

        *out = value;
        return true;
    }

    static void push_uint64_string(lua_State* L, uint64_t value)
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%" PRIu64, value);
        lua_pushstring(L, buffer);
    }

    static void push_uint32_number(lua_State* L, uint32_t value)
    {
        lua_pushnumber(L, (lua_Number)value);
    }

    static uint64_t encode_context_id(uint32_t slot_index, uint32_t generation)
    {
        return (uint64_t)generation * (uint64_t)MAX_CONTEXTS + (uint64_t)slot_index;
    }

    static bool check_context(lua_State* L, int arg_index, const char* name, pcg32_random_t** out)
    {
        double value = luaL_checknumber(L, arg_index);
        if (value < (double)MAX_CONTEXTS || value > (double)MAX_CONTEXT_ID || floor(value) != value)
        {
            dmLogError("%s must be a valid RNG context id", name);
            return false;
        }

        uint64_t    id = (uint64_t)value;
        uint32_t    slot_index = (uint32_t)(id % MAX_CONTEXTS);
        uint32_t    generation = (uint32_t)(id / MAX_CONTEXTS);

        RngContext* context = &contexts[slot_index];
        if (generation == 0 || !context->active || context->generation != generation)
        {
            dmLogError("%s is not an active RNG context id", name);
            return false;
        }

        *out = &context->rng;
        return true;
    }

    static bool seed_context_from_args(lua_State* L, int first_seed_arg_index, int seed_arg_count, const char* name, pcg32_random_t* target)
    {
        if (seed_arg_count == 0)
        {
            seed_from_entropy(target);
            return true;
        }

        if (seed_arg_count != 1 && seed_arg_count != 2)
        {
            dmLogError("%s: wrong number of arguments", name);
            return false;
        }

        uint32_t init_state = 0;
        if (!check_uint32(L, first_seed_arg_index, name, &init_state))
        {
            return false;
        }

        uint32_t init_seq = 1;
        if (seed_arg_count == 2 && !check_uint32(L, first_seed_arg_index + 1, name, &init_seq))
        {
            return false;
        }

        pcg32_srandom_r(target, init_state, init_seq);
        return true;
    }

    static uint32_t random_range_uint32(pcg32_random_t* target, uint32_t min, uint32_t max)
    {
        uint64_t span = (uint64_t)max - (uint64_t)min + 1ULL;
        return span == 4294967296ULL ? pcg32_random_r(target) : pcg32_boundedrand_r(target, (uint32_t)span) + min;
    }

    static double random_double(pcg32_random_t* target)
    {
        return ldexp(pcg32_random_r(target), -32);
    }

    static int create_context(lua_State* L, uint64_t init_state, uint64_t init_seq)
    {
        for (uint32_t i = 0; i < MAX_CONTEXTS; ++i)
        {
            RngContext* context = &contexts[i];
            if (context->active)
            {
                continue;
            }

            pcg32_srandom_r(&context->rng, init_state, init_seq);
            context->generation += 1;
            if (context->generation == 0)
            {
                context->generation = 1;
            }
            context->active = true;

            DM_LUA_STACK_CHECK(L, 1);
            lua_pushnumber(L, (lua_Number)encode_context_id(i, context->generation));
            return 1;
        }

        DM_LUA_STACK_CHECK(L, 0);
        dmLogError("no free RNG context slots");
        return 0;
    }

    int new_context(lua_State* L)
    {
        int top = lua_gettop(L);
        if (top > 2)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.new: wrong number of arguments");
            return 0;
        }

        pcg32_random_t seeded;
        if (!seed_context_from_args(L, 1, top, "rnd.new", &seeded))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        for (uint32_t i = 0; i < MAX_CONTEXTS; ++i)
        {
            RngContext* context = &contexts[i];
            if (context->active)
            {
                continue;
            }

            context->rng = seeded;
            context->generation += 1;
            if (context->generation == 0)
            {
                context->generation = 1;
            }
            context->active = true;

            DM_LUA_STACK_CHECK(L, 1);
            lua_pushnumber(L, (lua_Number)encode_context_id(i, context->generation));
            return 1;
        }

        DM_LUA_STACK_CHECK(L, 0);
        dmLogError("rnd.new: no free RNG context slots");
        return 0;
    }

    int new_context64(lua_State* L)
    {
        if (lua_gettop(L) != 2)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.new64: expected init_state and init_seq decimal strings");
            return 0;
        }

        uint64_t init_state = 0;
        uint64_t init_seq = 0;
        if (!check_uint64_string(L, 1, "rnd.new64: init_state", &init_state) ||
            !check_uint64_string(L, 2, "rnd.new64: init_seq", &init_seq))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        return create_context(L, init_state, init_seq);
    }

    int reset_context(lua_State* L)
    {
        int top = lua_gettop(L);
        if (top < 1 || top > 3)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.reset: wrong number of arguments");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.reset: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 0);
        seed_context_from_args(L, 2, top - 1, "rnd.reset", target);
        return 0;
    }

    int reset_context64(lua_State* L)
    {
        if (lua_gettop(L) != 3)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.reset64: expected id, init_state, and init_seq");
            return 0;
        }

        pcg32_random_t* target = 0;
        uint64_t        init_state = 0;
        uint64_t        init_seq = 0;
        if (!check_context(L, 1, "rnd.reset64: id", &target) ||
            !check_uint64_string(L, 2, "rnd.reset64: init_state", &init_state) ||
            !check_uint64_string(L, 3, "rnd.reset64: init_seq", &init_seq))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 0);
        pcg32_srandom_r(target, init_state, init_seq);
        return 0;
    }

    int delete_context(lua_State* L)
    {
        if (lua_gettop(L) != 1)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.delete: expected context id");
            return 0;
        }

        double value = luaL_checknumber(L, 1);
        if (value < (double)MAX_CONTEXTS || value > (double)MAX_CONTEXT_ID || floor(value) != value)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.delete: id must be a valid RNG context id");
            return 0;
        }

        uint64_t    id = (uint64_t)value;
        uint32_t    slot_index = (uint32_t)(id % MAX_CONTEXTS);
        uint32_t    generation = (uint32_t)(id / MAX_CONTEXTS);
        RngContext* context = &contexts[slot_index];

        if (generation == 0 || !context->active || context->generation != generation)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.delete: id is not an active RNG context id");
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 0);
        context->active = false;
        return 0;
    }

    int clear_contexts(lua_State* L)
    {
        if (lua_gettop(L) != 0)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.clear_contexts: expected no arguments");
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 0);
        for (uint32_t i = 0; i < MAX_CONTEXTS; ++i)
        {
            contexts[i].active = false;
        }
        return 0;
    }

    int state(lua_State* L)
    {
        if (lua_gettop(L) != 1)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.state: expected context id");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.state: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 2);
        push_uint64_string(L, target->state);
        push_uint64_string(L, target->inc);
        return 2;
    }

    int set_state(lua_State* L)
    {
        if (lua_gettop(L) != 3)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.set_state: expected id, state, and increment");
            return 0;
        }

        pcg32_random_t* target = 0;
        uint64_t        state_value = 0;
        uint64_t        increment_value = 0;
        if (!check_context(L, 1, "rnd.set_state: id", &target) ||
            !check_uint64_string(L, 2, "rnd.set_state: state", &state_value) ||
            !check_uint64_string(L, 3, "rnd.set_state: increment", &increment_value))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 0);
        target->state = state_value;
        target->inc = increment_value;
        return 0;
    }

    int double_range(lua_State* L)
    {
        if (lua_gettop(L) != 3)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.double_range: expected id, min, and max");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.double_range: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        double dmin = luaL_checknumber(L, 2);
        double dmax = luaL_checknumber(L, 3);
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
        lua_pushnumber(L, random_double(target) * (dmax - dmin) + dmin);
        return 1;
    }

    int double_num(lua_State* L)
    {
        if (lua_gettop(L) != 1)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.double: expected context id");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.double: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        lua_pushnumber(L, random_double(target));
        return 1;
    }

    int roll(lua_State* L)
    {
        if (lua_gettop(L) != 1)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.roll: expected context id");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.roll: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        lua_pushinteger(L, pcg32_boundedrand_r(target, 6) + 1);
        return 1;
    }

    int dice(lua_State* L)
    {
        if (lua_gettop(L) != 3)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.dice: expected id, roll, and type");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.dice: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        int roll_count = luaL_checkint(L, 2);
        if (roll_count <= 0)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("Roll must be bigger then 0");
            return 0;
        }

        DiceType type = (DiceType)luaL_checkint(L, 3);
        if (type != d4 && type != d6 && type != d8 && type != d10 && type != d12 && type != d20 && type != d100)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("Invalid dice type: %d", type);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 2);
        lua_createtable(L, roll_count, 0);
        int table = lua_gettop(L);
        int total = 0;

        for (int i = 0; i < roll_count; ++i)
        {
            uint32_t num = 0;
            if (type == d100)
            {
                num = pcg32_boundedrand_r(target, 10) * 10;
            }
            else if (type == d10)
            {
                num = pcg32_boundedrand_r(target, 10);
            }
            else
            {
                num = pcg32_boundedrand_r(target, type) + 1;
            }

            total += (int)num;
            lua_pushinteger(L, num);
            lua_rawseti(L, table, i + 1);
        }

        lua_pushinteger(L, total);
        return 2;
    }

    int toss(lua_State* L)
    {
        if (lua_gettop(L) != 1)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.toss: expected context id");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.toss: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        lua_pushinteger(L, pcg32_boundedrand_r(target, 2) ? 0 : 1);
        return 1;
    }

    int bound(lua_State* L)
    {
        if (lua_gettop(L) != 2)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.bound: expected id and upper_bound");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.bound: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        uint32_t upper_bound = 0;
        if (!check_uint32(L, 2, "rnd.bound: upper_bound", &upper_bound))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        if (upper_bound == 0)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.bound: upper_bound must be bigger than 0");
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        push_uint32_number(L, pcg32_boundedrand_r(target, upper_bound));
        return 1;
    }

    int range(lua_State* L)
    {
        if (lua_gettop(L) != 3)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.range: expected id, min, and max");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.range: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        uint32_t min = 0;
        if (!check_uint32(L, 2, "rnd.range: MIN", &min))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        uint32_t max = 0;
        if (!check_uint32(L, 3, "rnd.range: MAX", &max))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        if (min == max)
        {
            DM_LUA_STACK_CHECK(L, 1);
            push_uint32_number(L, min);
            return 1;
        }

        if (min > max)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.range: MAX(%u) must be bigger than MIN(%u)", max, min);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        push_uint32_number(L, random_range_uint32(target, min, max));
        return 1;
    }

    int numbers(lua_State* L)
    {
        if (lua_gettop(L) != 2)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.numbers: expected id and count");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.numbers: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        int count = 0;
        if (!check_count(L, 2, "rnd.numbers: count", &count))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        lua_createtable(L, count, 0);
        int table = lua_gettop(L);

        for (int i = 0; i < count; ++i)
        {
            push_uint32_number(L, pcg32_random_r(target));
            lua_rawseti(L, table, i + 1);
        }

        return 1;
    }

    int ranges(lua_State* L)
    {
        if (lua_gettop(L) != 4)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.ranges: expected id, count, min, and max");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.ranges: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        int count = 0;
        if (!check_count(L, 2, "rnd.ranges: count", &count))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        uint32_t min = 0;
        if (!check_uint32(L, 3, "rnd.ranges: MIN", &min))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        uint32_t max = 0;
        if (!check_uint32(L, 4, "rnd.ranges: MAX", &max))
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
            push_uint32_number(L, min == max ? min : random_range_uint32(target, min, max));
            lua_rawseti(L, table, i + 1);
        }

        return 1;
    }

    int doubles(lua_State* L)
    {
        if (lua_gettop(L) != 2)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.doubles: expected id and count");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.doubles: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        int count = 0;
        if (!check_count(L, 2, "rnd.doubles: count", &count))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        lua_createtable(L, count, 0);
        int table = lua_gettop(L);

        for (int i = 0; i < count; ++i)
        {
            lua_pushnumber(L, random_double(target));
            lua_rawseti(L, table, i + 1);
        }

        return 1;
    }

    int number(lua_State* L)
    {
        if (lua_gettop(L) != 1)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.number: expected context id");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.number: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        push_uint32_number(L, pcg32_random_r(target));
        return 1;
    }

    int card(lua_State* L)
    {
        if (lua_gettop(L) != 1)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.card: expected context id");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.card: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        lua_pushinteger(L, pcg32_boundedrand_r(target, 52) + 1);
        return 1;
    }

    int card2(lua_State* L)
    {
        if (lua_gettop(L) != 1)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.card2: expected context id");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.card2: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 2);
        lua_pushinteger(L, pcg32_boundedrand_r(target, 4) + 1);
        lua_pushinteger(L, pcg32_boundedrand_r(target, 13) + 1);
        return 2;
    }

    int rnd_boolean(lua_State* L)
    {
        if (lua_gettop(L) != 1)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.boolean: expected context id");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.boolean: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        lua_pushboolean(L, pcg32_boundedrand_r(target, 2) != 0);
        return 1;
    }

    int chance(lua_State* L)
    {
        if (lua_gettop(L) != 2)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.chance: expected id and probability");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.chance: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        double probability = luaL_checknumber(L, 2);
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
            lua_pushboolean(L, random_double(target) < probability);
        }

        return 1;
    }

    int shuffle(lua_State* L)
    {
        if (lua_gettop(L) != 2)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.shuffle: expected id and array");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.shuffle: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        if (!lua_istable(L, 2))
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.shuffle: array must be a table");
            return 0;
        }

        size_t count = lua_objlen(L, 2);
        if (count > UINT32_MAX)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.shuffle: array length(%zu) must fit in an unsigned 32-bit integer", count);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 1);
        for (size_t i = count; i > 1; --i)
        {
            uint32_t j = pcg32_boundedrand_r(target, (uint32_t)i) + 1;

            lua_rawgeti(L, 2, i);
            lua_rawgeti(L, 2, j);
            lua_rawseti(L, 2, i);
            lua_rawseti(L, 2, j);
        }

        lua_pushvalue(L, 2);
        return 1;
    }

    int check(lua_State* L)
    {
        if (lua_gettop(L) != 1)
        {
            DM_LUA_STACK_CHECK(L, 0);
            dmLogError("rnd.check: expected context id");
            return 0;
        }

        pcg32_random_t* target = 0;
        if (!check_context(L, 1, "rnd.check: id", &target))
        {
            DM_LUA_STACK_CHECK(L, 0);
            return 0;
        }

        DM_LUA_STACK_CHECK(L, 0);
        int      rounds = 1;
        int      round, i;

        uint32_t t = pcg32_random_r(target);
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
            printf("  32bit:");
            for (i = 0; i < 6; ++i)
                printf(" 0x%08x", pcg32_random_r(target));
            printf("\n");

            printf("  Coins: ");
            for (i = 0; i < 65; ++i)
                printf("%c", pcg32_boundedrand_r(target, 2) ? 'H' : 'T');
            printf("\n");

            printf("  Rolls:");
            for (i = 0; i < 33; ++i)
            {
                printf(" %d", (int)pcg32_boundedrand_r(target, 6) + 1);
            }
            printf("\n");

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
                int  chosen = pcg32_boundedrand_r(target, i);
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
} // namespace rnd
