#ifndef _embedding_internal_context_h
#define _embedding_internal_context_h

#include <lua.hpp>
#include <quickjs.h>

#include "../Config.h"
#include "../ContextKinds.h"
#include "Exception.h"

class EContext
{
private:
    void* m_state;
    ContextKinds m_kind;
public:
    EContext(ContextKinds kind);
    ~EContext();

    void RegisterLuaLib(const char* libName, lua_CFunction func);

    ContextKinds GetKind();
    int64_t GetMemoryUsage();
    void* GetState();
};

#endif