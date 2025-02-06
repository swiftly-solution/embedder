#include "Context.h"
#include "CHelpers.h"
#include "Exception.h"
#include "Value.h"

#include <set>

std::set<EContext*> ctxs;

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
        JSContext* ctx = JS_NewContext(rt);

        JSValue global_obj = JS_GetGlobalObject(ctx);
        JSValue console_obj = JS_NewObject(ctx);

        JS_SetPropertyStr(ctx, console_obj, "log",
            JS_NewCFunction(ctx, CHelpers::js_print_to_console, "log", 1));
        JS_SetPropertyStr(ctx, global_obj, "console", console_obj);

        JS_FreeValue(ctx, global_obj);   

        m_state = (void*)ctx;
    }

    EException::Enable(m_state, m_kind);

    ctxs.insert(this);
}

EContext::~EContext()
{
    ctxs.erase(this);

    for(auto it = mappedValues.begin(); it != mappedValues.end(); ++it)
        delete (*it);

    mappedValues.clear();

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
    } else return 0;
}

int EContext::RunCode(std::string code)
{
    if(m_kind == ContextKinds::Lua) {
        int cd = (luaL_dostring((lua_State*)m_state, code.c_str()));
        if(cd != 0) EException::Throw(EException(GetState(), GetKind(), cd));
        return cd;
    } else if(m_kind == ContextKinds::JavaScript) {
        auto res = JS_Eval((JSContext*)m_state, code.c_str(), code.length(), "runcode.js", JS_EVAL_TYPE_GLOBAL);
        bool isException = JS_IsException(res);
        JS_FreeValue((JSContext*)m_state, res);
        return (int)isException;
    } else return 0;
}

void EContext::PushValue(EValue* val)
{
    if(mappedValues.find(val) != mappedValues.end()) return;
    mappedValues.insert(val);
}

void EContext::PopValue(EValue* val)
{
    if(mappedValues.find(val) == mappedValues.end()) return;
    mappedValues.erase(val);
}

void* EContext::GetState()
{
    return m_state;
}

EContext* GetContextByState(JSContext* ctx)
{
    for(auto it = ctxs.begin(); it != ctxs.end(); ++it)
    {
        EContext* ct = *it;
        if(ct->GetState() == ctx)
            return ct;
    }
    return nullptr;
}

EContext* GetContextByState(lua_State* ctx)
{
    for(auto it = ctxs.begin(); it != ctxs.end(); ++it)
    {
        EContext* ct = *it;
        if(ct->GetState() == ctx)
            return ct;
    }
    return nullptr;
}