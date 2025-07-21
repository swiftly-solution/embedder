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
#include <cstdint>
#include <stdint.h>

#include "Context.h"
#include "Helpers.h"
#include "GarbageCollector.h"
#include "dotnet/invoker.h"
#include "dotnet/host.h"
#include "engine/classes.h"

template<typename T>
struct is_vector : std::false_type {};

template<typename V, typename...Args>
struct is_vector<std::vector<V, Args...>> : std::true_type {};

template<typename T>
struct is_map : std::false_type {};

template<typename K, typename V, typename...Args>
struct is_map<std::map<K, V, Args...>> : std::true_type {};

template<typename K, typename V, typename...Args>
struct is_map<std::unordered_map<K, V, Args...>> : std::true_type {};

template <class T>
struct Stack;

template <>
struct Stack<void>
{
    static void pushLua(EContext*) {}
    static void pushJS(EContext*) {}
};

template <>
struct Stack<EContext*>
{
    static EContext* getLua(EContext* ctx, int) { return ctx; }
    static EContext* getJS(EContext* ctx, JSValue) { return ctx; }
    static EContext* getDotnet(EContext* ctx, CallContext*, int) { return ctx; }
};

template <>
struct Stack<nullptr_t>
{
    static void pushLua(EContext* ctx, nullptr_t value = nullptr)
    {
        lua_pushnil((lua_State*)(ctx->GetState()));
    }

    static JSValue pushJS(EContext* ctx, nullptr_t value = nullptr)
    {
        return JS_NULL;
    }

    static nullptr_t pushRawDotnet(EContext* ctx, CallContext* context, nullptr_t value = nullptr)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, nullptr_t value = nullptr, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(0);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), 0);
            context->PushArgument(value);
        }
    }

    static nullptr_t getLua(EContext* ctx, int ref)
    {
        return nullptr;
    }

    static nullptr_t getJS(EContext* ctx, JSValue value)
    {
        return nullptr;
    }

    static nullptr_t getDotnet(EContext* ctx, CallContext*, int)
    {
        return nullptr;
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_isnil((lua_State*)(ctx->GetState()), ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsNull(value);
    }

    static bool isDotnetInstance(EContext* ctx, CallContext* context, int argument)
    {
        if (argument == -1) return context->GetReturnType() == 0;
        else context->GetArgumentType(argument) == 0;
    }
};

template <>
struct Stack<int>
{
    static void pushLua(EContext* ctx, int value)
    {
        lua_pushinteger((lua_State*)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext* ctx, int value)
    {
        return JS_NewInt32((JSContext*)(ctx->GetState()), value);
    }

    static int pushRawDotnet(EContext* ctx, CallContext* context, int value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, int value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(int)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(int)]);
            context->PushArgument(value);
        }
    }

    static int getLua(EContext* ctx, int ref)
    {
        return (int)luaL_checkinteger((lua_State*)(ctx->GetState()), ref);
    }

    static int getJS(EContext* ctx, JSValue value)
    {
        int val;
        JS_ToInt32((JSContext*)(ctx->GetState()), &val, value);
        return val;
    }

    static int getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(int*)value;
    }

    static int getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<int>();
        else return context->GetArgument<int>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        if (lua_type((lua_State*)(ctx->GetState()), ref) != LUA_TNUMBER)
            return false;

        int isNumber;
        lua_tointegerx((lua_State*)(ctx->GetState()), ref, &isNumber);
        return isNumber;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsNumber(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(int)];
        else return context->GetArgumentType(index) == typesMap[typeid(int)];
    }
};

template <>
struct Stack<long int>
{
    static void pushLua(EContext* ctx, long int value)
    {
        lua_pushinteger((lua_State*)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext* ctx, long int value)
    {
        return JS_NewInt32((JSContext*)(ctx->GetState()), value);
    }

    static long int pushRawDotnet(EContext* ctx, CallContext* context, long int value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, long int value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(int)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(int)]);
            context->PushArgument(value);
        }
    }

    static long int getLua(EContext* ctx, int ref)
    {
        return (long int)luaL_checkinteger((lua_State*)(ctx->GetState()), ref);
    }

    static long int getJS(EContext* ctx, JSValue value)
    {
        int val;
        JS_ToInt32((JSContext*)(ctx->GetState()), &val, value);
        return (long int)val;
    }

    static long int getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(long int*)value;
    }

    static long int getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<long int>();
        else return context->GetArgument<long int>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        if (lua_type((lua_State*)(ctx->GetState()), ref) != LUA_TNUMBER)
            return false;

        int isNumber;
        lua_tointegerx((lua_State*)(ctx->GetState()), ref, &isNumber);
        return isNumber;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsNumber(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(int)];
        else return context->GetArgumentType(index) == typesMap[typeid(int)];
    }
};

template <>
struct Stack<unsigned int>
{
    static void pushLua(EContext* ctx, unsigned int value)
    {
        lua_pushinteger((lua_State*)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext* ctx, unsigned int value)
    {
        return JS_NewUint32((JSContext*)(ctx->GetState()), value);
    }

    static unsigned int pushRawDotnet(EContext* ctx, CallContext* context, unsigned int value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, unsigned int value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(unsigned int)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(unsigned int)]);
            context->PushArgument(value);
        }
    }

    static unsigned int getLua(EContext* ctx, int ref)
    {
        return (unsigned int)luaL_checkinteger((lua_State*)(ctx->GetState()), ref);
    }

    static unsigned int getJS(EContext* ctx, JSValue value)
    {
        unsigned int val;
        JS_ToUint32((JSContext*)(ctx->GetState()), &val, value);
        return val;
    }

    static unsigned int getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(unsigned int*)value;
    }

    static unsigned int getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<unsigned int>();
        else return context->GetArgument<unsigned int>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(unsigned int)];
        else return context->GetArgumentType(index) == typesMap[typeid(unsigned int)];
    }
};

template <>
struct Stack<long unsigned int>
{
    static void pushLua(EContext* ctx, long unsigned int value)
    {
        lua_pushinteger((lua_State*)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext* ctx, long unsigned int value)
    {
        return JS_NewUint32((JSContext*)(ctx->GetState()), value);
    }

    static long unsigned int pushRawDotnet(EContext* ctx, CallContext* context, long unsigned int value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, long unsigned int value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(unsigned int)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(unsigned int)]);
            context->PushArgument(value);
        }
    }

    static long unsigned int getLua(EContext* ctx, int ref)
    {
        return (long unsigned int)luaL_checkinteger((lua_State*)(ctx->GetState()), ref);
    }

    static long unsigned int getJS(EContext* ctx, JSValue value)
    {
        unsigned int val;
        JS_ToUint32((JSContext*)(ctx->GetState()), &val, value);
        return (long unsigned int)val;
    }

    static long unsigned int getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(long unsigned int*)value;
    }

    static long unsigned int getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<long unsigned int>();
        else return context->GetArgument<long unsigned int>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(unsigned int)];
        else return context->GetArgumentType(index) == typesMap[typeid(unsigned int)];
    }
};

template <>
struct Stack<uint8_t>
{
    static void pushLua(EContext* ctx, uint8_t value)
    {
        lua_pushinteger((lua_State*)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext* ctx, uint8_t value)
    {
        return JS_NewUint32((JSContext*)(ctx->GetState()), value);
    }

    static uint8_t pushRawDotnet(EContext* ctx, CallContext* context, uint8_t value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, uint8_t value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(uint8_t)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(uint8_t)]);
            context->PushArgument(value);
        }
    }

    static uint8_t getLua(EContext* ctx, int ref)
    {
        return (uint8_t)luaL_checkinteger((lua_State*)(ctx->GetState()), ref);
    }

    static uint8_t getJS(EContext* ctx, JSValue value)
    {
        unsigned int val;
        JS_ToUint32((JSContext*)(ctx->GetState()), &val, value);
        return (uint8_t)val;
    }

    static uint8_t getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(uint8_t*)value;
    }

    static uint8_t getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<uint8_t>();
        else return context->GetArgument<uint8_t>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(uint8_t)];
        else return context->GetArgumentType(index) == typesMap[typeid(uint8_t)];
    }
};

template <>
struct Stack<int16_t>
{
    static void pushLua(EContext* ctx, int16_t value)
    {
        lua_pushinteger((lua_State*)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext* ctx, int16_t value)
    {
        return JS_NewInt32((JSContext*)(ctx->GetState()), value);
    }

    static int16_t pushRawDotnet(EContext* ctx, CallContext* context, int16_t value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, int16_t value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(short)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(short)]);
            context->PushArgument(value);
        }
    }

    static int16_t getLua(EContext* ctx, int ref)
    {
        return (int16_t)luaL_checkinteger((lua_State*)(ctx->GetState()), ref);
    }

    static int16_t getJS(EContext* ctx, JSValue value)
    {
        int val;
        JS_ToInt32((JSContext*)(ctx->GetState()), &val, value);
        return (int16_t)val;
    }

    static int16_t getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(int16_t*)value;
    }

    static int16_t getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<int16_t>();
        else return context->GetArgument<int16_t>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(short)];
        else return context->GetArgumentType(index) == typesMap[typeid(short)];
    }
};

template <>
struct Stack<uint16_t>
{
    static void pushLua(EContext* ctx, uint16_t value)
    {
        lua_pushinteger((lua_State*)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext* ctx, uint16_t value)
    {
        return JS_NewUint32((JSContext*)(ctx->GetState()), value);
    }

    static uint16_t pushRawDotnet(EContext* ctx, CallContext* context, uint16_t value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, uint16_t value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(unsigned short)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(unsigned short)]);
            context->PushArgument(value);
        }
    }

    static uint16_t getLua(EContext* ctx, int ref)
    {
        return (uint16_t)luaL_checkinteger((lua_State*)(ctx->GetState()), ref);
    }

    static uint16_t getJS(EContext* ctx, JSValue value)
    {
        unsigned int val;
        JS_ToUint32((JSContext*)(ctx->GetState()), &val, value);
        return (uint16_t)val;
    }

    static uint16_t getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(uint16_t*)value;
    }

    static uint16_t getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<uint16_t>();
        else return context->GetArgument<uint16_t>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(unsigned short)];
        else return context->GetArgumentType(index) == typesMap[typeid(unsigned short)];
    }
};

template <>
struct Stack<int8_t>
{
    static void pushLua(EContext* ctx, int8_t value)
    {
        lua_pushinteger((lua_State*)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext* ctx, int8_t value)
    {
        return JS_NewUint32((JSContext*)(ctx->GetState()), value);
    }

    static int8_t pushRawDotnet(EContext* ctx, CallContext* context, int8_t value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, int8_t value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(int8_t)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(int8_t)]);
            context->PushArgument(value);
        }
    }

    static int8_t getLua(EContext* ctx, int ref)
    {
        return (int8_t)luaL_checkinteger((lua_State*)(ctx->GetState()), ref);
    }

    static int8_t getJS(EContext* ctx, JSValue value)
    {
        unsigned int val;
        JS_ToUint32((JSContext*)(ctx->GetState()), &val, value);
        return (int8_t)val;
    }

    static int8_t getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(int8_t*)value;
    }

    static int8_t getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<int8_t>();
        else return context->GetArgument<int8_t>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(int8_t)];
        else return context->GetArgumentType(index) == typesMap[typeid(int8_t)];
    }
};

template<>
struct Stack<long long int>
{
    static void pushLua(EContext* ctx, long long int value)
    {
        lua_pushinteger((lua_State*)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext* ctx, long long int value)
    {
        return JS_NewInt64((JSContext*)(ctx->GetState()), value);
    }

    static long long int pushRawDotnet(EContext* ctx, CallContext* context, long long int value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, long long int value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(int64_t)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(int64_t)]);
            context->PushArgument(value);
        }
    }

    static long long int getLua(EContext* ctx, int ref)
    {
        return (long long int)luaL_checkinteger((lua_State*)(ctx->GetState()), ref);
    }

    static long long int getJS(EContext* ctx, JSValue value)
    {
        int64_t val;
        JS_ToInt64((JSContext*)(ctx->GetState()), &val, value);
        return (long long int)val;
    }

    static long long int getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(long long int*)value;
    }

    static long long int getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<long long int>();
        else return context->GetArgument<long long int>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(int64_t)];
        else return context->GetArgumentType(index) == typesMap[typeid(int64_t)];
    }
};

template<>
struct Stack<long long unsigned int>
{
    static void pushLua(EContext* ctx, long long unsigned int value)
    {
        lua_pushinteger((lua_State*)(ctx->GetState()), (lua_Integer)value);
    }

    static JSValue pushJS(EContext* ctx, long long unsigned int value)
    {
        return JS_NewBigUint64((JSContext*)(ctx->GetState()), value);
    }

    static long long unsigned int pushRawDotnet(EContext* ctx, CallContext* context, long long unsigned int value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, long long unsigned int value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(uint64_t)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(uint64_t)]);
            context->PushArgument(value);
        }
    }

    static long long unsigned int getLua(EContext* ctx, int ref)
    {
        return (long long unsigned int)luaL_checkinteger((lua_State*)(ctx->GetState()), ref);
    }

    static long long unsigned int getJS(EContext* ctx, JSValue value)
    {
        uint64_t val;
        JS_ToBigUint64((JSContext*)(ctx->GetState()), &val, value);
        return (long long unsigned int)val;
    }

    static long long unsigned int getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(long long unsigned int*)value;
    }

    static long long unsigned int getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<long long unsigned int>();
        else return context->GetArgument<long long unsigned int>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return Stack<int>::isLuaInstance(ctx, ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return Stack<int>::isJSInstance(ctx, value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(uint64_t)];
        else return context->GetArgumentType(index) == typesMap[typeid(uint64_t)];
    }
};

template <>
struct Stack<float>
{
    static void pushLua(EContext* ctx, float value)
    {
        lua_pushnumber((lua_State*)(ctx->GetState()), (lua_Number)value);
    }

    static JSValue pushJS(EContext* ctx, float value)
    {
        return JS_NewFloat64((JSContext*)(ctx->GetState()), (double)value);
    }

    static float pushRawDotnet(EContext* ctx, CallContext* context, float value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, float value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(float)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(float)]);
            context->PushArgument(value);
        }
    }

    static float getLua(EContext* ctx, int ref)
    {
        return (float)luaL_checknumber((lua_State*)(ctx->GetState()), ref);
    }

    static float getJS(EContext* ctx, JSValue value)
    {
        double val;
        JS_ToFloat64((JSContext*)(ctx->GetState()), &val, value);
        return (float)val;
    }

    static float getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(float*)value;
    }

    static float getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<float>();
        else return context->GetArgument<float>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_type((lua_State*)(ctx->GetState()), ref) == LUA_TNUMBER;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsNumber(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(float)];
        else return context->GetArgumentType(index) == typesMap[typeid(float)];
    }
};

template <>
struct Stack<double>
{
    static void pushLua(EContext* ctx, double value)
    {
        lua_pushnumber((lua_State*)(ctx->GetState()), (lua_Number)value);
    }

    static JSValue pushJS(EContext* ctx, double value)
    {
        return JS_NewFloat64((JSContext*)(ctx->GetState()), value);
    }

    static double pushRawDotnet(EContext* ctx, CallContext* context, double value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, double value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(double)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(double)]);
            context->PushArgument(value);
        }
    }

    static double getLua(EContext* ctx, int ref)
    {
        return (double)luaL_checknumber((lua_State*)(ctx->GetState()), ref);
    }

    static double getJS(EContext* ctx, JSValue value)
    {
        double val;
        JS_ToFloat64((JSContext*)(ctx->GetState()), &val, value);
        return val;
    }

    static double getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(double*)value;
    }

    static double getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<double>();
        else return context->GetArgument<double>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_type((lua_State*)(ctx->GetState()), ref) == LUA_TNUMBER;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsNumber(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(double)];
        else return context->GetArgumentType(index) == typesMap[typeid(double)];
    }
};

template <>
struct Stack<bool>
{
    static void pushLua(EContext* ctx, bool value)
    {
        lua_pushboolean((lua_State*)(ctx->GetState()), int(value));
    }

    static JSValue pushJS(EContext* ctx, bool value)
    {
        return JS_NewBool((JSContext*)(ctx->GetState()), value);
    }

    static bool pushRawDotnet(EContext* ctx, CallContext* context, bool value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, bool value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(bool)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(bool)]);
            context->PushArgument(value);
        }
    }

    static bool getLua(EContext* ctx, int ref)
    {
        return (lua_toboolean((lua_State*)(ctx->GetState()), ref) == 1);
    }

    static bool getJS(EContext* ctx, JSValue value)
    {
        return JS_ToBool((JSContext*)(ctx->GetState()), value) == 1;
    }

    static bool getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(bool*)value;
    }

    static bool getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<bool>();
        else return context->GetArgument<bool>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_type((lua_State*)(ctx->GetState()), ref) == LUA_TBOOLEAN;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsBool(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(bool)];
        else return context->GetArgumentType(index) == typesMap[typeid(bool)];
    }
};

template <>
struct Stack<char>
{
    static void pushLua(EContext* ctx, char value)
    {
        lua_pushlstring((lua_State*)(ctx->GetState()), &value, 1);
    }

    static JSValue pushJS(EContext* ctx, char value)
    {
        return JS_NewStringLen((JSContext*)(ctx->GetState()), &value, 1);
    }

    static char* pushRawDotnet(EContext* ctx, CallContext* context, char value)
    {
        char* string_buf = (char*)DotnetAllocateContextPointer(sizeof(char), 2);
        std::string tmp(1, value);
        strcpy(string_buf, tmp.c_str());
        return string_buf;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, char value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(std::string)]);
            context->SetResult(std::string(1, value));
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(std::string)]);
            context->PushArgument(std::string(1, value));
        }
    }

    static char getLua(EContext* ctx, int ref)
    {
        return luaL_checkstring((lua_State*)(ctx->GetState()), ref)[0];
    }

    static char getJS(EContext* ctx, JSValue value)
    {
        size_t len;
        auto val = JS_ToCStringLen((JSContext*)(ctx->GetState()), &len, value);
        char out = '\0';
        if (len != 0)
            out = (char)(val[0]);
        JS_FreeCString((JSContext*)(ctx->GetState()), val);
        return out;
    }

    static char getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(char*)value;
    }

    static char getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<char*>()[0];
        else return context->GetArgument<char*>(index)[0];
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_type((lua_State*)(ctx->GetState()), ref) == LUA_TSTRING;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsString(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(std::string)];
        else return context->GetArgumentType(index) == typesMap[typeid(std::string)];
    }
};

template <>
struct Stack<char const*>
{
    static void pushLua(EContext* ctx, char const* value)
    {
        if (value)
            lua_pushstring((lua_State*)(ctx->GetState()), value);
        else
            lua_pushnil((lua_State*)(ctx->GetState()));
    }

    static JSValue pushJS(EContext* ctx, char const* value)
    {
        return JS_NewString((JSContext*)(ctx->GetState()), value);
    }

    static char* pushRawDotnet(EContext* ctx, CallContext* context, std::string value)
    {
        char* string_buf = (char*)DotnetAllocateContextPointer(sizeof(char), value.size() + 1);
        strcpy(string_buf, value.c_str());
        return string_buf;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, char const* value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(std::string)]);
            context->SetResult(std::string(value));
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(std::string)]);
            context->PushArgument(std::string(value));
        }
    }

    static char const* getLua(EContext* ctx, int ref)
    {
        return lua_isnil((lua_State*)(ctx->GetState()), ref) ? nullptr : luaL_checkstring((lua_State*)(ctx->GetState()), ref);
    }

    static char const* getJS(EContext* ctx, JSValue value)
    {
        // Not getting the value due to it's necessity to free after getting the value, leaving memory leak.
        return nullptr;
    }

    static char const* getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(char const**)value;
    }

    static char const* getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<char const*>();
        else return context->GetArgument<char const*>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_type((lua_State*)(ctx->GetState()), ref) == LUA_TSTRING;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsString(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(std::string)];
        else return context->GetArgumentType(index) == typesMap[typeid(std::string)];
    }
};

template <>
struct Stack<std::string>
{
    static void pushLua(EContext* ctx, std::string value)
    {
        lua_pushlstring((lua_State*)(ctx->GetState()), value.data(), value.size());
    }

    static JSValue pushJS(EContext* ctx, std::string value)
    {
        return JS_NewStringLen((JSContext*)(ctx->GetState()), value.data(), value.size());
    }

    static char* pushRawDotnet(EContext* ctx, CallContext* context, std::string value)
    {
        char* string_buf = (char*)DotnetAllocateContextPointer(sizeof(char), value.size() + 1);
        strcpy(string_buf, value.c_str());
        return string_buf;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, std::string value, bool shouldReturn = false)
    {
        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(std::string)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(std::string)]);
            context->PushArgument(value);
        }
    }

    static std::string getLua(EContext* ctx, int ref)
    {
        size_t len;
        if (lua_type((lua_State*)(ctx->GetState()), ref) == LUA_TSTRING)
        {
            const char* str = lua_tolstring((lua_State*)(ctx->GetState()), ref, &len);
            return std::string(str, len);
        }

        lua_State* L = (lua_State*)(ctx->GetState());

        lua_pushvalue(L, ref);
        const char* str = lua_tolstring(L, -1, &len);
        std::string s(str, len);
        lua_pop(L, 1);
        return s;
    }

    static std::string getJS(EContext* ctx, JSValue value)
    {
        size_t len;
        const char* str = JS_ToCStringLen((JSContext*)(ctx->GetState()), &len, value);
        std::string s(str, len);
        JS_FreeCString((JSContext*)(ctx->GetState()), str);
        return s;
    }

    static std::string getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(char**)value;
    }

    static std::string getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<char*>();
        else return context->GetArgument<char*>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_type((lua_State*)(ctx->GetState()), ref) == LUA_TSTRING;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsString(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(std::string)];
        else return context->GetArgumentType(index) == typesMap[typeid(std::string)];
    }
};

template <class T>
struct Stack<std::vector<T>>
{
    static void pushLua(EContext* ctx, std::vector<T> value)
    {
        lua_State* L = (lua_State*)(ctx->GetState());

        lua_createtable(L, (int)(value.size()), 0);
        for (size_t i = 0; i < value.size(); i++)
        {
            lua_pushinteger(L, (lua_Integer)(i + 1));
            Stack<T>::pushLua(ctx, value[i]);
            lua_settable(L, -3);
        }
    }

    static JSValue pushJS(EContext* ctx, std::vector<T> value)
    {
        JSContext* ct = (JSContext*)(ctx->GetState());
        JSValue arr = JS_NewArray(ct);

        for (size_t i = 0; i < value.size(); i++)
        {
            JS_SetPropertyUint32(ct, arr, i, Stack<T>::pushJS(ctx, value[i]));
        }

        return arr;
    }

    static T* pushRawDotnet(EContext* ctx, CallContext* context, std::vector<T> value)
    {
        ArrayData* arrayData = (ArrayData*)DotnetAllocateContextPointer(sizeof(ArrayData), 1);
        if constexpr (is_map<T>::value) {
            arrayData->elements = (void**)DotnetAllocateContextPointer(sizeof(MapData*), value.size());
            arrayData->length = value.size();
            arrayData->type = 16;

            MapData** arrayPtr = (MapData**)arrayData->elements;
            for (int i = 0; i < value.size(); i++)
                arrayPtr[i] = Stack<T>::pushRawDotnet(ctx, context, value[i]);
        }
        else if constexpr (is_vector<T>::value) {
            arrayData->elements = (void**)DotnetAllocateContextPointer(sizeof(ArrayData*), value.size());
            arrayData->length = value.size();
            arrayData->type = 15;

            ArrayData** arrayPtr = (ArrayData**)arrayData->elements;
            for (int i = 0; i < value.size(); i++)
                arrayPtr[i] = Stack<T>::pushRawDotnet(ctx, context, value[i]);
        }
        else {
            arrayData->elements = (void**)DotnetAllocateContextPointer(sizeof(T), value.size());
            arrayData->length = value.size();
            arrayData->type = typesMap[typeid(T)];

            T* arrayPtr = (T*)arrayData->elements;
            for (int i = 0; i < value.size(); i++)
                arrayPtr[i] = Stack<T>::pushRawDotnet(ctx, context, value[i]);
        }
        return (T*)arrayData;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, std::vector<T> value, bool shouldReturn = false)
    {
        T* arrayPtr = pushRawDotnet(ctx, context, value);

        if (shouldReturn) {
            context->SetReturnType(15);
            context->SetResult(arrayPtr);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), 15);
            context->PushArgument(arrayPtr);
        }
    }

    static std::vector<T> getLua(EContext* ctx, int ref)
    {
        std::vector<T> v;

        lua_State* L = (lua_State*)(ctx->GetState());
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

    static std::vector<T> getJS(EContext* ctx, JSValue value)
    {
        std::vector<T> v;

        JSContext* L = (JSContext*)(ctx->GetState());
        if (!JS_IsArray(value))
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

    static std::vector<T> getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        ArrayData* arrayDatas = *(ArrayData**)value;
        std::vector<T> v;

        for (int i = 0; i < arrayDatas->length; i++)
            v.push_back(Stack<T>::getRawDotnet(ctx, context, &(arrayDatas->elements[i])));

        return v;
    }

    static std::vector<T> getDotnet(EContext* ctx, CallContext* context, int index)
    {
        std::vector<T> v;
        if (index == -1) {
            if (context->GetReturnType() != 15) return v;
            return getRawDotnet(ctx, context, context->GetResultPtr());
        }
        else {
            if (context->GetArgumentType(index) != 15) return v;
            return getRawDotnet(ctx, context, context->GetArgumentPtr(index));
        }
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_istable((lua_State*)(ctx->GetState()), ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsArray(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == 15;
        else return context->GetArgumentType(index) == 15;
    }
};

template <class K, class V>
struct Stack<std::map<K, V>>
{
    typedef std::map<K, V> M;

    static void pushLua(EContext* ctx, M value)
    {
        lua_State* L = (lua_State*)(ctx->GetState());

        lua_createtable(L, (int)(value.size()), 0);
        typedef typename M::const_iterator ConstIter;

        for (ConstIter it = value.begin(); it != value.end(); ++it)
        {
            Stack<K>::pushLua(ctx, it->first);
            Stack<V>::pushLua(ctx, it->second);
            lua_settable(L, -3);
        }
    }

    static JSValue pushJS(EContext* ctx, M value)
    {
        JSContext* ct = (JSContext*)(ctx->GetState());
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

    static MapData* pushRawDotnet(EContext* ctx, CallContext* context, M value)
    {
        MapData* mapData = (MapData*)DotnetAllocateContextPointer(sizeof(MapData), 1);
        int count = value.size();
        mapData->length = count;

        if constexpr (is_map<K>::value || is_vector<K>::value || std::is_same<std::string, K>::value) {
            mapData->keys = (void**)DotnetAllocateContextPointer(sizeof(void*), count);
            void** listKeys = (void**)mapData->keys;

            mapData->key_type = is_map<K>::value ? 16 : (is_vector<K>::value ? 15 : typesMap[typeid(K)]);

            int i = 0;
            for (auto it = value.begin(); it != value.end(); ++it)
            {
                listKeys[i] = Stack<K>::pushRawDotnet(ctx, context, it->first);
                i++;
            }
        }
        else {
            mapData->keys = (void**)DotnetAllocateContextPointer(sizeof(K), count);
            K* listKeys = (K*)mapData->keys;

            mapData->key_type = typesMap[typeid(K)];

            int i = 0;
            for (auto it = value.begin(); it != value.end(); ++it)
            {
                listKeys[i] = Stack<K>::pushRawDotnet(ctx, context, it->first);
                i++;
            }
        }

        if constexpr (is_map<V>::value || is_vector<V>::value || std::is_same<std::string, V>::value) {
            mapData->values = (void**)DotnetAllocateContextPointer(sizeof(void*), count);
            void** listValues = (void**)mapData->values;

            mapData->key_type = is_map<V>::value ? 16 : (is_vector<V>::value ? 15 : typesMap[typeid(V)]);

            int i = 0;
            for (auto it = value.begin(); it != value.end(); ++it)
            {
                listValues[i] = Stack<V>::pushRawDotnet(ctx, context, it->second);
                i++;
            }
        }
        else {
            mapData->values = (void**)DotnetAllocateContextPointer(sizeof(V), count);
            V* listValues = (V*)mapData->values;

            mapData->value_type = typesMap[typeid(V)];

            int i = 0;
            for (auto it = value.begin(); it != value.end(); ++it)
            {
                listValues[i] = Stack<V>::pushRawDotnet(ctx, context, it->second);
                i++;
            }
        }

        return mapData;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, M value, bool shouldReturn = false)
    {
        MapData* mapData = pushRawDotnet(ctx, context, value);

        if (shouldReturn) {
            context->SetReturnType(16);
            context->SetResult(mapData);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), 16);
            context->PushArgument(mapData);
        }
    }

    static M getLua(EContext* ctx, int ref)
    {
        M v;

        lua_State* L = (lua_State*)(ctx->GetState());
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

    static M getJS(EContext* ctx, JSValue value)
    {
        M v;

        JSContext* L = (JSContext*)(ctx->GetState());
        if (!JS_IsObject(value))
            return v;

        JSPropertyEnum* properties;
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

    static M getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        MapData* mapDatas = *(MapData**)value;
        M v;

        for (int i = 0; i < mapDatas->length; i++)
            v.emplace(Stack<K>::getRawDotnet(ctx, context, &(mapDatas->keys[i])), Stack<V>::getRawDotnet(ctx, context, &(mapDatas->values[i])));

        return v;
    }

    static M getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) {
            return getRawDotnet(ctx, context, context->GetResultPtr());
        }
        else {
            return getRawDotnet(ctx, context, context->GetArgumentPtr(index));
        }
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_istable((lua_State*)(ctx->GetState()), ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsObject(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == 16;
        else return context->GetArgumentType(index) == 16;
    }
};

template <class K, class V>
struct Stack<std::unordered_map<K, V>>
{
    typedef std::unordered_map<K, V> M;

    static void pushLua(EContext* ctx, M value)
    {
        lua_State* L = (lua_State*)(ctx->GetState());

        lua_createtable(L, (int)(value.size()), 0);
        typedef typename M::const_iterator ConstIter;

        for (ConstIter it = value.begin(); it != value.end(); ++it)
        {
            Stack<K>::pushLua(ctx, it->first);
            Stack<V>::pushLua(ctx, it->second);
            lua_settable(L, -3);
        }
    }

    static JSValue pushJS(EContext* ctx, M value)
    {
        JSContext* ct = (JSContext*)(ctx->GetState());
        JSValue arr = JS_NewArray(ct);

        typedef typename M::const_iterator ConstIter;

        for (ConstIter it = value.begin(); it != value.end(); ++it)
            JS_SetProperty(ct, arr, Stack<K>::pushJS(ctx, it->first), Stack<V>::pushJS(ctx, it->second));

        return arr;
    }

    static MapData* pushRawDotnet(EContext* ctx, CallContext* context, M value)
    {
        MapData* mapData = (MapData*)DotnetAllocateContextPointer(sizeof(MapData), 1);
        int count = value.size();
        mapData->keys = (void**)DotnetAllocateContextPointer(is_map<K>::value || is_vector<K>::value || std::is_same<std::string, K>::value ? sizeof(void*) : sizeof(K), count);
        mapData->values = (void**)DotnetAllocateContextPointer(is_map<V>::value || is_vector<V>::value || std::is_same<std::string, V>::value ? sizeof(void*) : sizeof(V), count);
        mapData->length = count;

        if constexpr (is_map<K>::value || is_vector<K>::value || std::is_same<std::string, K>::value) {
            void** listKeys = (void**)mapData->keys;

            mapData->key_type = is_map<K>::value ? 16 : (is_vector<K>::value ? 15 : typesMap[typeid(K)]);

            int i = 0;
            for (auto it = value.begin(); it != value.end(); ++it)
            {
                listKeys[i] = Stack<K>::pushRawDotnet(ctx, context, it->first);
                i++;
            }
        }
        else {
            K* listKeys = (K*)mapData->keys;

            mapData->key_type = typesMap[typeid(K)];

            int i = 0;
            for (auto it = value.begin(); it != value.end(); ++it)
            {
                listKeys[i] = Stack<K>::pushRawDotnet(ctx, context, it->first);
                i++;
            }
        }

        if constexpr (is_map<V>::value || is_vector<V>::value || std::is_same<std::string, V>::value) {
            void** listValues = (void**)mapData->values;

            mapData->key_type = is_map<V>::value ? 16 : (is_vector<V>::value ? 15 : typesMap[typeid(V)]);

            int i = 0;
            for (auto it = value.begin(); it != value.end(); ++it)
            {
                listValues[i] = Stack<V>::pushRawDotnet(ctx, context, it->second);
                i++;
            }
        }
        else {
            V* listValues = (V*)mapData->values;

            mapData->value_type = typesMap[typeid(V)];

            int i = 0;
            for (auto it = value.begin(); it != value.end(); ++it)
            {
                listValues[i] = Stack<V>::pushRawDotnet(ctx, context, it->second);
                i++;
            }
        }

        return mapData;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, M value, bool shouldReturn = false)
    {
        MapData* mapData = pushRawDotnet(ctx, context, value);

        if (shouldReturn) {
            context->SetReturnType(16);
            context->SetResult(mapData);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), 16);
            context->PushArgument(mapData);
        }
    }

    static M getLua(EContext* ctx, int ref)
    {
        M v;

        lua_State* L = (lua_State*)(ctx->GetState());
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

    static M getJS(EContext* ctx, JSValue value)
    {
        M v;

        JSContext* L = (JSContext*)(ctx->GetState());
        if (!JS_IsObject(value))
            return v;

        JSPropertyEnum* properties;
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

    static M getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        MapData* mapDatas = *(MapData**)value;
        M v;

        for (int i = 0; i < mapDatas->length; i++)
            v.emplace(Stack<K>::getRawDotnet(ctx, context, &(mapDatas->keys[i])), Stack<V>::getRawDotnet(ctx, context, &(mapDatas->values[i])));

        return v;
    }

    static M getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) {
            return getRawDotnet(ctx, context, context->GetResultPtr());
        }
        else {
            return getRawDotnet(ctx, context, context->GetArgumentPtr(index));
        }
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_istable((lua_State*)(ctx->GetState()), ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_IsObject(value);
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == 16;
        else return context->GetArgumentType(index) == 16;
    }
};

template <class T1, class T2>
struct Stack<std::pair<T1, T2>>
{
    static void pushLua(EContext* ctx, std::pair<T1, T2> value)
    {
        lua_State* L = (lua_State*)(ctx->GetState());

        lua_createtable(L, 2, 0);
        lua_pushinteger(L, (lua_Integer)1);
        Stack<T1>::pushLua(ctx, value.first);
        lua_settable(L, -3);
        lua_pushinteger(L, (lua_Integer)2);
        Stack<T2>::pushLua(ctx, value.second);
        lua_settable(L, -3);
    }

    static JSValue pushJS(EContext* ctx, std::pair<T1, T2> value)
    {
        JSContext* ct = (JSContext*)(ctx->GetState());
        JSValue arr = JS_NewArray(ct);

        JS_SetPropertyUint32(ct, arr, 0, Stack<T1>::pushJS(ctx, value.first));
        JS_SetPropertyUint32(ct, arr, 1, Stack<T2>::pushJS(ctx, value.second));

        return arr;
    }

    static T1* pushRawDotnet(EContext* ctx, CallContext* context, std::pair<T1, T2> value)
    {
        T1* arrayPtr = (T1*)DotnetAllocateContextPointer(sizeof(T1), 2);
        arrayPtr[0] = Stack<T1>::pushRawDotnet(ctx, context, value.first);
        arrayPtr[1] = Stack<T1>::pushRawDotnet(ctx, context, (T1)value.second);
    }

    static void pushDotnet(EContext* ctx, CallContext* context, std::pair<T1, T2> value, bool shouldReturn = false)
    {
        T1* arrayPtr = pushRawDotnet(ctx, context, value);

        if (shouldReturn) {
            context->SetReturnType(15);
            context->SetResult(arrayPtr);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), 15);
            context->PushArgument(arrayPtr);
        }
    }

    static std::pair<T1, T2> getLua(EContext* ctx, int ref)
    {
        std::pair<T1, T2> v;

        lua_State* L = (lua_State*)(ctx->GetState());
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

    static std::pair<T1, T2> getJS(EContext* ctx, JSValue value)
    {
        std::pair<T1, T2> v;

        JSContext* L = (JSContext*)(ctx->GetState());
        if (!JS_IsArray(value))
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

    static std::pair<T1, T2> getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        ArrayData* arrayDatas = *(ArrayData**)value;
        std::pair<T1, T2> v;
        if (arrayDatas->length != 2) return v;

        v.first = Stack<T1>::getRawDotnet(ctx, context, arrayDatas->elements[0]);
        v.second = Stack<T2>::getRawDotnet(ctx, context, arrayDatas->elements[1]);

        return v;
    }

    static std::pair<T1, T2> getDotnet(EContext* ctx, CallContext* context, int index)
    {
        std::pair<T1, T2> v;
        if (index == -1) {
            if (context->GetReturnType() != 15) return v;
            return getRawDotnet(ctx, context, context->GetResultPtr());
        }
        else {
            if (context->GetArgumentType(index) != 15) return v;
            return getRawDotnet(ctx, context, context->GetArgumentPtr(index));
        }
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_istable((lua_State*)(ctx->GetState()), ref) && get_length((lua_State*)(ctx->GetState()), ref) == 2;
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        if (!JS_IsArray(value))
            return false;
        return JSGetArrayLength((JSContext*)(ctx->GetState()), value) == 2;
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == 15;
        else return context->GetArgumentType(index) == 15;
    }
};

JSValue JSClassIndex(JSContext* L, JSValue this_val, int argc, JSValue* argv);
JSValue JSClassNewIndex(JSContext* L, JSValue this_val, int argc, JSValue* argv);

template<>
struct Stack<ClassData*>
{
    static void pushLua(EContext* ctx, ClassData* value)
    {
        if (ShouldDeleteOnGC(value)) {
            value = new ClassData(*value);
            MarkDeleteOnGC(value);

            std::vector<ClassData**> emptyData{};
            value->SetData("lua_udatas", emptyData);
        }

        auto L = ctx->GetLuaState();
        ClassData** udata = (ClassData**)lua_newuserdata(L, sizeof(ClassData*));
        *udata = value;

        luaL_getmetatable(L, value->GetClassname().c_str());
        lua_setmetatable(L, -2);

        std::vector<ClassData**> udatas = value->GetDataOr<std::vector<ClassData**>>("lua_udatas", std::vector<ClassData**>{});
        udatas.push_back(udata);
        value->SetData("lua_udatas", udatas);
    }

    static JSValue pushJS(EContext* ctx, ClassData* value)
    {
        if (ShouldDeleteOnGC(value)) {
            value = new ClassData(*value);
            MarkDeleteOnGC(value);
        }

        auto L = ctx->GetJSState();

        JSValue global_obj = JS_GetGlobalObject(L);
        JSValue proxy_ctor = JS_GetPropertyStr(L, global_obj, "Proxy");
        JS_FreeValue(L, global_obj);

        JSValue handler = JS_NewObject(L);
        JS_SetPropertyStr(L, handler, "get", JS_NewCFunction(L, JSClassIndex, "get", 2));
        JS_SetPropertyStr(L, handler, "set", JS_NewCFunction(L, JSClassNewIndex, "set", 3));

        JSValue args[2] = { JS_NewObject(L), handler };
        JSValue proxy_obj = JS_CallConstructor(L, proxy_ctor, 2, args);

        JS_FreeValue(L, proxy_ctor);
        JS_FreeValue(L, handler);

        JSClassID& id = *(ctx->GetClassID(value->GetClassname()));
        JS_SetClassProto(L, id, proxy_obj);
        auto ret = JS_NewObjectProtoClass(L, JS_GetClassProto(L, id), id);

        if (JS_IsException(ret))
        {
            JS_FreeValue(L, ret);
            return JS_EXCEPTION;
        }
        else
        {
            JS_SetOpaque(ret, (void*)value);

            std::vector<JSRuntime*> rts = value->GetDataOr<std::vector<JSRuntime*>>("js_runtimes", std::vector<JSRuntime*>{});
            bool found = false;

            auto rt = JS_GetRuntime(L);

            for (int i = 0; i < rts.size(); i++) {
                if (rts[i] == rt) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                rts.push_back(rt);
                value->SetData("js_runtimes", rts);
            }
        }

        return ret;
    }

    static ClassData* pushRawDotnet(EContext* ctx, CallContext* context, ClassData* value)
    {
        return value;
    }

    static void pushDotnet(EContext* ctx, CallContext* context, ClassData* value, bool shouldReturn = false)
    {
        if (ShouldDeleteOnGC(value)) {
            value = new ClassData(*value);
            MarkDeleteOnGC(value);
        }

        if (shouldReturn) {
            context->SetReturnType(typesMap[typeid(void*)]);
            context->SetResult(value);
        }
        else {
            context->SetArgumentType(context->GetArgumentCount(), typesMap[typeid(void*)]);
            context->PushArgument(value);
        }
    }

    static ClassData* getLua(EContext* ctx, int ref)
    {
        ClassData** data = (ClassData**)lua_touserdata(ctx->GetLuaState(), ref);
        return data ? *data : nullptr;
    }

    static ClassData* getJS(EContext* ctx, JSValue value)
    {
        auto vl = (ClassData*)JS_GetOpaque(value, JS_GetClassID(value));

        auto deletedClassDatas = (std::set<ClassData*>*)JS_GetRuntimeOpaque(JS_GetRuntime(ctx->GetJSState()));
        if (!deletedClassDatas) return vl;

        if (deletedClassDatas->find(vl) != deletedClassDatas->end()) return nullptr;
        return vl;
    }

    static ClassData* getRawDotnet(EContext* ctx, CallContext* context, void* value)
    {
        return *(ClassData**)value;
    }

    static ClassData* getDotnet(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetResult<ClassData*>();
        else return context->GetArgument<ClassData*>(index);
    }

    static bool isLuaInstance(EContext* ctx, int ref)
    {
        return lua_isuserdata(ctx->GetLuaState(), ref);
    }

    static bool isJSInstance(EContext* ctx, JSValue value)
    {
        return JS_GetOpaque(value, JS_GetClassID(value)) != nullptr;
    }

    static bool IsDotnetInstance(EContext* ctx, CallContext* context, int index)
    {
        if (index == -1) return context->GetReturnType() == typesMap[typeid(void*)];
        else return context->GetArgumentType(index) == typesMap[typeid(void*)];
    }
};

#endif
