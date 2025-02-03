#include "Namespace.h"

Namespace GetGlobalNamespace(EContext* ctx) {
    return Namespace(ctx, "_G");
}

Namespace::Namespace(EContext* ctx, std::string name, Namespace *parent)
{
    m_name = name;
    m_parent = parent;
    m_ctx = ctx;

    if(ctx->GetKind() == ContextKinds::Lua) {
        lua_State* L = (lua_State*)ctx->GetState();
        if(name == "_G") {
            lua_pushglobaltable(L); // ns
            m_ref = luaL_ref(L, LUA_REGISTRYINDEX); // empty
        } else {
            if(!m_parent) return;

            lua_rawgeti(L, LUA_REGISTRYINDEX, m_parent->m_ref); // parent ns
            rawgetfield(L, -1, name.c_str()); // parent ns, ns?

            if(lua_isnil(L, -1)) {
                lua_pop(L, 1); // parent ns

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

                lua_newtable(L); // pns, ns, propget table (pg)
                lua_rawsetp(L, -2, getPropgetKey()); // ns [propgetKey] = pg. pns, ns
    
                lua_newtable(L); // pns, ns, propset table (ps)
                lua_rawsetp(L, -2, getPropsetKey()); // ns [propsetKey] = ps. pns, ns

                // pns [name] = ns
                lua_pushvalue(L, -1); // pns, ns, ns
                rawsetfield(L, -3, name.c_str()); // pns, ns
            }

            m_ref = luaL_ref(L, LUA_REGISTRYINDEX); // parent ns
            lua_pop(L, 1); // empty
        }
    } else if(ctx->GetKind() == ContextKinds::JavaScript) {
        if(name == "_G") {
            m_ns = JS_GetGlobalObject((JSContext*)(ctx->GetState()));
        } else {
            if(!m_parent) return;

            m_ns = JS_NewObject((JSContext*)(ctx->GetState()));
            JS_SetPropertyStr((JSContext*)(ctx->GetState()), m_parent->m_ns, name.c_str(), m_ns);
        }
    }
}

Namespace::~Namespace()
{
    if(!m_parent) {
        if(m_ctx->GetKind() == ContextKinds::Lua) {}
        else if(m_ctx->GetKind() == ContextKinds::JavaScript) {
            JS_FreeValue((JSContext*)m_ctx->GetState(), m_ns);
        }
    }
}

Namespace Namespace::beginNamespace(std::string nsName)
{
    return Namespace(m_ctx, nsName, this);
}

Namespace Namespace::endNamespace()
{
    if(!m_parent) return GetGlobalNamespace(m_ctx);

    return *m_parent;
}