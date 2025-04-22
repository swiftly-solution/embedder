#ifndef _embedding_internal_exception_h
#define _embedding_internal_exception_h

#include <exception>
#include <string>

#include <lua.hpp>
#include <quickjs.h>

#include "ContextKinds.h"
#include "Context.h"

class EException : public std::exception
{
private:
    std::string m_what;
    ContextKinds m_kind;
    void* m_ctx;
    char* toDeallocate = nullptr;

public:
    EException(void* ctx, ContextKinds kind, int /*code*/) { m_kind = kind; m_ctx = ctx; whatFromStack(); }
    EException(void* ctx, ContextKinds kind, char const*, char const*, long) { m_kind = kind; m_ctx = ctx; whatFromStack(); }
    ~EException() throw() {
        if (toDeallocate != nullptr) {
            JS_FreeCString((JSContext*)m_ctx, toDeallocate);
        }
    }

    const char* what() const throw() { return m_what.c_str(); }

    template<class Exception>
    static void Throw(Exception e)
    {
        throw e;
    }

    static void Enable(void* ctx, ContextKinds kind) {
        if (kind == ContextKinds::Lua) {
            lua_atpanic((lua_State*)ctx, thrower);
        }
    }

    static void pcall(EContext* ctx, int args, int results)
    {
        if (ctx->GetKind() == ContextKinds::Lua) {
            int code = lua_pcall((lua_State*)ctx->GetState(), args, results, 0);
            if (code != LUA_OK) Throw(EException(ctx->GetState(), ctx->GetKind(), code));
        }
    }

    static void xpcall(EContext* ctx, int args, int results)
    {
        if (ctx->GetKind() == ContextKinds::Lua) {
            lua_pushcfunction((lua_State*)ctx->GetState(), traceback);
            lua_insert((lua_State*)ctx->GetState(), 1);
            int code = lua_pcall((lua_State*)ctx->GetState(), args, results, 1);
            lua_remove((lua_State*)ctx->GetState(), 1);
            if (code != LUA_OK) Throw(EException(ctx->GetState(), ctx->GetKind(), code));
        }
    }

protected:
    void whatFromStack() {
        if (m_kind == ContextKinds::Lua) {
            if (lua_gettop((lua_State*)m_ctx) > 0) {
                std::string errorMessage;
                const char* errorPtr = lua_tostring((lua_State*)m_ctx, -1);
                errorMessage = errorPtr ? errorPtr : "Empty error.";
                lua_pop((lua_State*)m_ctx, 1);

                lua_pushcfunction((lua_State*)m_ctx, traceback);
                lua_pushvalue((lua_State*)m_ctx, -2);
                lua_pushinteger((lua_State*)m_ctx, 2);
                lua_call((lua_State*)m_ctx, 2, 1);

                const char* tracebackPtr = lua_tostring((lua_State*)m_ctx, -1);
                std::string tracebackString = tracebackPtr ? tracebackPtr : "No traceback available.";
                lua_pop((lua_State*)m_ctx, 1);

                m_what = errorMessage;
            }
            else {
                m_what = "Empty error.";
            }
        }
        else if (m_kind == ContextKinds::JavaScript) {
            if (JS_HasException((JSContext*)m_ctx)) {
                JSValue exc = JS_GetException((JSContext*)m_ctx);

                if (JS_IsString(exc)) {
                    const char* tmp = JS_ToCString((JSContext*)m_ctx, exc);
                    std::string str(tmp, strlen(tmp));
                    m_what = str;
                    JS_FreeCString((JSContext*)m_ctx, tmp);
                }
                else if (JS_IsError((JSContext*)m_ctx, exc)) {
                    JSValue stack = JS_GetPropertyStr((JSContext*)m_ctx, exc, "stack");

                    const char* tmp = JS_ToCString((JSContext*)m_ctx, exc);
                    const char* stk = JS_ToCString((JSContext*)m_ctx, stack);
                    std::string str(tmp);
                    str += "\nStack Trace:\n" + std::string(stk);
                    m_what = str;
                    JS_FreeCString((JSContext*)m_ctx, stk);
                    JS_FreeCString((JSContext*)m_ctx, tmp);

                    JS_FreeValue((JSContext*)m_ctx, stack);
                }
                else m_what = "Empty error or invalid value.";

                JS_FreeValue((JSContext*)m_ctx, exc);
            }
        }
    }

private:
    static int thrower(lua_State* L) { throw EException((void*)L, ContextKinds::Lua, -1); }
    static int traceback(lua_State* L) {
        lua_getglobal(L, "debug");
        if (!lua_istable(L, -1)) {
            lua_pop(L, 1);
            return 1;
        }
        lua_getfield(L, -1, "traceback");
        if (!lua_isfunction(L, -1)) {
            lua_pop(L, 2);
            return 1;
        }
        lua_pushvalue(L, 1);
        lua_pushinteger(L, 2);
        lua_call(L, 2, 1);
        return 1;
    }
};

#endif