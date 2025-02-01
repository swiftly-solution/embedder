#ifndef _embedder_internal_helpers_h
#define _embedder_internal_helpers_h

#include <lua.hpp>

inline int get_length(lua_State* L, int idx)
{
    lua_len(L, idx);
    int len = int(luaL_checknumber(L, -1));
    lua_pop(L, 1);
    return len;
}

#endif