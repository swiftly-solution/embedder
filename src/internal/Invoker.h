#ifndef _embedder_internal_functions_h
#define _embedder_internal_functions_h

#include <lua.hpp>
#include <quickjs.h>

#include <exception>

#include "Context.h"
#include "Stack.h"

class LuaInvoker
{
public:
    template<class ReturnType, class... Params, std::size_t... I>
    static ReturnType callFunctionImpl(EContext* ctx, ReturnType (*func)(Params...), std::index_sequence<I...>) {
        return func(Stack<Params>::getLua(ctx, int(I + 1))...);
    }

    template<class ReturnType, class... Params>
    static int run(lua_State* L, ReturnType (*func)(Params...)) {
        try {
            EContext* ctx = GetContextByState(L);

            if constexpr (std::is_same<ReturnType, void>::value) {
                callFunctionImpl<ReturnType, Params...>(ctx, func, std::index_sequence_for<Params...>{});
                return 0;
            } else {
                ReturnType val = callFunctionImpl<ReturnType, Params...>(ctx, func, std::index_sequence_for<Params...>{});
                Stack<ReturnType>::pushLua(ctx, val);
                return 1;
            }
        } catch(const std::exception& e) {
            return luaL_error(L, e.what());
        }
    }

    template<class ReturnType, class T, class... Params, std::size_t... I>
    static ReturnType callFunctionClassImpl(EContext* ctx, ReturnType(T::*func)(Params...), std::index_sequence<I...>) {
        T* ptr = Stack<T*>::getLua(ctx, 1);
        return (ptr->*func)(Stack<Params>::getLua(ctx, int(I + 2))...);
    }

    template<class ReturnType, class T, class... Params>
    static int runClass(lua_State* L, ReturnType(T::*func)(Params...)) {
        try {
            EContext* ctx = GetContextByState(L);

            if constexpr (std::is_same<ReturnType, void>::value) {
                callFunctionClassImpl<ReturnType, T, Params...>(ctx, func, std::index_sequence_for<Params...>{});
                return 0;
            } else {
                ReturnType val = callFunctionClassImpl<ReturnType, T, Params...>(ctx, func, std::index_sequence_for<Params...>{});
                Stack<ReturnType>::pushLua(ctx, val);
                return 1;
            }
        } catch(const std::exception& e) {
            return luaL_error(L, e.what());
        }
    }
};

class JSInvoker
{
public:
    template<class ReturnType, class... Params, std::size_t... I>
    static ReturnType callFunctionImpl(EContext* ctx, JSValue* args, ReturnType (*func)(Params...), std::index_sequence<I...>) {
        return func(Stack<Params>::getJS(ctx, args[I])...);
    }

    template<class ReturnType, class... Params>
    static JSValue run(JSContext* ctx, ReturnType (*func)(Params...), JSValue* args) {
        try {
            EContext* ictx = GetContextByState(ctx);

            if constexpr (std::is_same<ReturnType, void>::value) {
                callFunctionImpl<ReturnType, Params...>(ictx, args, func, std::index_sequence_for<Params...>{});
                return JS_UNDEFINED;
            } else {
                ReturnType val = callFunctionImpl<ReturnType, Params...>(ictx, args, func, std::index_sequence_for<Params...>{});
                return Stack<ReturnType>::pushJS(ictx, val);
            }
        } catch(const std::exception& e) {
            return JS_ThrowInternalError(ctx, e.what());
        }
    }

    template<class ReturnType, class T, class... Params, std::size_t... I>
    static ReturnType callFunctionImplClass(EContext* ctx, void* ptr, JSValue* args, ReturnType (T::*func)(Params...), std::index_sequence<I...>) {
        T* vPtr = (T*)ptr;
        return (vPtr->*func)(Stack<Params>::getJS(ctx, args[I])...);
    }

    template<class ReturnType, class T, class... Params>
    static JSValue runClass(JSContext* ctx, JSValue this_obj, ReturnType (T::*func)(Params...), JSValue* args) {
        try {
            EContext* ictx = GetContextByState(ctx);

            if constexpr (std::is_same<ReturnType, void>::value) {
                callFunctionImplClass<ReturnType, T, Params...>(ictx, Stack<T*>::getJS(ictx, this_obj), args, func, std::index_sequence_for<Params...>{});
                return JS_UNDEFINED;
            } else {
                ReturnType val = callFunctionImplClass<ReturnType, T, Params...>(ictx, Stack<T*>::getJS(ictx, this_obj), args, func, std::index_sequence_for<Params...>{});
                return Stack<ReturnType>::pushJS(ictx, val);
            }
        } catch(const std::exception& e) {
            return JS_ThrowInternalError(ctx, e.what());
        }
    }
};

#endif