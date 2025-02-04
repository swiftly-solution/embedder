#ifndef _embedder_internal_namespace_h
#define _embedder_internal_namespace_h

#include <string>
#include <unordered_map>

#include <lua.hpp>
#include <quickjs.h>

#include "Context.h"
#include "CHelpers.h"

class Namespace
{
private:
    int m_ref;
    JSValue m_ns;
    EContext* m_ctx;
    std::string m_name;
    Namespace* m_parent;

public:
    Namespace(EContext* ctx, std::string nsName, Namespace *parent = nullptr);
    ~Namespace();

    Namespace beginNamespace(std::string nsName);
    Namespace endNamespace();

    template<class T>
    Namespace& addConstant(std::string name, T value)
    {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();
            
            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref); // ns

            Stack<T>::pushLua(m_ctx, value); // ns, value
            rawsetfield(L, -2, name.c_str()); // ns[name] = value. ns

            lua_pop(L, 1); // empty
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JS_SetPropertyStr((JSContext*)m_ctx->GetState(), m_ns, name.c_str(), Stack<T>::pushJS(m_ctx, value));
        }

        return *this;
    }

    template<class T>
    Namespace& addProperty(std::string name, T* value, bool isWritable = true)
    {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();

            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref); // ns

            lua_pushlightuserdata(L, (void*)value); // ns, ptr (upvalue 1 passed to closure)
            lua_pushcclosure(L, CHelpers::getVariable<T>, 1); // ns, ptr, getFunc (1 upvalue)
            CHelpers::addGetter(L, name.c_str(), -2); // ns

            if(isWritable) {
                lua_pushlightuserdata(L, (void*)value); // ns, ptr (upvalue 1 passed to closure)
                lua_pushcclosure(L, CHelpers::setVariable<T>, 1);  // ns, ptr, setFunc (1 upvalue) 
            } else {
                lua_pushstring(L, name.c_str()); // ns, name (upvalue 1 passed to closure)
                lua_pushcclosure(L, CHelpers::readOnlyError, 1);// ns, name, err (1 upvalue) 
            }
            CHelpers::addSetter(L, name.c_str(), -2); // ns

            lua_pop(L, 1); // empty
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSAtom atom = JS_NewAtom((JSContext*)m_ctx->GetState(), name.c_str());

            // Due to the value being duplicated inside NewCFunctionData, we need to cast the data into a string, so that the string with the pointer is duplicated but not the value
            JS_DefinePropertyGetSet(
                (JSContext*)m_ctx->GetState(), m_ns, atom,
                JS_NewCFunctionData((JSContext*)m_ctx->GetState(), CHelpers::propGetter<T>, 0, 1, 1, (JSValue*)(CHelpers::string_format("%p", value).c_str())),
                isWritable ? JS_NewCFunctionData((JSContext*)m_ctx->GetState(), CHelpers::propSetter<T>, 1, 1, 1, (JSValue*)(CHelpers::string_format("%p", value).c_str())) : JS_UNDEFINED,
                0
            );

            JS_FreeAtom((JSContext*)m_ctx->GetState(), atom);
        }

        return *this;
    }

    template<class ReturnType, class... Params>
    Namespace& addFunction(std::string name, ReturnType (*func)(Params...)) {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();
            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref); // ns

            lua_pushlightuserdata(L, reinterpret_cast<void*>(func)); // ns, ptr (upvalue 1)
            lua_pushcclosure(L, CHelpers::LuaCallFunction<ReturnType, Params...>, 1); // ns, ptr, closure
            rawsetfield(L, -2, name.c_str()); // ns

            lua_pop(L, 1); // empty
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ctx = (JSContext*)m_ctx->GetState();

            JS_SetPropertyStr(
                ctx, m_ns, name.c_str(), 
                JS_NewCFunctionData(ctx, CHelpers::JSCallFunction<ReturnType, Params...>, 0, 1, 1, (JSValue*)(CHelpers::string_format("%p", reinterpret_cast<void*>(func)).c_str()))
            );
        }

        return *this;
    }

    Namespace& addLuaFunction(std::string name, lua_CFunction func) {
        if(m_ctx->GetKind() != ContextKinds::Lua) return *this;

        lua_State* L = (lua_State*)m_ctx->GetState();
        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref); // ns

        lua_pushcfunction(L, func); // ns, func
        rawsetfield(L, -2, name.c_str()); // ns[name] = func. ns
        
        lua_pop(L, 1); // empty
        return *this;
    }

    Namespace& addJSFunction(std::string name, JSCFunction func) {
        if(m_ctx->GetKind() != ContextKinds::JavaScript) return *this;

        JSContext* ctx = (JSContext*)m_ctx->GetState();

        JS_SetPropertyStr(
            ctx, m_ns, name.c_str(), 
            JS_NewCFunction(ctx, func, name.c_str(), 0)
        );
        return *this;
    }
};

Namespace GetGlobalNamespace(EContext* ctx);

#endif