#ifndef _embedder_internal_chelpers_h
#define _embedder_internal_chelpers_h

#include <lua.hpp>
#include <quickjs.h>
#include <cassert>
#include <string>
#include <iostream>
#include "Context.h"
#include "Stack.h"
#include "Invoker.h"
#include "GarbageCollector.h"

template <typename R, typename ... Params> constexpr size_t getArgumentCount( R(*f)(Params ...))
{
   return sizeof...(Params);
}

class CHelpers
{
public:
    template<class T>
    static JSValue propGetter(JSContext *ctx, JSValue this_val, int argc, JSValue *argv, int magic, JSValue *func_data)
    {
        return Stack<T>::pushJS(GetContextByState(ctx), *(T*)StringToPtr((const char*)func_data));
    }

    template<class T>
    static JSValue propSetter(JSContext *ctx, JSValue this_val, int argc, JSValue *argv, int magic, JSValue *func_data)
    {
        EContext* ct = GetContextByState(ctx);

        if(!Stack<T>::isJSInstance(ct, argv[0])) return JS_ThrowTypeError(ctx, "Property Setter: Invalid data type.");

        T& val = *(T*)StringToPtr((const char*)func_data);
        val = Stack<T>::getJS(ct, argv[0]);

        return JS_UNDEFINED;
    }

    template<class C, class T>
    static JSValue propClassGetter(JSContext *ctx, JSValue this_val, int argc, JSValue *argv, int magic, JSValue *func_data)
    {
        auto ectx = GetContextByState(ctx);
        C* val = Stack<C*>::getJS(ectx, this_val);
        T C::** mp = static_cast<T C::**>(StringToPtr((const char*)func_data));

        try {
            return Stack<T>::pushJS(ectx, val->**mp);
        } catch(std::exception& e) {
            return JS_ThrowInternalError(ctx, e.what());
        }
    }

    template<class C, class T>
    static JSValue propClassSetter(JSContext *ctx, JSValue this_val, int argc, JSValue *argv, int magic, JSValue *func_data)
    {
        auto ectx = GetContextByState(ctx);
        if(!Stack<T>::isJSInstance(ectx, argv[0])) return JS_ThrowTypeError(ctx, "Property Setter: Invalid data type.");

        C* val = Stack<C*>::getJS(ectx, this_val);
        T C::** mp = static_cast<T C::**>(StringToPtr((const char*)func_data));

        try {
            val->**mp = Stack<T>::getJS(ectx, argv[0]);
        } catch(std::exception& e) {
            return JS_ThrowInternalError(ctx, e.what());
        }

        return JS_UNDEFINED;
    }

    template<class ReturnType, class... Params>
    static JSValue JSCallFunction(JSContext *ctx, JSValue this_val, int argc, JSValue *argv, int magic, JSValue *func_data)
    {
        using FnType = ReturnType(*)(Params...);
        FnType func = reinterpret_cast<FnType>(StringToPtr((const char*)func_data));
        if(!func) return JS_UNDEFINED;

        return JSInvoker::run<ReturnType, Params...>(ctx, func, argv);
    }

    template<class ReturnType, class T, class... Params>
    static JSValue JSCallClassFunction(JSContext *ctx, JSValue this_val, int argc, JSValue *argv, int magic, JSValue *func_data)
    {
        union {
            void* ptr;
            ReturnType(T::*func)(Params...);
        } conv;
        
        conv.ptr = StringToPtr((const char*)func_data);

        if(!conv.ptr) return JS_UNDEFINED;
        return JSInvoker::runClass<ReturnType, T, Params...>(ctx, this_val, conv.func, argv);
    }

    static JSValue js_print_to_console(JSContext* ctx, JSValueConst this_val, int argc, JSValueConst* argv) {
        int i;
        const char* str;
        size_t len;
    
        for (i = 0; i < argc; i++) {
            if (i != 0) fputc('\t', stdout);
            str = JS_ToCStringLen(ctx, &len, argv[i]);
            if (!str) return JS_EXCEPTION;
            fwrite(str, 1, len, stdout);
            JS_FreeCString(ctx, str);
        }
        fputc('\n', stdout);
        return JS_UNDEFINED;
    }

    template <typename... Args>
    static std::string string_format(const std::string &format, Args... args)
    {
        int size_s = snprintf(nullptr, 0, format.c_str(), args...) + 1;
        if (size_s <= 0)
            return "";

        size_t size = static_cast<size_t>(size_s);
        char* buf = new char[size];
        snprintf(buf, size, format.c_str(), args...);
        std::string out = std::string(buf, buf + size - 1);
        delete buf;
        return out;
    }

    /************************************
    * All of the Lua Helpers were provided by LuaBridge 2.9. (https://github.com/vinniefalco/LuaBridge/tree/2.9)
    * LuaBridge is licensed under MIT License 
    * Copyright 2019, Dmitry Tarakanov
    * Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
    *************************************/

    static int indexMetaMethod(lua_State* L)
    {
        assert(lua_istable(L, 1) ||
               lua_isuserdata(L, 1)); // Stack (further not shown): table | userdata, name

        lua_getmetatable(L, 1); // Stack: class/const table (mt)
        assert(lua_istable(L, -1));

        for (;;)
        {
            lua_pushvalue(L, 2); // Stack: mt, field name
            lua_rawget(L, -2); // Stack: mt, field | nil

            if (lua_iscfunction(L, -1)) // Stack: mt, field
            {
                lua_remove(L, -2); // Stack: field
                return 1;
            }

            assert(lua_isnil(L, -1)); // Stack: mt, nil
            lua_pop(L, 1); // Stack: mt

            lua_rawgetp(L, -1, getPropgetKey()); // Stack: mt, propget table (pg)
            assert(lua_istable(L, -1));

            lua_pushvalue(L, 2); // Stack: mt, pg, field name
            lua_rawget(L, -2); // Stack: mt, pg, getter | nil
            lua_remove(L, -2); // Stack: mt, getter | nil

            if (lua_iscfunction(L, -1)) // Stack: mt, getter
            {
                lua_remove(L, -2); // Stack: getter
                lua_pushvalue(L, 1); // Stack: getter, table | userdata
                lua_call(L, 1, 1); // Stack: value
                return 1;
            }

            assert(lua_isnil(L, -1)); // Stack: mt, nil
            lua_pop(L, 1); // Stack: mt

            // It may mean that the field may be in const table and it's constness violation.
            // Don't check that, just return nil

            // Repeat the lookup in the parent metafield,
            // or return nil if the field doesn't exist.
            lua_rawgetp(L, -1, getParentKey()); // Stack: mt, parent mt | nil

            if (lua_isnil(L, -1)) // Stack: mt, nil
            {
                lua_remove(L, -2); // Stack: nil
                return 1;
            }

            // Removethe  metatable and repeat the search in the parent one.
            assert(lua_istable(L, -1)); // Stack: mt, parent mt
            lua_remove(L, -2); // Stack: parent mt
        }

        // no return
    }

   static int newindexStaticMetaMethod(lua_State* L) { return newindexMetaMethod(L, false); }

   static int newindexObjectMetaMethod(lua_State* L) { return newindexMetaMethod(L, true); }

    static int newindexMetaMethod(lua_State* L, bool pushSelf)
    {
       assert(
           lua_istable(L, 1) ||
           lua_isuserdata(L, 1)); // Stack (further not shown): table | userdata, name, new value

       lua_getmetatable(L, 1); // Stack: metatable (mt)
       assert(lua_istable(L, -1));

       for (;;)
       {
           lua_rawgetp(L, -1, getPropsetKey()); // Stack: mt, propset table (ps) | nil

           if (lua_isnil(L, -1)) // Stack: mt, nil
           {
               lua_pop(L, 2); // Stack: -
               return luaL_error(L, "No member named '%s'", lua_tostring(L, 2));
           }

           assert(lua_istable(L, -1));

           lua_pushvalue(L, 2); // Stack: mt, ps, field name
           lua_rawget(L, -2); // Stack: mt, ps, setter | nil
           lua_remove(L, -2); // Stack: mt, setter | nil

           if (lua_iscfunction(L, -1)) // Stack: mt, setter
           {
               lua_remove(L, -2); // Stack: setter
               if (pushSelf)
               {
                   lua_pushvalue(L, 1); // Stack: setter, table | userdata
               }
               lua_pushvalue(L, 3); // Stack: setter, table | userdata, new value
               lua_call(L, pushSelf ? 2 : 1, 0); // Stack: -
               return 0;
           }

           assert(lua_isnil(L, -1)); // Stack: mt, nil
           lua_pop(L, 1); // Stack: mt

           lua_rawgetp(L, -1, getParentKey()); // Stack: mt, parent mt | nil

           if (lua_isnil(L, -1)) // Stack: mt, nil
           {
               lua_pop(L, 1); // Stack: -
               return luaL_error(L, "No writable member '%s'", lua_tostring(L, 2));
           }

           assert(lua_istable(L, -1)); // Stack: mt, parent mt
           lua_remove(L, -2); // Stack: parent mt
           // Repeat the search in the parent
       }

       // no return
    }

   static int readOnlyError(lua_State* L)
    {
       std::string s;

       s = s + "'" + lua_tostring(L, lua_upvalueindex(1)) + "' is read-only";

       return luaL_error(L, s.c_str());
    }

    static void addGetter(lua_State* L, const char* name, int tableIndex)
    {
        assert(lua_istable(L, tableIndex));
        assert(lua_iscfunction(L, -1)); // getter

        lua_rawgetp(L, tableIndex, getPropgetKey()); // getter, pg
        lua_pushvalue(L, -2); // getter, pg, getter
        rawsetfield(L, -2, name); // getter, pg
        lua_pop(L, 2); // empty
    }

    static void addSetter(lua_State* L, const char* name, int tableIndex)
    {
        assert(lua_istable(L, tableIndex));
        assert(lua_iscfunction(L, -1)); // setter

        lua_rawgetp(L, tableIndex, getPropsetKey()); // setter, ps
        lua_pushvalue(L, -2); // setter, ps, setter
        rawsetfield(L, -2, name); // setter, ps
        lua_pop(L, 2); // empty
    }

    template<class T>
    static int getVariable(lua_State* L)
    {
        T* ptr = (T*)(lua_touserdata(L, lua_upvalueindex(1)));
        if(!ptr) Stack<std::nullptr_t>::pushLua(GetContextByState(L));
        else Stack<T>::pushLua(GetContextByState(L), *ptr);
        return 1;
    }

    template<class T>
    static int setVariable(lua_State* L)
    {
        T* ptr = (T*)(lua_touserdata(L, lua_upvalueindex(1)));
        if(!ptr) return 0;

        *ptr = Stack<T>::getLua(GetContextByState(L), 1);

        return 0;
    }

    template<class C, typename T>
    static int getProperty(lua_State* L)
    {
        C* c = *(C**)lua_touserdata(L, 1);
        T C::** mp = static_cast<T C::**>(lua_touserdata(L, lua_upvalueindex(1)));
        try
        {
            Stack<T>::pushLua(GetContextByState(L), c->**mp);
        }
        catch (const std::exception& e)
        {
            luaL_error(L, e.what());
        }
        return 1;
    }

    template<class C, typename T>
    static int setProperty(lua_State* L)
    {
        C* c = *(C**)lua_touserdata(L, 1);
        T C::** mp = static_cast<T C::**>(lua_touserdata(L, lua_upvalueindex(1)));
        try
        {
            c->** mp = Stack<T>::getLua(GetContextByState(L), 2);
        }
        catch (const std::exception& e)
        {
            luaL_error(L, e.what());
        }
        return 0;
    }

    /************************************
    * The end of the Lua Helpers were provided by LuaBridge 2.9. (https://github.com/vinniefalco/LuaBridge/tree/2.9)
    * LuaBridge is licensed under MIT License 
    * Copyright 2019, Dmitry Tarakanov
    * Copyright 2012, Vinnie Falco <vinnie.falco@gmail.com>
    *************************************/

    template<class ReturnType, class... Params>
    static int LuaCallFunction(lua_State* L)
    {
        using FnType = ReturnType(*)(Params...);

        FnType func = reinterpret_cast<FnType>(lua_touserdata(L, lua_upvalueindex(1)));
        if(!func) return 0;
        return LuaInvoker::run<ReturnType, Params...>(L, func);
    }

    template<class ReturnType, class T, class...Params>
    static int LuaCallClassFunction(lua_State* L)
    {
        union {
            void* ptr;
            ReturnType(T::*func)(Params...);
        } conv;
        
        conv.ptr = lua_touserdata(L, lua_upvalueindex(1));

        if(!conv.ptr) return 0;
        return LuaInvoker::runClass<ReturnType, T, Params...>(L, conv.func);
    }

    template<class T>
    static int LuaGCFunction(lua_State* L)
    {
        T** udata = (T**)lua_touserdata(L, 1);
        if(udata && *udata) {
            if(CheckAndPopDeleteOnGC((void*)*udata)) delete *udata;
            *udata = nullptr;
        }
        return 0;
    }

    template<class T, typename... Params, std::size_t... I>
    static T* construct_lua_helper(lua_State* L, std::index_sequence<I...>) {
        EContext* ctx = GetContextByState(L);
        return new T(Stack<std::decay_t<Params>>::getLua(ctx, static_cast<int>(I + 2))...);
    }

    template<class T, typename... Params>
    static int lua_dynamic_constructor(lua_State* L) {
        T* obj = construct_lua_helper<T, Params...>(L, std::index_sequence_for<Params...>{});
        T** udata = static_cast<T**>(lua_newuserdata(L, sizeof(T*)));
        *udata = obj;
        luaL_getmetatable(L, typeid(T).name());
        lua_setmetatable(L, -2);
        return 1;
    }
    
    template<class T>
    static void JSGCFunction(JSRuntime* rt, JSValue val)
    {
        auto opaque = JS_GetOpaque(val, JS_GetClassID(val));
        T* ptr = (T*)opaque;

        if(ptr && CheckAndPopDeleteOnGC((void*)ptr))
            delete ptr;
    }
    
    template<class T>
    static bool register_js_class(EContext* ct) {
        JSContext* ctx = (JSContext*)ct->GetState();
        JSClassID& id = *(ct->GetClassID(typeid(T).name()));
        if(id != 0) return false;

        JSRuntime* rt = JS_GetRuntime(ctx);
        JS_NewClassID(rt, &id);
        JSClassDef class_def = {
            typeid(T).name(),
        };
        class_def.finalizer = JSGCFunction<T>;

        return JS_NewClass(rt, id, &class_def) >= 0;
    }

    template<class T, typename... Params, std::size_t... I>
    static T* construct_js_helper(JSContext* L, JSValue* argv, std::index_sequence<I...>) {
        EContext* ctx = GetContextByState(L);
        return new T(Stack<std::decay_t<Params>>::getJS(ctx, argv[I])...);
    }

    template<class T, typename... Params>
    static JSValue js_dynamic_constructor(JSContext *ctx, JSValue this_val, int argc, JSValue *argv, int magic, JSValue *func_data)
    {
        T* ptr = construct_js_helper<T, Params...>(ctx, argv, std::index_sequence_for<Params...>{});

        JSClassID& id = *(GetContextByState(ctx)->GetClassID(typeid(T).name()));
        JSValue proto = JS_GetClassProto(ctx, id);
        JSValue ret = JS_NewObjectProtoClass(ctx, proto, id);

        if(!JS_IsException(ret)) JS_SetOpaque(ret, ptr);
        return ret;
    }

    template<class T>
    static std::string PtrToString(T v) {
        return string_format("%p", v);
    }

    static void* StringToPtr(std::string v) {
        #ifdef _WIN32
            return reinterpret_cast<void*>(strtoll(v.c_str(), nullptr, 16));
        #else
            return reinterpret_cast<void*>(strtol(v.c_str(), nullptr, 16));
        #endif
    }
};

#endif