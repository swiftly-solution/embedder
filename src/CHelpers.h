#ifndef _embedder_internal_chelpers_h
#define _embedder_internal_chelpers_h

#include <lua.hpp>
#include <quickjs.h>
#include <cassert>
#include <string>
#include <iostream>
#include <vector>

#include "GarbageCollector.h"
#include "engine/classes.h"

class CHelpers
{
public:
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

    static int LuaGCFunction(lua_State* L)
    {
        ClassData** udata = (ClassData**)lua_touserdata(L, 1);
        if (udata && *udata) {
            std::vector<ClassData**> udatas = (*udata)->GetDataOr<std::vector<ClassData**>>("lua_udatas", std::vector<ClassData**>{});
            for (int i = 0; i < udatas.size(); i++) {
                if (udatas[i] == udata) {
                    udatas.erase(udatas.begin() + i);
                    break;
                }
            }
            (*udata)->SetData("lua_udatas", udatas);

            if (CheckAndPopDeleteOnGC(*udata)) {
                delete (*udata);
            }
            *udata = nullptr;
        }
        return 0;
    }

    static void JSGCFunction(JSRuntime* rt, JSValue val)
    {
        ClassData* opaque = (ClassData*)JS_GetOpaque(val, JS_GetClassID(val));
        if (opaque && CheckAndPopDeleteOnGC(opaque)) {
            delete opaque;
        }

        JS_SetOpaque(val, nullptr);
    }

    static void DotNetGCFunction(EContext* ctx, ClassData* data, std::set<void*>* droppedValues)
    {
        if (data && CheckAndPopDeleteOnGC(data)) {
            if (droppedValues->find(data) != droppedValues->end()) return;
            droppedValues->insert(data);
            delete data;
        }
    }
};

#endif