#ifndef _embedder_internal_value_h
#define _embedder_internal_value_h

#include <lua.hpp>
#include <quickjs.h>

#include "Context.h"
#include "Exception.h"
#include "Helpers.h"
#include "Stack.h"

class Vector;
class Vector2D;
class Vector4D;
class Color;
class QAngle;

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
    void* m_ptr = nullptr;
    int m_ptrtype = 0;

    void swap(EValue& other)
    {
        std::swap(m_ctx, other.m_ctx);
        std::swap(m_ref, other.m_ref);
        std::swap(m_val, other.m_val);
        std::swap(m_ptr, other.m_ptr);
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
        else if (ctx->GetKind() == ContextKinds::Dotnet) m_ptr = nullptr;
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
        else if (ctx->GetKind() == ContextKinds::Dotnet) {
            if constexpr (
                is_map<T>::value || is_vector<T>::value || std::is_same<std::string, T>::value || std::is_same<ClassData*, T>::value || std::is_same<Vector, T>::value ||
                std::is_same<Vector2D, T>::value || std::is_same<Vector4D, T>::value || std::is_same<Color, T>::value || std::is_same<QAngle, T>::value
            ) {
                void* val = (void*)Stack<T>::pushRawDotnet(ctx, nullptr, value);
                m_ptr = val;
            }
            else {
                T val = Stack<T>::pushRawDotnet(ctx, nullptr, value);
                m_ptr = *(void**)&val;
            }

            if constexpr (is_map<T>::value) m_ptrtype = 16;
            else if constexpr (is_vector<T>::value) m_ptrtype = 15;
            else if constexpr (std::is_same<std::string, T>::value) m_ptrtype = typesMap[typeid(std::string)];
            else m_ptrtype = typesMap[typeid(T)];
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

    EValue(EContext* ctx, void* value, int kind)
    {
        m_ctx = ctx;
        m_ctx->PushValue(this);
        if (ctx->GetKind() == ContextKinds::Dotnet) {
            m_ptr = value;
            m_ptrtype = kind;
        }
    }

    ~EValue()
    {
        if (nofree) return;

        if (m_ctx) m_ctx->PopValue(this);

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
        m_ptr = other.m_ptr;
    }

    EValue(const EValue& other) {
        EValue& nonConstOther = const_cast<EValue&>(other);

        m_ctx = nonConstOther.m_ctx;
        m_ctx->PushValue(this);
        m_ref = nonConstOther.createRef();
        m_val = nonConstOther.createValue();
        m_ptr = nonConstOther.m_ptr;
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

    void* getPointer() {
        return m_ptr;
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

    void* pushDotnet() {
        return m_ptr;
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
        else if (m_ctx->GetKind() == ContextKinds::Dotnet) {
            if constexpr (is_map<T>::value) return m_ptrtype == 16;
            else if constexpr (is_vector<T>::value) return m_ptrtype == 15;
            else if constexpr (std::is_same<std::string, T>::value) return m_ptrtype == typesMap[typeid(std::string)];
            else return m_ptrtype == typesMap[typeid(T)];
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
        else if (m_ctx->GetKind() == ContextKinds::Dotnet) {
            return Stack<T>::getRawDotnet(m_ctx, nullptr, (void*)&m_ptr);
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
        else if (m_ctx->GetKind() == ContextKinds::Dotnet) return m_ptr == nullptr;
        else return false;
    }

    bool isBool() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TBOOLEAN;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsBool(m_val);
        else if (m_ctx->GetKind() == ContextKinds::Dotnet) return m_ptrtype == typesMap[typeid(bool)];
        else return false;
    }

    bool isNumber() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TNUMBER;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsNumber(m_val);
        else if (m_ctx->GetKind() == ContextKinds::Dotnet) return m_ptrtype >= 2 && m_ptrtype <= 11;
        else return false;
    }

    bool isString() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TSTRING;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsString(m_val);
        else if (m_ctx->GetKind() == ContextKinds::Dotnet) return m_ptrtype == typesMap[typeid(std::string)];
        else return false;
    }

    bool isTable() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TTABLE;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsArray(m_val) || JS_IsObject(m_val);
        else if (m_ctx->GetKind() == ContextKinds::Dotnet) return m_ptrtype == 15 || m_ptrtype == 16;
        else return false;
    }

    bool isFunction() {
        if (m_ctx->GetKind() == ContextKinds::Lua) return getLuaType() == LUA_TFUNCTION;
        else if (m_ctx->GetKind() == ContextKinds::JavaScript) return JS_IsFunction((JSContext*)m_ctx->GetState(), m_val);
        else if (m_ctx->GetKind() == ContextKinds::Dotnet) return m_ptrtype == 17;
        else return false;
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
        else if (m_ctx->GetKind() == ContextKinds::Dotnet) {
            static char out[8192];
            memset(out, 0, sizeof(out));
            InterpretAsString((void*)&m_ptr, m_ptrtype, out, sizeof(out));

            return std::string(out);
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
            EException::xpcall(m_ctx, sizeof...(params), 1);
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
        else if (m_ctx->GetKind() == ContextKinds::Dotnet) {
            CallData data;
            data.args_count = 0;

            CallContext ctx(data);

            pushDotnetArguments(&ctx, std::forward<Params>(params)...);

            data.function_str = (const char*)m_ptr;
            data.function_len = strlen((const char*)m_ptr);

            DotnetUpdateGlobalStateCleanupLock(true);
            DotnetExecuteFunction(&data, m_ctx);
            DotnetUpdateGlobalStateCleanupLock(false);

            return EValue(m_ctx, ctx.GetResultPtr(), ctx.GetReturnType());
        }
        else return EValue(m_ctx);
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

    void pushDotnetArguments(CallContext* ctx) {}

    template<typename T, typename... Params>
    void pushDotnetArguments(CallContext* ctx, T& param, Params&&... params)
    {
        if constexpr (is_map<T>::value || is_vector<T>::value || std::is_same<T, std::any>::value) {
            ctx->PushArgument<void*>(Stack<T>::pushRawDotnet(m_ctx, ctx, param));
        }
        else {
            ctx->PushArgument(param);
        }
        pushDotnetArguments(ctx, std::forward<Params>(params)...);
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

    static EValue pushRawDotnet(EContext* ctx, void* context, EValue value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, EValue value, bool shouldReturn = false)
    {

    }

    static EValue getLua(EContext* ctx, int ref)
    {
        return EValue::fromLuaStack(ctx, ref);
    }

    static EValue getJS(EContext* ctx, JSValue value)
    {
        return EValue::fromJSStack(ctx, value);
    }

    static EValue getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(EValue*)value;
    }

    static EValue getDotnet(EContext* ctx, CallContext* context, bool shouldReturn = false)
    {
        return EValue(ctx);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return true;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return true;
    }

    static bool isDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        return true;
    }
};

#endif