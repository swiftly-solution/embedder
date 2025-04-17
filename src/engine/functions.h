#ifndef _embedder_engine_functions_h
#define _embedder_engine_functions_h

#include "../Value.h"
#include <vector>

class FunctionContext
{
private:
    std::string m_function_key;
    ContextKinds m_kind;
    EContext *m_ctx;
    bool m_shouldSkipFirstArgument = false;
    bool m_skipCreatedUData = false;
    bool m_shouldSkipSecondArgument = false;

    int returnRef = LUA_NOREF;
    JSValue returnVal = JS_UNDEFINED;
    bool stopExecution = false;

    JSValue *m_vals;
    int m_argc;

    
public:
    std::vector<int64_t> temporaryData;
    
    FunctionContext(std::string function_key, ContextKinds kind, EContext *ctx, bool shouldSkipFirstArgument = false, bool skipCreatedUData = false, bool shouldSkipSecondArgument = false);
    FunctionContext(std::string function_key, ContextKinds kind, EContext *ctx, JSValue *vals, int argc);
    ~FunctionContext();

    bool HasResult();
    EContext *GetPluginContext();

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

    template <class T>
    T GetArgument(int index)
    {
        if (m_kind == ContextKinds::Lua)
        {
            if (index < 0 || index + 1 > lua_gettop(m_ctx->GetLuaState()) - (int)m_shouldSkipFirstArgument - (int)m_skipCreatedUData - (int)m_shouldSkipSecondArgument)
                return *(T *)0;

            return Stack<T>::getLua(m_ctx, index + 1 + (int)m_shouldSkipFirstArgument + (int)m_shouldSkipSecondArgument);
        }
        else if (m_kind == ContextKinds::JavaScript)
        {
            if (index < 0 || index >= m_argc)
                return *(T *)0;
            return Stack<T>::getJS(m_ctx, m_vals[index]);
        }
        else
            return *(T *)0;
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
    }

    template <class T>
    T GetReturn()
    {
        if (m_kind == ContextKinds::Lua)
        {
            if (returnRef == LUA_NOREF)
                return *(T *)0;
            return Stack<T>::getLua(m_ctx, returnRef);
        }
        else if (m_kind == ContextKinds::JavaScript)
        {
            if (JS_IsUndefined(returnVal))
                return *(T *)0;

            return Stack<T>::getJS(m_ctx, returnVal);
        }
        else
            return *(T *)0;
    }

    int GetArgumentsCount();
};

#endif