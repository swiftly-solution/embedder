#ifndef _embedder_internal_value_h
#define _embedder_internal_value_h

#include <lua.hpp>
#include <quickjs.h>

#include "Context.h"
#include "Exception.h"
#include "Helpers.h"
#include "Stack.h"

struct StackCallFunc
{
    JSValue* args;
    int count;
    int items;
};

class EValue
{
private:
    EContext* m_ctx;
    JSValue m_val = JS_NULL;
    bool nofree = false;

    void swap(EValue& other)
    {
        std::swap(m_ctx, other.m_ctx);
        std::swap(m_ref, other.m_ref);
        std::swap(m_val, other.m_val);
    }

public:
    int m_ref = LUA_NOREF;

    EValue() = default;

    EValue(EContext* ctx)
    {
        m_ctx = ctx;
        m_ctx->PushValue(this);
        if (ctx->GetKind() == ContextKinds::Lua) m_ref = LUA_REFNIL;
        else if (ctx->GetKind() == ContextKinds::JavaScript) m_val = JS_NULL;
    }

    template<class T>
    EValue(EContext* ctx, T value)
    {
        m_ctx = ctx;
        m_ctx->PushValue(this);
        if (ctx->GetKind() == ContextKinds::Lua) {
            Stack<T>::pushLua(ctx, value);
            m_ref = luaL_ref((lua_State*)ctx->GetState(), LUA_REGISTRYINDEX);
        }
        else if (ctx->GetKind() == ContextKinds::JavaScript) {
            m_val = Stack<T>::pushJS(ctx, value);
        }
    }

    EValue(EContext* ctx, int indexStack, bool pushIndexStack)
    {
        (void)pushIndexStack;
        m_ctx = ctx;
        m_ctx->PushValue(this);
        if (ctx->GetKind() == ContextKinds::Lua) {
            if (indexStack != 0) lua_pushvalue((lua_State*)ctx->GetState(), indexStack);
            m_ref = luaL_ref((lua_State*)ctx->GetState(), LUA_REGISTRYINDEX);
        }
        else if (ctx->GetKind() == ContextKinds::JavaScript) {}
    }

    EValue(EContext* ctx, JSValue value)
    {
        m_ctx = ctx;
        m_ctx->PushValue(this);
        if (ctx->GetKind() == ContextKinds::Lua) {}
        else if (ctx->GetKind() == ContextKinds::JavaScript) {
            m_val = JS_DupValue((JSContext*)ctx->GetState(), value);
        }
    }

    ~EValue()
    {
        if (nofree) return;

        m_ctx->PopValue(this);
        if (m_ctx->GetKind() == ContextKinds::Lua) {
            if (m_ref == LUA_NOREF) return;
            luaL_unref((lua_State*)m_ctx->GetState(), LUA_REGISTRYINDEX, m_ref);
        }
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) {
            if (JS_IsNull(m_val)) return;
            JS_FreeValue((JSContext*)m_ctx->GetState(), m_val);
        }
    }

    EValue(EValue& other) {
        m_ctx = other.m_ctx;
        m_ctx->PushValue(this);
        m_ref = other.createRef();
        m_val = other.createValue();
    }

    EValue(const EValue& other) {
        EValue& nonConstOther = const_cast<EValue&>(other);

        m_ctx = nonConstOther.m_ctx;
        m_ctx->PushValue(this);
        m_ref = nonConstOther.createRef();
        m_val = nonConstOther.createValue();
    }

    int createRef() {
        if (m_ctx->GetKind() != ContextKinds::Lua) return LUA_NOREF;
        pushLua();
        return luaL_ref((lua_State*)m_ctx->GetState(), LUA_REGISTRYINDEX);
    }

    JSValue createValue() {
        if (m_ctx->GetKind() != ContextKinds::JavaScript) return JS_NULL;
        return JS_DupValue((JSContext*)m_ctx->GetState(), m_val);
    }

    EValue& operator=(EValue& rhs)
    {
        EValue val(rhs);
        swap(val);
        return *this;
    }

    template<class T>
    EValue& operator=(T rhs)
    {
        EValue val(m_ctx, rhs);
        swap(val);
        return *this;
    }

    void pushLua() {
        if (m_ctx->GetKind() != ContextKinds::Lua) return;
        lua_rawgeti((lua_State*)m_ctx->GetState(), LUA_REGISTRYINDEX, m_ref);
    }

    JSValue pushJS() {
        if (m_ctx->GetKind() != ContextKinds::JavaScript) return JS_NULL;
        return m_val;
    }

    template<class T>
    bool isInstance()
    {
        if (m_ctx->GetKind() == ContextKinds::Lua) {
            pushLua();
            bool ans = Stack<T>::isLuaInstance(m_ctx, -1);
            lua_pop((lua_State*)m_ctx->GetState(), 1);
            return ans;
        }
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) {
            return Stack<T>::isJSInstance(m_ctx, m_val);
        }
        else return false;
    }

    template<class T>
    T cast()
    {
        if (m_ctx->GetKind() == ContextKinds::Lua) {
            pushLua();
            T val = Stack<T>::getLua(m_ctx, -1);
            lua_pop((lua_State*)m_ctx->GetState(), 1);
            return val;
        }
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) {
            return Stack<T>::getJS(m_ctx, m_val);
        }
        else {
            return *(T*)0;
        }
    }

    template<class T>
    T cast_or(T value)
    {
        if (!isInstance<T>()) return value;
        return cast<T>();
    }

    template<class T>
    operator T()
    {
        return cast<T>();
    }

private:
    int getLuaType() {
        lua_State* L = (lua_State*)m_ctx->GetState();
        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
        int type = lua_type(L, -1);
        lua_pop(L, 1);
        return type;
    }

public:
    bool isNull() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TNIL;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsNull(m_val);
        else return false;
    }

    bool isBool() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TBOOLEAN;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsBool(m_val);
        else return false;
    }

    bool isNumber() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TNUMBER;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsNumber(m_val);
        else return false;
    }

    bool isString() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TSTRING;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsString(m_val);
        else return false;
    }

    bool isTable() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TTABLE;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsArray(m_ctx->GetJSState(), m_val) || JS_IsObject(m_val);
        else return false;
    }

    bool isFunction() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TFUNCTION;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsFunction((JSContext*)m_ctx->GetState(), m_val);
        else return false;
    }

    template<class T>
    EValue operator[](T value)
    {
        if (!isTable()) return *this;

        if (m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();
            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
            Stack<T>::pushLua(m_ctx, value);
            lua_rawget(L, lua_absindex(L, -2));
            return EValue(m_ctx, 0, true);
        }
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ctx = (JSContext*)m_ctx->GetState();

            if constexpr (std::is_same<T, std::string>::value) {
                std::string str = value;
                JSValue vl = JS_GetPropertyStr(ctx, m_val, str.c_str());
                return EValue(m_ctx, vl);
            }
            else if constexpr (std::is_same<T, const char*>::value) {
                JSValue vl = JS_GetPropertyStr(ctx, m_val, (const char*)value);
                return EValue(m_ctx, vl);
            }
            else if constexpr (std::is_same<T, int8_t>::value || std::is_same<T, int16_t>::value || std::is_same<T, int32_t>::value || std::is_same<T, int64_t>::value || std::is_same<T, uint8_t>::value || std::is_same<T, uint16_t>::value || std::is_same<T, uint32_t>::value || std::is_same<T, uint64_t>::value) {
                JSValue vl = JS_GetPropertyInt64(ctx, m_val, (int64_t)value);
                return EValue(m_ctx, vl);
            }

            return EValue(m_ctx);
        }
        else return *this;
    }

    template<class T>
    EValue setProperty(std::string key, T value)
    {
        if (!isTable()) return *this;

        if (m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();
            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
            Stack<T>::pushLua(m_ctx, value);
            rawsetfield(L, -2, key.c_str());
            lua_pop(L, 1);
        }
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ctx = (JSContext*)m_ctx->GetState();
            JS_SetPropertyStr(ctx, m_val, key.c_str(), Stack<T>::pushJS(m_ctx, value));
        }

        return *this;
    }

    std::string tostring()
    {
        if (m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();
            lua_getglobal(L, "tostring");
            pushLua();
            lua_call(L, 1, 1);
            const char* str = lua_tostring(L, -1);
            lua_pop(L, 1);
            return str;
        }
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* L = (JSContext*)m_ctx->GetState();

            auto value = JS_ToCString(L, m_val);
            if (value == nullptr) {
                std::string className = getClassName(L, m_val);
                if (className != "") return className;
            }

            std::string out(value);
            JS_FreeCString(L, value);
            return out;
        }
        else return "";
    }

    EContext* getContext()
    {
        return m_ctx;
    }

    static EValue fromLuaStack(EContext* ctx) {
        EValue newval(ctx, 0, true);
        return newval;
    }

    static EValue fromLuaStack(EContext* ctx, int idx) {
        EValue newval(ctx, idx, true);
        return newval;
    }

    static EValue fromJSStack(EContext* ctx, JSValue val) {
        EValue newval(ctx, val);
        return newval;
    }

    template<typename... Params>
    EValue operator()(Params&&... params)
    {
        if (m_ctx->GetKind() == ContextKinds::Lua) {
            pushLua();
            pushLuaArguments(std::forward<Params>(params)...);
            EException::pcall(m_ctx, sizeof...(params), 1);
            return EValue::fromLuaStack(m_ctx);
        }
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* state = (JSContext*)m_ctx->GetState();

            StackCallFunc callstack;
            callstack.count = sizeof...(Params);
            callstack.items = 0;
            if (callstack.count > 0)
                callstack.args = (JSValue*)malloc(callstack.count * sizeof(JSValue));
            else
                callstack.args = nullptr;

            pushJSArguments(callstack, std::forward<Params>(params)...);

            JSValue result = JS_Call(state, m_val, JS_UNDEFINED, callstack.items, callstack.args);

            if (callstack.args != nullptr) {
                for (size_t i = 0; i < callstack.items; i++) {
                    JS_FreeValue(state, callstack.args[i]);
                }

                free(callstack.args);
            }

            if (JS_IsException(result)) EException::Throw(EException(m_ctx->GetState(), m_ctx->GetKind(), -1));

            EValue returnVal(m_ctx, result);
            JS_FreeValue(state, result);
            return returnVal;
        }
        else return EValue(m_ctx);
    }

    static EValue getGlobal(EContext* ctx, std::string global_name)
    {
        if (ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)ctx->GetState();
            lua_pushglobaltable(L);
            rawgetfield(L, -1, global_name.c_str());
            EValue val(ctx, 0, true);
            lua_pop(L, 1);
            return val;
        }
        else if (ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ct = (JSContext*)ctx->GetState();
            auto db = JS_GetGlobalObject(ct);
            EValue val(ctx, JS_GetPropertyStr(ct, db, global_name.c_str()));
            JS_FreeValue(ct, db);
            return val;
        }
        else return EValue(ctx);
    }

    void MarkNoFree()
    {
        nofree = true;
    }

    template<class T>
    bool operator==(T const& rhs) const
    {
        return true;
    }

    template<class T>
    bool operator<(T const& rhs) const
    {
        return true;

    }

    template<class T>
    bool operator<=(T const& rhs) const
    {
        return true;

    }

    template<class T>
    bool operator>(T const& rhs) const
    {
        return true;

    }

    template<class T>
    bool operator>=(T rhs) const
    {
        return true;
    }

private:
    void pushLuaArguments() {}

    template<typename T, typename... Params>
    void pushLuaArguments(T& param, Params&&... params)
    {
        Stack<T>::pushLua(m_ctx, param);
        pushLuaArguments(std::forward<Params>(params)...);
    }

    void pushJSArguments(StackCallFunc& data) {}

    template<typename T, typename... Params>
    void pushJSArguments(StackCallFunc& data, T& param, Params&&... params)
    {
        if (data.args == nullptr) return;

        data.args[data.items] = Stack<T>::pushJS(m_ctx, param);
        data.items++;

        pushJSArguments(data, std::forward<Params>(params)...);
    }
};

template<>
struct Stack<EValue>
{
    static void pushLua(EContext* ctx, EValue value)
    {
        value.pushLua();
    }

    static JSValue pushJS(EContext* ctx, EValue value)
    {
        return JS_DupValue((JSContext*)ctx->GetState(), value.pushJS());
    }

    static EValue getLua(EContext* ctx, int ref)
    {
        return EValue::fromLuaStack(ctx, ref);
    }

    static EValue getJS(EContext* ctx, JSValue value)
    {
        return EValue::fromJSStack(ctx, value);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return true;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return true;
    }
};

#endif