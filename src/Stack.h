#ifndef _embedder_internal_stack_h
#define _embedder_internal_stack_h

#include <optional>
#include <array>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <utility>
#include <typeinfo>
#include <any>

#include "Context.h"
#include "Helpers.h"
#include "GarbageCollector.h"
#include "engine/classes.h"

template <class T>
struct Stack;

template <>
struct Stack<void>
{
    static void pushLua(EContext *) {}
    static void pushJS(EContext *) {}
};

template <>
struct Stack<EContext *>
{
    static EContext *getLua(EContext *ctx, int) { return ctx; }
    static EContext *getJS(EContext *ctx, JSValue) { return ctx; }
};

template <>
struct Stack<nullptr_t>
{
    static void pushLua(EContext *ctx, nullptr_t value = nullptr)
    {
        lua_pushnil((lua_State *)(ctx->GetState()));
    }

    static JSValue pushJS(EContext *ctx, nullptr_t value = nullptr)
    {
        return JS_NULL;
    }

    static nullptr_t getLua(EContext *ctx, int ref)
    {
        return nullptr;
    }

    static nullptr_t getJS(EContext *ctx, JSValue value)
    {
        return nullptr;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_isnil((lua_State *)(ctx->GetState()), ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsNull(value);
    }
};

template <>
struct Stack<int>
{
    static void pushLua(EContext *ctx, int value)
    {
        lua_pushinteger((lua_State *)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext *ctx, int value)
    {
        return JS_NewInt32((JSContext *)(ctx->GetState()), value);
    }

    static int getLua(EContext *ctx, int ref)
    {
        return (int)luaL_checkinteger((lua_State *)(ctx->GetState()), ref);
    }

    static int getJS(EContext *ctx, JSValue value)
    {
        int val;
        JS_ToInt32((JSContext *)(ctx->GetState()), &val, value);
        return val;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        if (lua_type((lua_State *)(ctx->GetState()), ref) != LUA_TNUMBER)
            return false;

        int isNumber;
        lua_tointegerx((lua_State *)(ctx->GetState()), ref, &isNumber);
        return isNumber;
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsNumber(value);
    }
};

template <>
struct Stack<unsigned int>
{
    static void pushLua(EContext *ctx, unsigned int value)
    {
        lua_pushinteger((lua_State *)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext *ctx, unsigned int value)
    {
        return JS_NewUint32((JSContext *)(ctx->GetState()), value);
    }

    static unsigned int getLua(EContext *ctx, int ref)
    {
        return (unsigned int)luaL_checkinteger((lua_State *)(ctx->GetState()), ref);
    }

    static unsigned int getJS(EContext *ctx, JSValue value)
    {
        unsigned int val;
        JS_ToUint32((JSContext *)(ctx->GetState()), &val, value);
        return val;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }
};

template <>
struct Stack<uint8_t>
{
    static void pushLua(EContext *ctx, uint8_t value)
    {
        lua_pushinteger((lua_State *)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext *ctx, uint8_t value)
    {
        return JS_NewUint32((JSContext *)(ctx->GetState()), value);
    }

    static uint8_t getLua(EContext *ctx, int ref)
    {
        return (uint8_t)luaL_checkinteger((lua_State *)(ctx->GetState()), ref);
    }

    static uint8_t getJS(EContext *ctx, JSValue value)
    {
        unsigned int val;
        JS_ToUint32((JSContext *)(ctx->GetState()), &val, value);
        return (uint8_t)val;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }
};

template <>
struct Stack<int16_t>
{
    static void pushLua(EContext *ctx, int16_t value)
    {
        lua_pushinteger((lua_State *)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext *ctx, int16_t value)
    {
        return JS_NewInt32((JSContext *)(ctx->GetState()), value);
    }

    static int16_t getLua(EContext *ctx, int ref)
    {
        return (int16_t)luaL_checkinteger((lua_State *)(ctx->GetState()), ref);
    }

    static int16_t getJS(EContext *ctx, JSValue value)
    {
        int val;
        JS_ToInt32((JSContext *)(ctx->GetState()), &val, value);
        return (int16_t)val;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }
};

template <>
struct Stack<uint16_t>
{
    static void pushLua(EContext *ctx, uint16_t value)
    {
        lua_pushinteger((lua_State *)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext *ctx, uint16_t value)
    {
        return JS_NewUint32((JSContext *)(ctx->GetState()), value);
    }

    static uint16_t getLua(EContext *ctx, int ref)
    {
        return (uint16_t)luaL_checkinteger((lua_State *)(ctx->GetState()), ref);
    }

    static uint16_t getJS(EContext *ctx, JSValue value)
    {
        unsigned int val;
        JS_ToUint32((JSContext *)(ctx->GetState()), &val, value);
        return (uint16_t)val;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }
};

template <>
struct Stack<int8_t>
{
    static void pushLua(EContext *ctx, int8_t value)
    {
        lua_pushinteger((lua_State *)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext *ctx, int8_t value)
    {
        return JS_NewUint32((JSContext *)(ctx->GetState()), value);
    }

    static int8_t getLua(EContext *ctx, int ref)
    {
        return (int8_t)luaL_checkinteger((lua_State *)(ctx->GetState()), ref);
    }

    static int8_t getJS(EContext *ctx, JSValue value)
    {
        unsigned int val;
        JS_ToUint32((JSContext *)(ctx->GetState()), &val, value);
        return (int8_t)val;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }
};

template <>
struct Stack<int64_t>
{
    static void pushLua(EContext *ctx, int64_t value)
    {
        lua_pushinteger((lua_State *)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext *ctx, int64_t value)
    {
        return JS_NewInt64((JSContext *)(ctx->GetState()), value);
    }

    static int64_t getLua(EContext *ctx, int ref)
    {
        return (int64_t)luaL_checkinteger((lua_State *)(ctx->GetState()), ref);
    }

    static int64_t getJS(EContext *ctx, JSValue value)
    {
        int64_t val;
        JS_ToInt64((JSContext *)(ctx->GetState()), &val, value);
        return (int64_t)val;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }
};

template <>
struct Stack<uint64_t>
{
    static void pushLua(EContext *ctx, uint64_t value)
    {
        lua_pushinteger((lua_State *)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext *ctx, uint64_t value)
    {
        return JS_NewBigUint64((JSContext *)(ctx->GetState()), value);
    }

    static uint64_t getLua(EContext *ctx, int ref)
    {
        return (uint64_t)luaL_checkinteger((lua_State *)(ctx->GetState()), ref);
    }

    static uint64_t getJS(EContext *ctx, JSValue value)
    {
        uint64_t val;
        JS_ToBigUint64((JSContext *)(ctx->GetState()), &val, value);
        return (uint64_t)val;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }
};

template <>
struct Stack<float>
{
    static void pushLua(EContext *ctx, float value)
    {
        lua_pushnumber((lua_State *)(ctx->GetState()), (lua_Number)value);
    }

    static JSValue pushJS(EContext *ctx, float value)
    {
        return JS_NewFloat64((JSContext *)(ctx->GetState()), (double)value);
    }

    static float getLua(EContext *ctx, int ref)
    {
        return (float)luaL_checknumber((lua_State *)(ctx->GetState()), ref);
    }

    static float getJS(EContext *ctx, JSValue value)
    {
        double val;
        JS_ToFloat64((JSContext *)(ctx->GetState()), &val, value);
        return (float)val;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_type((lua_State *)(ctx->GetState()), ref) == LUA_TNUMBER;
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsNumber(value);
    }
};

template <>
struct Stack<double>
{
    static void pushLua(EContext *ctx, double value)
    {
        lua_pushnumber((lua_State *)(ctx->GetState()), (lua_Number)value);
    }

    static JSValue pushJS(EContext *ctx, double value)
    {
        return JS_NewFloat64((JSContext *)(ctx->GetState()), value);
    }

    static double getLua(EContext *ctx, int ref)
    {
        return (double)luaL_checknumber((lua_State *)(ctx->GetState()), ref);
    }

    static double getJS(EContext *ctx, JSValue value)
    {
        double val;
        JS_ToFloat64((JSContext *)(ctx->GetState()), &val, value);
        return val;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_type((lua_State *)(ctx->GetState()), ref) == LUA_TNUMBER;
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsNumber(value);
    }
};

template <>
struct Stack<bool>
{
    static void pushLua(EContext *ctx, bool value)
    {
        lua_pushboolean((lua_State *)(ctx->GetState()), int(value));
    }

    static JSValue pushJS(EContext *ctx, bool value)
    {
        return JS_NewBool((JSContext *)(ctx->GetState()), value);
    }

    static bool getLua(EContext *ctx, int ref)
    {
        return (lua_toboolean((lua_State *)(ctx->GetState()), ref) == 1);
    }

    static bool getJS(EContext *ctx, JSValue value)
    {
        return JS_ToBool((JSContext *)(ctx->GetState()), value) == 1;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_type((lua_State *)(ctx->GetState()), ref) == LUA_TBOOLEAN;
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsBool(value);
    }
};

template <>
struct Stack<char>
{
    static void pushLua(EContext *ctx, char value)
    {
        lua_pushlstring((lua_State *)(ctx->GetState()), &value, 1);
    }

    static JSValue pushJS(EContext *ctx, char value)
    {
        return JS_NewStringLen((JSContext *)(ctx->GetState()), &value, 1);
    }

    static char getLua(EContext *ctx, int ref)
    {
        return luaL_checkstring((lua_State *)(ctx->GetState()), ref)[0];
    }

    static char getJS(EContext *ctx, JSValue value)
    {
        size_t len;
        auto val = JS_ToCStringLen((JSContext *)(ctx->GetState()), &len, value);
        char out = '\0';
        if (len != 0)
            out = (char)(val[0]);
        JS_FreeCString((JSContext *)(ctx->GetState()), val);
        return out;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_type((lua_State *)(ctx->GetState()), ref) == LUA_TBOOLEAN;
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsBool(value);
    }
};

template <>
struct Stack<char const *>
{
    static void pushLua(EContext *ctx, char const *value)
    {
        if (value)
            lua_pushstring((lua_State *)(ctx->GetState()), value);
        else
            lua_pushnil((lua_State *)(ctx->GetState()));
    }

    static JSValue pushJS(EContext *ctx, char const *value)
    {
        return JS_NewString((JSContext *)(ctx->GetState()), value);
    }

    static char const *getLua(EContext *ctx, int ref)
    {
        return lua_isnil((lua_State *)(ctx->GetState()), ref) ? nullptr : luaL_checkstring((lua_State *)(ctx->GetState()), ref);
    }

    static char const *getJS(EContext *ctx, JSValue value)
    {
        // Not getting the value due to it's necessity to free after getting the value, leaving memory leak.
        return nullptr;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_type((lua_State *)(ctx->GetState()), ref) == LUA_TSTRING;
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsString(value);
    }
};

template <>
struct Stack<std::string>
{
    static void pushLua(EContext *ctx, std::string value)
    {
        lua_pushlstring((lua_State *)(ctx->GetState()), value.data(), value.size());
    }

    static JSValue pushJS(EContext *ctx, std::string value)
    {
        return JS_NewStringLen((JSContext *)(ctx->GetState()), value.data(), value.size());
    }

    static std::string getLua(EContext *ctx, int ref)
    {
        size_t len;
        if (lua_type((lua_State *)(ctx->GetState()), ref) == LUA_TSTRING)
        {
            const char *str = lua_tolstring((lua_State *)(ctx->GetState()), ref, &len);
            return std::string(str, len);
        }

        lua_State *L = (lua_State *)(ctx->GetState());

        lua_pushvalue(L, ref);
        const char *str = lua_tolstring(L, -1, &len);
        std::string s(str, len);
        lua_pop(L, 1);
        return s;
    }

    static std::string getJS(EContext *ctx, JSValue value)
    {
        size_t len;
        const char *str = JS_ToCStringLen((JSContext *)(ctx->GetState()), &len, value);
        std::string s(str, len);
        JS_FreeCString((JSContext *)(ctx->GetState()), str);
        return s;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_type((lua_State *)(ctx->GetState()), ref) == LUA_TSTRING;
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsString(value);
    }
};

template <class T>
struct Stack<std::optional<T>>
{
    static void pushLua(EContext *ctx, std::optional<T> value)
    {
        if (value.has_value())
        {
            Stack<T>::pushLua(ctx, *value);
        }
        else
        {
            Stack<nullptr_t>::pushLua(ctx);
        }
    }

    static JSValue pushJS(EContext *ctx, std::optional<T> value)
    {
        if (value.has_value())
        {
            return Stack<T>::pushJS(ctx, *value);
        }
        else
        {
            return Stack<nullptr_t>::pushJS(ctx);
        }
    }

    static std::optional<T> getLua(EContext *ctx, int ref)
    {
        if (Stack<nullptr_t>::isLuaInstance(ctx, ref))
            return std::nullopt;
        return Stack<T>::getLua(ctx, ref);
    }

    static std::optional<T> getJS(EContext *ctx, JSValue value)
    {
        if (Stack<nullptr_t>::isJSInstance(ctx, value))
            return std::nullopt;
        return Stack<T>::getJS(ctx, value);
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_isnil((lua_State *)(ctx->GetState()), ref) || Stack<T>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsNull(value) || Stack<T>::isJSInstance(ctx, value);
    }
};

template <class T>
struct Stack<std::vector<T>>
{
    static void pushLua(EContext *ctx, std::vector<T> value)
    {
        lua_State *L = (lua_State *)(ctx->GetState());

        lua_createtable(L, (int)(value.size()), 0);
        for (size_t i = 0; i < value.size(); i++)
        {
            lua_pushinteger(L, (lua_Integer)(i + 1));
            Stack<T>::pushLua(ctx, value[i]);
            lua_settable(L, -3);
        }
    }

    static JSValue pushJS(EContext *ctx, std::vector<T> value)
    {
        JSContext *ct = (JSContext *)(ctx->GetState());
        JSValue arr = JS_NewArray(ct);

        for (size_t i = 0; i < value.size(); i++)
        {
            JS_SetPropertyUint32(ct, arr, i, Stack<T>::pushJS(ctx, value[i]));
        }

        return arr;
    }

    static std::vector<T> getLua(EContext *ctx, int ref)
    {
        std::vector<T> v;

        lua_State *L = (lua_State *)(ctx->GetState());
        if (!lua_istable(L, ref))
            return v;

        v.reserve((std::size_t)(get_length(L, ref)));

        int absidx = lua_absindex(L, ref);
        lua_pushnil(L);
        while (lua_next(L, absidx) != 0)
        {
            v.push_back(Stack<T>::getLua(ctx, -1));
            lua_pop(L, 1);
        }

        return v;
    }

    static std::vector<T> getJS(EContext *ctx, JSValue value)
    {
        std::vector<T> v;

        JSContext *L = (JSContext *)(ctx->GetState());
        if (!JS_IsArray(L, value))
            return v;

        uint32_t len = JSGetArrayLength(L, value);

        v.reserve((std::size_t)(len));

        for (uint32_t i = 0; i < len; i++)
        {
            JSValue item = JS_GetPropertyUint32(L, value, i);
            v.push_back(Stack<T>::getJS(ctx, item));
            JS_FreeValue(L, item);
        }

        return v;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_istable((lua_State *)(ctx->GetState()), ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsArray((JSContext *)(ctx->GetState()), value);
    }
};

template <class T>
struct Stack<std::list<T>>
{
    static void pushLua(EContext *ctx, std::list<T> value)
    {
        lua_State *L = (lua_State *)(ctx->GetState());

        lua_createtable(L, (int)(value.size()), 0);
        typename std::list<T>::const_iterator item = value.begin();

        for (size_t i = 0; i < value.size(); i++)
        {
            lua_pushinteger(L, (lua_Integer)(i + 1));
            Stack<T>::pushLua(ctx, *item);
            lua_settable(L, -3);
            ++item;
        }
    }

    static JSValue pushJS(EContext *ctx, std::list<T> value)
    {
        JSContext *ct = (JSContext *)(ctx->GetState());
        JSValue arr = JS_NewArray(ct);

        typename std::list<T>::const_iterator item = value.begin();
        for (size_t i = 0; i < value.size(); i++)
        {
            JS_SetPropertyUint32(ct, arr, i, Stack<T>::pushJS(ctx, *item));
            ++item;
        }

        return arr;
    }

    static std::list<T> getLua(EContext *ctx, int ref)
    {
        std::list<T> v;

        lua_State *L = (lua_State *)(ctx->GetState());
        if (!lua_istable(L, ref))
            return v;

        int absidx = lua_absindex(L, ref);
        lua_pushnil(L);
        while (lua_next(L, absidx) != 0)
        {
            v.push_back(Stack<T>::getLua(ctx, -1));
            lua_pop(L, 1);
        }

        return v;
    }

    static std::list<T> getJS(EContext *ctx, JSValue value)
    {
        std::list<T> v;

        JSContext *L = (JSContext *)(ctx->GetState());
        if (!JS_IsArray(L, value))
            return v;

        uint32_t len = JSGetArrayLength(L, value);

        for (uint32_t i = 0; i < len; i++)
        {
            JSValue item = JS_GetPropertyUint32(L, value, i);
            v.push_back(Stack<T>::getJS(ctx, item));
            JS_FreeValue(L, item);
        }

        return v;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_istable((lua_State *)(ctx->GetState()), ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsArray((JSContext *)(ctx->GetState()), value);
    }
};

template <class T, size_t s>
struct Stack<std::array<T, s>>
{
    static void pushLua(EContext *ctx, std::array<T, s> value)
    {
        lua_State *L = (lua_State *)(ctx->GetState());

        lua_createtable(L, (int)(s), 0);

        for (size_t i = 0; i < s; i++)
        {
            lua_pushinteger(L, (lua_Integer)(i + 1));
            Stack<T>::pushLua(ctx, value[i]);
            lua_settable(L, -3);
        }
    }

    static JSValue pushJS(EContext *ctx, std::array<T, s> value)
    {
        JSContext *ct = (JSContext *)(ctx->GetState());
        JSValue arr = JS_NewArray(ct);

        for (size_t i = 0; i < s; i++)
        {
            JS_SetPropertyUint32(ct, arr, i, Stack<T>::pushJS(ctx, value[i]));
        }

        return arr;
    }

    static std::array<T, s> getLua(EContext *ctx, int ref)
    {
        std::array<T, s> v;

        lua_State *L = (lua_State *)(ctx->GetState());
        if (!lua_istable(L, ref))
            return v;
        if (get_length(L, ref) != s)
            return v;

        int absidx = lua_absindex(L, ref);
        lua_pushnil(L);
        int arrIdx = 0;
        while (lua_next(L, absidx) != 0)
        {
            v[arrIdx] = Stack<T>::getLua(ctx, -1);
            lua_pop(L, 1);
            ++arrIdx;
        }

        return v;
    }

    static std::array<T, s> getJS(EContext *ctx, JSValue value)
    {
        std::array<T, s> v;

        JSContext *L = (JSContext *)(ctx->GetState());
        if (!JS_IsArray(L, value))
            return v;

        uint32_t len = JSGetArrayLength(L, value);

        if (len != s)
            return v;

        for (uint32_t i = 0; i < len; i++)
        {
            JSValue item = JS_GetPropertyUint32(L, value, i);
            v[i] = Stack<T>::getJS(ctx, item);
            JS_FreeValue(L, item);
        }

        return v;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_istable((lua_State *)(ctx->GetState()), ref) && get_length((lua_State *)(ctx->GetState()), ref) == s;
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        if (!JS_IsArray((JSContext *)(ctx->GetState()), value))
            return false;
        return JSGetArrayLength((JSContext *)(ctx->GetState()), value) == s;
    }
};

template <class K, class V>
struct Stack<std::map<K, V>>
{
    typedef std::map<K, V> M;

    static void pushLua(EContext *ctx, M value)
    {
        lua_State *L = (lua_State *)(ctx->GetState());

        lua_createtable(L, (int)(value.size()), 0);
        typedef typename M::const_iterator ConstIter;

        for (ConstIter it = value.begin(); it != value.end(); ++it)
        {
            Stack<K>::pushLua(ctx, it->first);
            Stack<V>::pushLua(ctx, it->second);
            lua_settable(L, -3);
        }
    }

    static JSValue pushJS(EContext *ctx, M value)
    {
        JSContext *ct = (JSContext *)(ctx->GetState());
        JSValue arr = JS_NewArray(ct);

        typedef typename M::const_iterator ConstIter;

        for (ConstIter it = value.begin(); it != value.end(); ++it)
        {
            JSAtom at = JS_ValueToAtom(ct, Stack<K>::pushJS(ctx, it->first));
            JS_SetProperty(ct, arr, at, Stack<V>::pushJS(ctx, it->second));
            JS_FreeAtom(ct, at);
        }

        return arr;
    }

    static M getLua(EContext *ctx, int ref)
    {
        M v;

        lua_State *L = (lua_State *)(ctx->GetState());
        if (!lua_istable(L, ref))
            return v;

        int absidx = lua_absindex(L, ref);
        lua_pushnil(L);
        while (lua_next(L, absidx) != 0)
        {
            v.emplace(Stack<K>::getLua(ctx, -2), Stack<V>::getLua(ctx, -1));
            lua_pop(L, 1);
        }

        return v;
    }

    static M getJS(EContext *ctx, JSValue value)
    {
        M v;

        JSContext *L = (JSContext *)(ctx->GetState());
        if (!JS_IsObject(value))
            return v;

        JSPropertyEnum *properties;
        uint32_t propCount;

        if (JS_GetOwnPropertyNames(L, &properties, &propCount, value, JS_GPN_ENUM_ONLY | JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK) < 0)
        {
            return v;
        }

        for (uint32_t i = 0; i < propCount; i++)
        {
            JSValue propVal = JS_AtomToValue(L, properties[i].atom);
            JSValue property = JS_GetProperty(L, value, properties[i].atom);

            v.emplace(Stack<K>::getJS(ctx, propVal), Stack<V>::getJS(ctx, property));

            JS_FreeValue(L, propVal);
            JS_FreeValue(L, property);
        }

        JS_FreePropertyEnum(L, properties, propCount);

        return v;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_istable((lua_State *)(ctx->GetState()), ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsObject(value);
    }
};

template <class K, class V>
struct Stack<std::unordered_map<K, V>>
{
    typedef std::unordered_map<K, V> M;

    static void pushLua(EContext *ctx, M value)
    {
        lua_State *L = (lua_State *)(ctx->GetState());

        lua_createtable(L, (int)(value.size()), 0);
        typedef typename M::const_iterator ConstIter;

        for (ConstIter it = value.begin(); it != value.end(); ++it)
        {
            Stack<K>::pushLua(ctx, it->first);
            Stack<V>::pushLua(ctx, it->second);
            lua_settable(L, -3);
        }
    }

    static JSValue pushJS(EContext *ctx, M value)
    {
        JSContext *ct = (JSContext *)(ctx->GetState());
        JSValue arr = JS_NewArray(ct);

        typedef typename M::const_iterator ConstIter;

        for (ConstIter it = value.begin(); it != value.end(); ++it)
            JS_SetProperty(ct, arr, Stack<K>::pushJS(ctx, it->first), Stack<V>::pushJS(ctx, it->second));

        return arr;
    }

    static M getLua(EContext *ctx, int ref)
    {
        M v;

        lua_State *L = (lua_State *)(ctx->GetState());
        if (!lua_istable(L, ref))
            return v;

        int absidx = lua_absindex(L, ref);
        lua_pushnil(L);
        while (lua_next(L, absidx) != 0)
        {
            v.emplace(Stack<K>::getLua(ctx, -2), Stack<V>::getLua(ctx, -1));
            lua_pop(L, 1);
        }

        return v;
    }

    static M getJS(EContext *ctx, JSValue value)
    {
        M v;

        JSContext *L = (JSContext *)(ctx->GetState());
        if (!JS_IsObject(value))
            return v;

        JSPropertyEnum *properties;
        uint32_t propCount;

        if (JS_GetOwnPropertyNames(L, &properties, &propCount, value, JS_GPN_ENUM_ONLY | JS_GPN_STRING_MASK | JS_GPN_SYMBOL_MASK) < 0)
        {
            return v;
        }

        for (uint32_t i = 0; i < propCount; i++)
        {
            JSValue propVal = JS_AtomToValue(L, properties[i].atom);
            JSValue property = JS_GetProperty(L, value, properties[i].atom);

            v.emplace(Stack<K>::getJS(ctx, propVal), Stack<V>::getJS(ctx, property));

            JS_FreeValue(L, propVal);
            JS_FreeValue(L, property);
        }

        JS_FreePropertyEnum(L, properties, propCount);

        return v;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_istable((lua_State *)(ctx->GetState()), ref);
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        return JS_IsObject(value);
    }
};

template <class T1, class T2>
struct Stack<std::pair<T1, T2>>
{
    static void pushLua(EContext *ctx, std::pair<T1, T2> value)
    {
        lua_State *L = (lua_State *)(ctx->GetState());

        lua_createtable(L, 2, 0);
        lua_pushinteger(L, (lua_Integer)1);
        Stack<T1>::pushLua(ctx, value.first);
        lua_settable(L, -3);
        lua_pushinteger(L, (lua_Integer)2);
        Stack<T2>::pushLua(ctx, value.second);
        lua_settable(L, -3);
    }

    static JSValue pushJS(EContext *ctx, std::pair<T1, T2> value)
    {
        JSContext *ct = (JSContext *)(ctx->GetState());
        JSValue arr = JS_NewArray(ct);

        JS_SetPropertyUint32(ct, arr, 0, Stack<T1>::pushJS(ctx, value.first));
        JS_SetPropertyUint32(ct, arr, 1, Stack<T2>::pushJS(ctx, value.second));

        return arr;
    }

    static std::pair<T1, T2> getLua(EContext *ctx, int ref)
    {
        std::pair<T1, T2> v;

        lua_State *L = (lua_State *)(ctx->GetState());
        if (!lua_istable(L, ref))
            return v;
        if (get_length(L, ref) != 2)
            return v;

        int absidx = lua_absindex(L, ref);
        lua_pushnil(L);

        if (lua_next(L, absidx) != 0)
        {
            v.first = Stack<T1>::getLua(ctx, -1);
            lua_pop(L, 1);
        }

        if (lua_next(L, absidx) != 0)
        {
            v.second = Stack<T2>::getLua(ctx, -1);
            lua_pop(L, 1);
        }

        lua_next(L, absidx);

        return v;
    }

    static std::pair<T1, T2> getJS(EContext *ctx, JSValue value)
    {
        std::pair<T1, T2> v;

        JSContext *L = (JSContext *)(ctx->GetState());
        if (!JS_IsArray(L, value))
            return v;

        uint32_t len = JSGetArrayLength(L, value);
        if (len != 2)
            return v;

        JSValue item = JS_GetPropertyUint32(L, value, 0);
        v.first = Stack<T1>::getJS(ctx, item);
        JS_FreeValue(L, item);

        JSValue item2 = JS_GetPropertyUint32(L, value, 1);
        v.second = Stack<T2>::getJS(ctx, item2);
        JS_FreeValue(L, item2);

        return v;
    }

    static bool isLuaInstance(EContext *ctx, int ref)
    {
        return lua_istable((lua_State *)(ctx->GetState()), ref) && get_length((lua_State *)(ctx->GetState()), ref) == 2;
    }

    static bool isJSInstance(EContext *ctx, JSValue value)
    {
        if (!JS_IsArray((JSContext *)(ctx->GetState()), value))
            return false;
        return JSGetArrayLength((JSContext *)(ctx->GetState()), value) == 2;
    }
};

template<>
struct Stack<ClassData*>
{
    static void pushLua(EContext* ctx, ClassData* value)
    {
        if(ShouldDeleteOnGC(value)) {
            value = new ClassData(*value);
            MarkDeleteOnGC(value);
        }

        auto L = ctx->GetLuaState();
        ClassData **udata = (ClassData **)lua_newuserdata(L, sizeof(ClassData *));
        *udata = value;

        luaL_getmetatable(L, value->GetClassname().c_str());
        lua_setmetatable(L, -2);
    }

    static JSValue pushJS(EContext* ctx, ClassData* value)
    {
        if(ShouldDeleteOnGC(value)) {
            value = new ClassData(*value);
            MarkDeleteOnGC(value);
        }
        
        auto L = ctx->GetJSState();
        JSClassID &id = *(ctx->GetClassID(value->GetClassname()));
        auto proto = ctx->GetClassPrototype(value->GetClassname());
        JS_SetClassProto(L, id, proto);
        auto ret = JS_NewObjectProtoClass(L, JS_GetClassProto(L, id), id);

        if (JS_IsException(ret))
        {
            JS_FreeValue(L, ret);
            return JS_EXCEPTION;
        }
        else
        {
            JS_SetOpaque(ret, (void *)value);
        }

        return ret;
    }

    static ClassData* getLua(EContext* ctx, int ref)
    {
        ClassData** data = (ClassData**)lua_touserdata(ctx->GetLuaState(), ref);
        return data ? *data : nullptr;
    }

    static ClassData* getJS(EContext* ctx, JSValue value)
    {
        return (ClassData*)JS_GetOpaque(value, JS_GetClassID(value));
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_isuserdata(ctx->GetLuaState(), ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_GetOpaque(value, JS_GetClassID(value)) != nullptr;
    }
};

#endif