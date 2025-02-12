#ifndef _embedder_internal_value_h
#define _embedder_internal_value_h

#include <lua.hpp>
#include <quickjs.h>

#include "Context.h"
#include "Exception.h"

class EValue
{
private:
    EContext* m_ctx;
    int m_ref = LUA_NOREF;
    JSValue m_val = JS_NULL;

    void swap(EValue& other)
    {
        std::swap(m_ctx, other.m_ctx);
        std::swap(m_ref, other.m_ref);
        std::swap(m_val, other.m_val);
    }

public:
    EValue(EContext* ctx)
    {
        m_ctx = ctx;
        m_ctx->PushValue(this);
        if(ctx->GetKind() == ContextKinds::Lua) m_ref = LUA_NOREF;
        else if(ctx->GetKind() == ContextKinds::JavaScript) m_val = JS_NULL;
    }
    
    template<class T>
    EValue(EContext* ctx, T value)
    {
        m_ctx = ctx;
        m_ctx->PushValue(this);
        if(ctx->GetKind() == ContextKinds::Lua) {
            Stack<T>::pushLua(ctx, value);
            m_ref = luaL_ref((lua_State*)ctx->GetState(), LUA_REGISTRYINDEX);
        } else if(ctx->GetKind() == ContextKinds::JavaScript) {
            m_val = Stack<T>::pushJS(ctx, value);
        }
    }

    EValue(EContext* ctx, int indexStack, bool pushIndexStack)
    {
        (void)pushIndexStack;
        m_ctx = ctx;
        m_ctx->PushValue(this);
        if(ctx->GetKind() == ContextKinds::Lua) {
            if(indexStack != 0) lua_pushvalue((lua_State*)ctx->GetState(), indexStack);
            m_ref = luaL_ref((lua_State*)ctx->GetState(), LUA_REGISTRYINDEX);
        } else if(ctx->GetKind() == ContextKinds::JavaScript) {}
    }

    EValue(EContext* ctx, JSValue value)
    {
        m_ctx = ctx;
        m_ctx->PushValue(this);
        if(ctx->GetKind() == ContextKinds::Lua) {} 
        else if(ctx->GetKind() == ContextKinds::JavaScript) {
            m_val = JS_DupValue((JSContext*)ctx->GetState(), value);
        }
    }

    ~EValue()
    {
        m_ctx->PopValue(this);
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            if(m_ref == LUA_NOREF) return;
            luaL_unref((lua_State*)m_ctx->GetState(), LUA_REGISTRYINDEX, m_ref);
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            if(JS_IsNull(m_val)) return;
            JS_FreeValue((JSContext*)m_ctx->GetState(), m_val);
        }
    }

    EValue(EValue& other) {
        m_ctx = other.m_ctx;
        m_ctx->PushValue(this);
        m_ref = other.createRef();
        m_val = other.createValue();
    }

    int createRef() {
        if(m_ctx->GetKind() != ContextKinds::Lua) return LUA_NOREF;
        pushLua();
        return luaL_ref((lua_State*)m_ctx->GetState(), LUA_REGISTRYINDEX);
    }

    JSValue createValue() {
        if(m_ctx->GetKind() != ContextKinds::JavaScript) return JS_NULL;
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
        if(m_ctx->GetKind() != ContextKinds::Lua) return;
        lua_rawgeti((lua_State*)m_ctx->GetState(), LUA_REGISTRYINDEX, m_ref);
    }

    JSValue pushJS() {
        if(m_ctx->GetKind() != ContextKinds::JavaScript) return JS_NULL;
        return m_val;
    }

    template<class T>
    bool isInstance()
    {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            pushLua();
            bool ans = Stack<T>::isLuaInstance(m_ctx, -1);
            lua_pop((lua_State*)m_ctx->GetState(), 1);
            return ans;
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            return Stack<T>::isJSInstance(m_ctx, m_val);
        } else return false;
    }

    template<class T>
    T cast()
    {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            pushLua();
            T val = Stack<T>::getLua(m_ctx, -1);
            lua_pop((lua_State*)m_ctx->GetState(), 1);
            return val;
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            return Stack<T>::getJS(m_ctx, m_val);
        } else {
            return *(T*)0;
        }
    }

    template<class T>
    T cast_or(T value)
    {
        if(!isInstance<T>()) return value;
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
        if(m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TNIL;
        else if(m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsNull(m_val);
        else return false;
    }

    bool isBool() {
        if(m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TBOOLEAN;
        else if(m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsBool(m_val);
        else return false;
    }

    bool isNumber() {
        if(m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TNUMBER;
        else if(m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsNumber(m_val);
        else return false;
    }

    bool isString() {
        if(m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TSTRING;
        else if(m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsString(m_val);
        else return false;
    }

    bool isTable() {
        if(m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TTABLE;
        else if(m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsArray((JSContext*)m_ctx->GetState(), m_val) || JS_IsObject(m_val);
        else return false;
    }

    bool isFunction() {
        if(m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TFUNCTION;
        else if(m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsFunction((JSContext*)m_ctx->GetState(), m_val);
        else return false;
    }

    template<class T>
    EValue operator[](T value)
    {
        if(!isTable()) return *this;

        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();
            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
            Stack<T>::pushLua(m_ctx, value);
            lua_rawget(L, lua_absindex(L, -2));
            return EValue(m_ctx, 0, true);
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ctx = (JSContext*)m_ctx->GetState();
            JSAtom at = JS_ValueToAtom(ctx, Stack<T>::pushJS(m_ctx, value));
            JSValue vl = JS_GetProperty(ctx, m_val, at);
            JS_FreeAtom(ctx, at);
            return EValue(m_ctx, vl);
        } else return *this;
    }

    std::string tostring()
    {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();
            lua_getglobal(L, "tostring");
            pushLua();
            lua_call(L, 1, 1);
            const char* str = lua_tostring(L, -1);
            lua_pop(L, 1);
            return str;
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* L = (JSContext*)m_ctx->GetState();
            auto value = JS_ToCString(L, m_val);
            std::string out(value);
            JS_FreeCString(L, value);
            return out;
        } else return "";
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
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            pushLua();
            pushLuaArguments(std::forward<Params>(params)...);
            EException::pcall(m_ctx, sizeof...(params), 1);
            return EValue::fromLuaStack(m_ctx);
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* state = (JSContext*)m_ctx->GetState();

            constexpr size_t argCount = sizeof...(Params);
            JSValue args[argCount] = { Stack<std::decay_t<Params>>::pushJS(m_ctx, std::forward<Params>(params))... };
            
            JSValue result = JS_Call(state, m_val, JS_UNDEFINED, argCount, args);
            
            for(size_t i = 0; i < argCount; i++) {
                JS_FreeValue(state, args[i]);
            }

            if(JS_IsException(result)) EException::Throw(EException(m_ctx->GetState(), m_ctx->GetKind(), -1));

            EValue returnVal(m_ctx, result);
            JS_FreeValue(state, result);
            return returnVal;
        } else return EValue(m_ctx);
    }

    static EValue getGlobal(EContext* ctx, std::string global_name)
    {
        if(ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)ctx->GetState();
            lua_pushglobaltable(L);
            rawgetfield(L, -1, global_name.c_str());
            EValue val(ctx, 0, true);
            lua_pop(L, 1);
            return val;
        } else if(ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ct = (JSContext*)ctx->GetState();
            auto db = JS_GetGlobalObject(ct);
            EValue val(ctx, JS_GetPropertyStr(ct, db, global_name.c_str()));
            JS_FreeValue(ct, db);
            return val;
        } else return EValue(ctx);
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
        return value.pushJS();
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