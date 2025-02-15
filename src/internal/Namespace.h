#ifndef _embedder_internal_namespace_h
#define _embedder_internal_namespace_h

#include <string>
#include <unordered_map>

#include <lua.hpp>
#include <quickjs.h>

#include "Context.h"
#include "CHelpers.h"
#include "Value.h"

template<class T>
class EClass;

class Namespace
{
public:
    int m_ref;
    JSValue m_ns;
    EContext* m_ctx;
    std::string m_name;
    Namespace* m_parent;

    Namespace() = default;
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

template<class T>
class EClass
{
private:
    std::string m_name;
    EContext* m_ctx;
    int m_ref;
    JSValue m_constructor = JS_NULL;
    JSValue m_proto = JS_NULL;
    bool protoSet = false;
    JSClassID id;

protected:
    void createLuaConstTable(lua_State* L, const std::string& name, bool trueConst = true) {
        std::string type_name = (trueConst ? "const " : "") + name;
        luaL_newmetatable(L, typeid(T).name());
        lua_pushvalue(L, -1);
        lua_setmetatable(L, -2);
        lua_pushstring(L, type_name.c_str());
        lua_rawsetp(L, -2, getTypeKey());
        lua_pushcfunction(L, &CHelpers::indexMetaMethod);
        rawsetfield(L, -2, "__index");
        lua_pushcfunction(L, &CHelpers::newindexObjectMetaMethod);
        rawsetfield(L, -2, "__newindex");
        lua_newtable(L);
        lua_rawsetp(L, -2, getPropgetKey());
        lua_newtable(L);
        lua_rawsetp(L, -2, getPropsetKey());
        lua_pushnil(L);
        rawsetfield(L, -2, "__metatable");
    }

public:
    EClass(std::string name, EContext* ctx)
    {
        m_name = name;
        m_ctx = ctx;

        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();

            lua_newtable(L);
            createLuaConstTable(L, name);

            m_ref = luaL_ref(L, LUA_REGISTRYINDEX);
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            CHelpers::register_js_class<T>(m_ctx);
            id = *(m_ctx->GetClassID(typeid(T).name()));

            m_proto = JS_NewObject((JSContext*)m_ctx->GetState());

            JS_SetPropertyStr((JSContext*)m_ctx->GetState(), m_proto, "_className", Stack<std::string>::pushJS(m_ctx, typeid(T).name()));
        }
    }

    template<class... Params>
    EClass<T> addConstructor()
    {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();
            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);

            lua_newtable(L);
            lua_pushcclosure(L, &CHelpers::lua_dynamic_constructor<T, Params...>, 0);
            rawsetfield(L, -2, "__call");
            lua_setmetatable(L, -2);

            lua_pop(L, 1);
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ctx = static_cast<JSContext*>(m_ctx->GetState());
            m_constructor = JS_NewCFunctionData(ctx, &CHelpers::js_dynamic_constructor<T, Params...>, sizeof...(Params), 1, 1, (JSValue*)(m_name.c_str()));
        }
        return *this;
    }

    template<class ReturnType, class... Params>
    EClass<T> addFunction(std::string name, ReturnType (*func)(T*, Params...)) {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = static_cast<lua_State*>(m_ctx->GetState());

            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
            lua_pushlightuserdata(L, reinterpret_cast<void*>(strtol(CHelpers::string_format("%p", func).c_str(), nullptr, 16)));
            lua_pushcclosure(L, CHelpers::LuaCallFunction<ReturnType, T*, Params...>, 1);
            rawsetfield(L, -2, name.c_str());
            lua_pop(L, 1);
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ctx = (JSContext*)m_ctx->GetState();
            JS_SetPropertyStr(
                ctx, m_proto, name.c_str(), 
                JS_NewCFunctionData(ctx, CHelpers::JSCallClassFunction<ReturnType, T, Params...>, 0, 1, 1, (JSValue*)(CHelpers::string_format("%p", func).c_str()))
            );
        }

        return *this;
    }

    template<class ReturnType, class... Params>
    EClass<T> addFunction(std::string name, ReturnType(T::*func)(Params...) const) {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = static_cast<lua_State*>(m_ctx->GetState());

            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
            lua_pushlightuserdata(L, reinterpret_cast<void*>(strtol(CHelpers::string_format("%p", func).c_str(), nullptr, 16)));
            lua_pushcclosure(L, CHelpers::LuaCallFunction<ReturnType, T*, Params...>, 1);
            rawsetfield(L, -2, name.c_str());
            lua_pop(L, 1);
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ctx = (JSContext*)m_ctx->GetState();
            JS_SetPropertyStr(
                ctx, m_proto, name.c_str(), 
                JS_NewCFunctionData(ctx, CHelpers::JSCallClassFunction<ReturnType, T, Params...>, 0, 1, 1, (JSValue*)(CHelpers::string_format("%p", func).c_str()))
            );
        }

        return *this;
    }

    template<class ReturnType, class... Params>
    EClass<T> addFunction(std::string name, ReturnType(T::*func)(Params...)) {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = static_cast<lua_State*>(m_ctx->GetState());

            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
            lua_pushlightuserdata(L, reinterpret_cast<void*>(strtol(CHelpers::string_format("%p", func).c_str(), nullptr, 16)));
            lua_pushcclosure(L, CHelpers::LuaCallFunction<ReturnType, T*, Params...>, 1);
            rawsetfield(L, -2, name.c_str());
            lua_pop(L, 1);
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ctx = (JSContext*)m_ctx->GetState();
            JS_SetPropertyStr(
                ctx, m_proto, name.c_str(), 
                JS_NewCFunctionData(ctx, CHelpers::JSCallClassFunction<ReturnType, T, Params...>, 0, 1, 1, (JSValue*)(CHelpers::string_format("%p", func).c_str()))
            );
        }

        return *this;
    }

    template<class ReturnType, class... Params>
    EClass<T> addLuaFunction(std::string name, ReturnType(T::*func)(Params...))
    {
        if(m_ctx->GetKind() != ContextKinds::Lua) return *this;

        lua_State* L = static_cast<lua_State*>(m_ctx->GetState());

        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
        lua_pushlightuserdata(L, reinterpret_cast<void*>(strtol(CHelpers::string_format("%p", func).c_str(), nullptr, 16)));
        lua_pushcclosure(L, CHelpers::LuaCallFunction<ReturnType, T*, Params...>, 1);
        rawsetfield(L, -2, name.c_str());
        lua_pop(L, 1);

        return *this;
    }

    template<class ReturnType, class... Params>
    EClass<T> addLuaFunction(std::string name, ReturnType (*func)(T*, Params...))
    {
        if(m_ctx->GetKind() != ContextKinds::Lua) return *this;

        lua_State* L = static_cast<lua_State*>(m_ctx->GetState());

        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
        lua_pushlightuserdata(L, reinterpret_cast<void*>(strtol(CHelpers::string_format("%p", func).c_str(), nullptr, 16)));
        lua_pushcclosure(L, CHelpers::LuaCallFunction<ReturnType, T*, Params...>, 1);
        rawsetfield(L, -2, name.c_str());
        lua_pop(L, 1);

        return *this;
    }

    EClass<T> addLuaFunction(std::string name, lua_CFunction func)
    {
        if(m_ctx->GetKind() != ContextKinds::Lua) return *this;

        lua_State* L = static_cast<lua_State*>(m_ctx->GetState());

        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
        lua_pushcfunction(L, func);
        rawsetfield(L, -2, name.c_str());
        lua_pop(L, 1);

        return *this;
    }

    template<class ReturnType, class... Params>
    EClass<T> addJSFunction(std::string name, ReturnType(T::*func)(Params...))
    {
        if(m_ctx->GetKind() != ContextKinds::JavaScript) return *this;

        JSContext* ctx = (JSContext*)m_ctx->GetState();
        JS_SetPropertyStr(
            ctx, m_proto, name.c_str(), 
            JS_NewCFunctionData(ctx, CHelpers::JSCallClassFunction<ReturnType, T, Params...>, 0, 1, 1, (JSValue*)(CHelpers::string_format("%p", func).c_str()))
        );

        return *this;
    }

    template<class ReturnType, class... Params>
    EClass<T> addJSFunction(std::string name, ReturnType (*func)(T*, Params...))
    {
        if(m_ctx->GetKind() != ContextKinds::JavaScript) return *this;

        JSContext* ctx = (JSContext*)m_ctx->GetState();
        JS_SetPropertyStr(
            ctx, m_proto, name.c_str(), 
            JS_NewCFunctionData(ctx, CHelpers::JSCallClassFunction<ReturnType, T, Params...>, 0, 1, 1, (JSValue*)(CHelpers::string_format("%p", func).c_str()))
        );

        return *this;
    }

    EClass<T> addJSFunction(std::string name, JSCFunction func)
    {
        if(m_ctx->GetKind() != ContextKinds::JavaScript) return *this;

        JSContext* ctx = (JSContext*)m_ctx->GetState();
        JS_SetPropertyStr(ctx, m_proto, name.c_str(), JS_NewCFunction(ctx, func, name.c_str(), 0));
        
        return *this;
    }

    template<class PropType>
    EClass<T> addProperty(std::string name, PropType T::*member, bool writable = true) {
        typedef const PropType T::*mp_t;
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = static_cast<lua_State*>(m_ctx->GetState());

            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);

            new (lua_newuserdata(L, sizeof(mp_t))) mp_t(member); 
            lua_pushcclosure(L, &CHelpers::getProperty<T, PropType>, 1); 
            CHelpers::addGetter(L, name.c_str(), -2); 

            if (writable)
            {
                new (lua_newuserdata(L, sizeof(mp_t))) mp_t(member);
                lua_pushcclosure(L, &CHelpers::setProperty<T, PropType>, 1);
                CHelpers::addSetter(L, name.c_str(), -2);
            }
            lua_pop(L, 1);
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* ctx = (JSContext*)m_ctx->GetState();

            JSAtom atom = JS_NewAtom(ctx, name.c_str());

            auto memb = new mp_t(member);

            JS_DefinePropertyGetSet(
                ctx, m_proto, atom,
                JS_NewCFunctionData(ctx, CHelpers::propClassGetter<T, PropType>, 0, 1, 1, (JSValue*)(CHelpers::string_format("%p", memb).c_str())),
                writable ? JS_NewCFunctionData(ctx, CHelpers::propClassSetter<T, PropType>, 1, 1, 1, (JSValue*)(CHelpers::string_format("%p", memb).c_str())) : JS_UNDEFINED,
                0
            );

            JS_FreeAtom(ctx, atom);
        }
        return *this;
    }

    EClass<T> addLuaCustomIndex(lua_CFunction index, lua_CFunction newindex)
    {
        if(m_ctx->GetKind() != ContextKinds::Lua) return *this;
        lua_State* L = (lua_State*)m_ctx->GetState();

        lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
        lua_pushcfunction(L, index);
        rawsetfield(L, -2, "__index");
        lua_pushcfunction(L, newindex);
        rawsetfield(L, -2, "__newindex");
        lua_pop(L, 1);

        return *this;
    }

    EClass<T> addJSCustomIndex(JSCFunction jsindex, JSCFunction jsnewindex)
    {
        if(m_ctx->GetKind() != ContextKinds::JavaScript) return *this;
        JSContext* jsctx = (JSContext*)m_ctx->GetState();
        
        JSValue global_obj = JS_GetGlobalObject(jsctx);
        JSValue proxy_ctor = JS_GetPropertyStr(jsctx, global_obj, "Proxy");
        JS_FreeValue(jsctx, global_obj);

        JSValue handler = JS_NewObject(jsctx);
        JS_SetPropertyStr(jsctx, handler, "get", JS_NewCFunction(jsctx, jsindex, "get", 2));
        JS_SetPropertyStr(jsctx, handler, "set", JS_NewCFunction(jsctx, jsnewindex, "set", 3));

        JSValue args[2] = { m_proto, handler };
        JSValue proxy_obj = JS_CallConstructor(jsctx, proxy_ctor, 2, args);

        JS_FreeValue(jsctx, proxy_ctor);
        JS_FreeValue(jsctx, handler);

        protoSet = true;
        JS_SetClassProto(jsctx, id, proxy_obj);
        return *this;
    }

    void endClass()
    {
        if(m_ctx->GetKind() == ContextKinds::Lua) {
            lua_State* L = (lua_State*)m_ctx->GetState();
            lua_pushglobaltable(L);
            lua_rawgeti(L, LUA_REGISTRYINDEX, m_ref);
            rawsetfield(L, -2, m_name.c_str());
            lua_pop(L, 1);
        } else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JSContext* L = (JSContext*)m_ctx->GetState();
            if(!JS_IsNull(m_constructor)) JS_SetPropertyStr(L, JS_GetGlobalObject((JSContext*)(m_ctx->GetState())), m_name.c_str(), m_constructor);
            if(!protoSet) {
                JS_SetClassProto(L, id, m_proto);
            }
        }
    }
};

Namespace GetGlobalNamespace(EContext* ctx);

template<class T>
EClass<T> BeginClass(std::string name, EContext* ctx)
{
    return EClass<T>(name, ctx);
}

#endif