#ifndef _embedding_internal_exception_h
#define _embedding_internal_exception_h

#include <exception>
#include <string>

#include <lua.hpp>
#include <quickjs.h>

#include "../ContextKinds.h"

class EException: public std::exception
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
        if(toDeallocate != nullptr) {
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
        if(kind == ContextKinds::Lua) {
            lua_atpanic((lua_State*)ctx, thrower);
        }
    }

protected:
    void whatFromStack() {
        if(m_kind == ContextKinds::Lua) {
            if(lua_gettop((lua_State*)m_ctx) > 0) {
                auto s = lua_tostring((lua_State*)m_ctx, -1);
                m_what = s ? s : "Empty error.";
            } else {
                m_what = "Empty error.";
            }
        } else if(m_kind == ContextKinds::JavaScript) {
            if(JS_HasException((JSContext*)m_ctx)) {
                JSValue exc = JS_GetException((JSContext*)m_ctx);
                
                if(JS_IsString(exc)) {
                    const char* tmp = JS_ToCString((JSContext*)m_ctx, exc);
                    std::string str(tmp, strlen(tmp));
                    m_what = str;
                    JS_FreeCString((JSContext*)m_ctx, tmp);
                } else m_what = "Empty error or invalid value.";
            }
        }
    }

private:
    static int thrower(lua_State* L) { throw EException((void*)L, ContextKinds::Lua, -1); }
};

#endif