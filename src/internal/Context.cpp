#include "Context.h"

static const luaL_Reg lualibs[] = {
    {"_G", luaopen_base},
    {LUA_TABLIBNAME, luaopen_table},
    {LUA_STRLIBNAME, luaopen_string},
    {LUA_MATHLIBNAME, luaopen_math},
    {LUA_DBLIBNAME, luaopen_debug},
    {LUA_COLIBNAME, luaopen_coroutine},
    {LUA_UTF8LIBNAME, luaopen_utf8},
    {LUA_IOLIBNAME, luaopen_io},
    {LUA_OSLIBNAME, luaopen_os},
    {NULL, NULL},
};

EContext::EContext(ContextKinds kind)
{
    m_kind = kind;

    if(kind == ContextKinds::Lua) {
        auto state = luaL_newstate();
        m_state = (void*)state;

        const luaL_Reg* lib = lualibs;
        for (; lib->func; lib++) RegisterLuaLib(lib->name, lib->func);
    } else if(kind == ContextKinds::JavaScript) {
        JSRuntime* rt = JS_NewRuntime();

        JS_SetMemoryLimit(rt, 80 * 1024);
        JS_SetMaxStackSize(rt, 10 * 1024);

        JSContext* ctx = JS_NewContext(rt);

        m_state = (void*)ctx;
    }

    EException::Enable(m_state, m_kind);
}

EContext::~EContext()
{
    if(m_kind == ContextKinds::Lua) {
        lua_close((lua_State*)m_state);
    } else if(m_kind == ContextKinds::JavaScript) {
        JSContext* ctx = (JSContext*)m_state;
        JSRuntime* rt = JS_GetRuntime(ctx);

        JS_FreeContext(ctx);
        JS_FreeRuntime(rt);
    }
}

ContextKinds EContext::GetKind()
{
    return m_kind;
}

void EContext::RegisterLuaLib(const char* libName, lua_CFunction func)
{
    luaL_requiref((lua_State*)m_state, libName, func, 1);
    lua_pop((lua_State*)m_state, 1);
}

int64_t EContext::GetMemoryUsage()
{
    if(m_kind == ContextKinds::Lua) {
        int64_t count = lua_gc((lua_State*)m_state, LUA_GCCOUNT, 0);
        count *= 1024;
        count += lua_gc((lua_State*)m_state, LUA_GCCOUNTB, 0);
        return count;
    } else if(m_kind == ContextKinds::JavaScript) {
        JSMemoryUsage stats;
        JS_ComputeMemoryUsage(JS_GetRuntime((JSContext*)m_state), &stats);
        return stats.memory_used_size;
    }
}