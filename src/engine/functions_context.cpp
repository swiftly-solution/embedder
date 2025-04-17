#include "functions.h"

typedef void (*ScriptingFunctionCallback)(FunctionContext*);

FunctionContext::FunctionContext(std::string function_key, ContextKinds kind, EContext *ctx, bool shouldSkipFirstArgument, bool skipCreatedUData, bool shouldSkipSecondArgument)
{
    m_function_key = function_key;
    m_kind = kind;
    m_ctx = ctx;
    m_shouldSkipFirstArgument = shouldSkipFirstArgument;
    m_skipCreatedUData = skipCreatedUData;
    m_shouldSkipSecondArgument = shouldSkipSecondArgument;

    void* cb = ctx->GetFunctionCall("_G OnFunctionContextRegister");
    if(!cb) return;
    reinterpret_cast<ScriptingFunctionCallback>(cb)(this);
}

FunctionContext::FunctionContext(std::string function_key, ContextKinds kind, EContext *ctx, JSValue *vals, int argc)
{
    m_function_key = function_key;
    m_kind = kind;
    m_ctx = ctx;
    m_vals = vals;
    m_argc = argc;

    void* cb = ctx->GetFunctionCall("_G OnFunctionContextRegister");
    if(!cb) return;
    reinterpret_cast<ScriptingFunctionCallback>(cb)(this);
}

FunctionContext::~FunctionContext()
{
    if (returnRef != LUA_NOREF)
        luaL_unref(m_ctx->GetLuaState(), LUA_REGISTRYINDEX, returnRef);

    void* cb = m_ctx->GetFunctionCall("_G OnFunctionContextUnregister");
    if(!cb) return;
    reinterpret_cast<ScriptingFunctionCallback>(cb)(this);
}

bool FunctionContext::HasResult()
{
    return (returnRef != LUA_NOREF || !JS_IsUndefined(returnVal));
}

void FunctionContext::StopExecution()
{
    stopExecution = true;
}

bool FunctionContext::ShouldStopExecution()
{
    return stopExecution;
}

int FunctionContext::GetArgumentsCount()
{
    if (m_kind == ContextKinds::Lua)
    {
        return lua_gettop(m_ctx->GetLuaState()) - (int)m_shouldSkipFirstArgument - (int)m_skipCreatedUData - (int)m_shouldSkipSecondArgument;
    }
    else if (m_kind == ContextKinds::JavaScript)
    {
        return m_argc;
    }
    else
        return 0;
}

EContext *FunctionContext::GetPluginContext()
{
    return m_ctx;
}

std::string FunctionContext::GetFunctionKey()
{
    return m_function_key;
}