#ifndef _embedding_internal_context_h
#define _embedding_internal_context_h

#include <set>
#include <map>
#include <string>

#include <lua.hpp>
#include <quickjs.h>

#include "../Config.h"
#include "../ContextKinds.h"

class EValue;

class EContext
{
private:
    void* m_state;
    ContextKinds m_kind;
    std::set<EValue*> mappedValues;
    std::map<std::string, JSClassID> classIDs;

public:
    EContext(ContextKinds kind);
    ~EContext();

    void RegisterLuaLib(const char* libName, lua_CFunction func);

    ContextKinds GetKind();
    int64_t GetMemoryUsage();
    void* GetState();

    int RunCode(std::string code);
    int RunFile(std::string path);

    void PushValue(EValue* val);
    void PopValue(EValue* val);

    JSClassID* GetClassID(std::string className);
    std::string GetClsName(JSClassID id);
};

EContext* GetContextByState(JSContext* ctx);
EContext* GetContextByState(lua_State* ctx);

#endif