#include "../Engine.h"
#include "../CHelpers.h"
#include "../Helpers.h"

#include <vector>

void AddScriptingVariable(EContext* ctx, std::string namespace_path, std::string variable_name, EValue value)
{
    if (ctx->GetKind() == ContextKinds::Lua)
    {
        int pop_values = 0;
        pop_values++;

        auto L = ctx->GetLuaState();
        lua_pushglobaltable(L);

        if (namespace_path != "_G")
        {
            auto paths = str_split(namespace_path, ".");
            for (auto path : paths)
            {
                rawgetfield(L, -1, path.c_str());

                if (lua_isnil(L, -1))
                {
                    lua_pop(L, 1);

                    lua_newtable(L); // parent ns, ns

                    lua_pushvalue(L, -1); // parent ns, ns, ns

                    // ns.__metatable = ns
                    lua_setmetatable(L, -2); // parent ns, ns

                    // ns.__index = indexMetaMethod
                    lua_pushcfunction(L, &CHelpers::indexMetaMethod);
                    rawsetfield(L, -2, "__index"); // parent ns, ns

                    // ns.__newindex = newindexMetaMethod
                    lua_pushcfunction(L, &CHelpers::newindexStaticMetaMethod);
                    rawsetfield(L, -2, "__newindex"); // pns, ns

                    lua_newtable(L);                     // pns, ns, propget table (pg)
                    lua_rawsetp(L, -2, getPropgetKey()); // ns [propgetKey] = pg. pns, ns

                    lua_newtable(L);                     // pns, ns, propset table (ps)
                    lua_rawsetp(L, -2, getPropsetKey()); // ns [propsetKey] = ps. pns, ns

                    // pns [name] = ns
                    lua_pushvalue(L, -1);             // pns, ns, ns
                    rawsetfield(L, -3, path.c_str()); // pns, ns
                }

                pop_values++;
            }
        }

        value.pushLua();
        rawsetfield(L, -2, variable_name.c_str());

        lua_pop(L, pop_values);
    }
}

void AddScriptingVariables(EContext* ctx, std::string namespace_path, std::map<std::string, EValue> values)
{
    for (auto it = values.begin(); it != values.end(); ++it)
        AddScriptingVariable(ctx, namespace_path, it->first, it->second);
}