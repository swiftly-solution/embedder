#ifndef _embedder_engine_functions_h
#define _embedder_engine_functions_h

#include "../Value.h"
#include "../dotnet/host.h"
#include <vector>

class FunctionContext
{
private:
    std::string m_function_key;
    ContextKinds m_kind;
    EContext* m_ctx;
    bool m_shouldSkipFirstArgument = false;
    bool m_skipCreatedUData = false;
    bool m_shouldSkipSecondArgument = false;

    int returnRef = LUA_NOREF;
    JSValue returnVal = JS_UNDEFINED;
    bool stopExecution = false;

    JSValue* m_vals;
    int m_argc;

public:
    std::vector<int64_t> temporaryData;

    FunctionContext(std::string function_key, ContextKinds kind, EContext* ctx, bool shouldSkipFirstArgument = false, bool skipCreatedUData = false, bool shouldSkipSecondArgument = false);
    FunctionContext(std::string function_key, ContextKinds kind, EContext* ctx, JSValue* vals, int argc);
    FunctionContext(std::string function_key, ContextKinds kind, EContext* ctx, CallContext* callctx, bool shouldSkipFirstArgument = false, bool skipUData = false);
    ~FunctionContext();

    bool HasResult();
    EContext* GetPluginContext();

    std::string GetFunctionKey();

    void StopExecution();
    bool ShouldStopExecution();

    void pushLuaResult()
    {
        if (returnRef == LUA_NOREF)
            return;
        lua_rawgeti(m_ctx->GetLuaState(), LUA_REGISTRYINDEX, returnRef);
    }

    JSValue pushJSResult()
    {
        return returnVal;
    }

    std::string GetDebugInfo()
    {
        if (m_kind == ContextKinds::Dotnet)
        {
            return ((CallContext*)m_vals)->GetDebugInfo();
        }
        else
        {
            return "";
        }
    }

    template <class T>
    T GetArgument(int index)
    {
        if (m_kind == ContextKinds::Lua)
        {
            if (index < 0 || index + 1 > lua_gettop(m_ctx->GetLuaState()) - (int)m_shouldSkipFirstArgument - (int)m_skipCreatedUData - (int)m_shouldSkipSecondArgument)
                return *(T*)0;

            return Stack<T>::getLua(m_ctx, index + 1 + (int)m_shouldSkipFirstArgument + (int)m_shouldSkipSecondArgument);
        }
        else if (m_kind == ContextKinds::JavaScript)
        {
            if (index < 0 || index >= m_argc)
                return *(T*)0;
            return Stack<T>::getJS(m_ctx, m_vals[index]);
        }
        else if (m_kind == ContextKinds::Dotnet)
        {
            if (index < 0 || index + 1 >((CallContext*)m_vals)->GetArgumentCount() - (int)m_shouldSkipFirstArgument - (int)m_skipCreatedUData)
                return *(T*)0;

            return Stack<T>::getDotnet(m_ctx, (CallContext*)m_vals, index + (int)m_shouldSkipFirstArgument + (int)m_skipCreatedUData);
        }
        else
            return *(T*)0;
    }

    std::string GetArgumentAsString(int index)
    {
        if (m_kind == ContextKinds::Lua)
        {
            if (index < 0 || index + 1 > lua_gettop(m_ctx->GetLuaState()) - (int)m_shouldSkipFirstArgument - (int)m_skipCreatedUData - (int)m_shouldSkipSecondArgument)
                return "";

            return EValue::fromLuaStack(m_ctx, index + 1 + (int)m_shouldSkipFirstArgument + (int)m_shouldSkipSecondArgument).tostring();
        }
        else if (m_kind == ContextKinds::JavaScript)
        {
            if (index < 0 || index >= m_argc)
                return "";

            return EValue::fromJSStack(m_ctx, m_vals[index]).tostring();
        }
        else if (m_kind == ContextKinds::Dotnet)
        {
            if (index < 0 || index + 1 >((CallContext*)m_vals)->GetArgumentCount() - (int)m_shouldSkipFirstArgument - (int)m_skipCreatedUData)
                return "";

            static char out[8192];
            memset(out, 0, sizeof(out));
            InterpretAsString(const_cast<void*>(((CallContext*)m_vals)->GetArgumentPtr(index + (int)m_shouldSkipFirstArgument + (int)m_skipCreatedUData)), ((CallContext*)m_vals)->GetArgumentType(index + (int)m_shouldSkipFirstArgument + (int)m_skipCreatedUData), out, sizeof(out));

            return std::string(out);
        }
        else
            return "";
    }

    template <class T>
    T GetArgumentOr(int index, T defaultVal)
    {
        if (m_kind == ContextKinds::Lua)
        {
            if (index < 0 || index + 1 > lua_gettop(m_ctx->GetLuaState()) - (int)m_shouldSkipFirstArgument - (int)m_skipCreatedUData - (int)m_shouldSkipSecondArgument)
                return defaultVal;

            return Stack<T>::getLua(m_ctx, index + 1 + (int)m_shouldSkipFirstArgument + (int)m_shouldSkipSecondArgument);
        }
        else if (m_kind == ContextKinds::JavaScript)
        {
            if (index < 0 || index >= m_argc)
                return defaultVal;
            return Stack<T>::getJS(m_ctx, m_vals[index]);
        }
        else if (m_kind == ContextKinds::Dotnet)
        {
            if (index < 0 || index >= ((CallContext*)m_vals)->GetArgumentCount() - (int)m_shouldSkipFirstArgument - (int)m_skipCreatedUData)
                return defaultVal;

            return Stack<T>::getDotnet(m_ctx, (CallContext*)m_vals, index + (int)m_shouldSkipFirstArgument + (int)m_skipCreatedUData);
        }
        else
            return defaultVal;
    }

    template <class T>
    void SetReturn(T value)
    {
        if (m_kind == ContextKinds::Lua)
        {
            if (returnRef != LUA_NOREF)
                luaL_unref(m_ctx->GetLuaState(), LUA_REGISTRYINDEX, returnRef);

            Stack<T>::pushLua(m_ctx, value);
            returnRef = luaL_ref(m_ctx->GetLuaState(), LUA_REGISTRYINDEX);
        }
        else if (m_kind == ContextKinds::JavaScript)
        {
            if (!JS_IsUndefined(returnVal))
                JS_FreeValue(m_ctx->GetJSState(), returnVal);

            returnVal = Stack<T>::pushJS(m_ctx, value);
        }
        else if (m_kind == ContextKinds::Dotnet)
        {
            Stack<T>::pushDotnet(m_ctx, (CallContext*)m_vals, value, true);
        }
    }

    template <class T>
    T GetReturn()
    {
        if (m_kind == ContextKinds::Lua)
        {
            if (returnRef == LUA_NOREF)
                return *(T*)0;
            return Stack<T>::getLua(m_ctx, returnRef);
        }
        else if (m_kind == ContextKinds::JavaScript)
        {
            if (JS_IsUndefined(returnVal))
                return *(T*)0;

            return Stack<T>::getJS(m_ctx, returnVal);
        }
        else if (m_kind == ContextKinds::Dotnet)
        {
            if (((CallContext*)m_vals)->m_has_return == 0) return *(T*)0;

            return Stack<T>::getDotnet(m_ctx, (CallContext*)m_vals, -1);
        }
        else
            return *(T*)0;
    }

    int GetArgumentsCount();
};

#endif