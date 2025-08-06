#ifndef _embedder_helpers_h
#define _embedder_helpers_h

#include <lua.hpp>
#include <cassert>
#include <string>
#include <vector>

#include "Context.h"

inline int get_length(lua_State* L, int idx)
{
    lua_len(L, idx);
    int len = int(luaL_checknumber(L, -1));
    lua_pop(L, 1);
    return len;
}

inline void rawgetfield(lua_State* L, int index, char const* key)
{
    assert(lua_istable(L, index));
    index = lua_absindex(L, index);
    lua_pushstring(L, key);
    lua_rawget(L, index);
}

inline void rawsetfield(lua_State* L, int index, char const* key)
{
    assert(lua_istable(L, index));
    index = lua_absindex(L, index);
    lua_pushstring(L, key);
    lua_insert(L, -2);
    lua_rawset(L, index);
}

inline const void* getPropgetKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0x6e7);
#endif
}

inline const void* getParentKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0xdad);
#endif
}

inline const void* getPropsetKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0x5e7);
#endif
}

inline const void* getContextKey()
{
#ifdef _NDEBUG
    static char value;
    return &value;
#else
    return reinterpret_cast<void*>(0xbee);
#endif
}

inline std::vector<std::string> str_split(std::string s, std::string delimiter)
{
    if (s.size() == 0) return {};
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
    {
        token = s.substr(pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back(token);
    }

    res.push_back(s.substr(pos_start));
    return res;
}

#endif