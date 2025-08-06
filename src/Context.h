#ifndef _embedder_context_h
#define _embedder_context_h

#include <set>
#include <map>
#include <string>

#include <lua.hpp>
#include "dotnet/invoker.h"
#include <vector>

#include "ContextKinds.h"

class EValue;

class EContext
{
private:
    void* m_state;
    ContextKinds m_kind;
    std::set<EValue*> mappedValues;

    std::map<std::string, void*> functionCalls;

    std::map<std::string, std::vector<void*>> functionPreCalls;
    std::map<std::string, std::vector<void*>> functionPostCalls;
    std::map<std::string, std::vector<void*>> functionValidPreCalls;
    std::map<std::string, std::vector<void*>> functionValidPostCalls;

    std::map<std::string, void*> classFunctionCalls;
    std::map<std::string, std::vector<void*>> classFunctionPreCalls;
    std::map<std::string, std::vector<void*>> classFunctionPostCalls;
    std::map<std::string, std::vector<void*>> classFunctionValidPreCalls;
    std::map<std::string, std::vector<void*>> classFunctionValidPostCalls;

    std::map<std::string, std::pair<void*, void*>> classMemberCalls;
    std::map<std::string, std::vector<std::pair<void*, void*>>> classMemberPreCalls;
    std::map<std::string, std::vector<std::pair<void*, void*>>> classMemberPostCalls;
    std::map<std::string, std::vector<std::pair<void*, void*>>> classMemberValidPreCalls;
    std::map<std::string, std::vector<std::pair<void*, void*>>> classMemberValidPostCalls;

public:
    EContext(ContextKinds kind);
    ~EContext();

    void RegisterLuaLib(const char* libName, lua_CFunction func);

    ContextKinds GetKind();
    int64_t GetMemoryUsage();
    void* GetState();
    lua_State* GetLuaState();

    int RunFile(std::string path);

    void PushValue(EValue* val);
    void PopValue(EValue* val);

    void AddFunctionCall(std::string key, void* val);
    void* GetFunctionCall(std::string key);

    void AddFunctionPreCall(std::string key, void* val);
    std::vector<void*> GetFunctionPreCalls(std::string function_key);

    void AddFunctionPostCall(std::string key, void* val);
    std::vector<void*> GetFunctionPostCalls(std::string function_key);

    void AddClassFunctionCalls(std::string key, void* val);
    void* GetClassFunctionCall(std::string key);

    void AddClassFunctionPreCalls(std::string key, void* val);
    std::vector<void*> GetClassFunctionPreCalls(std::string function_key);

    void AddClassFunctionPostCalls(std::string key, void* val);
    std::vector<void*> GetClassFunctionPostCalls(std::string function_key);

    void AddClassMemberCalls(std::string key, std::pair<void*, void*> val);
    std::pair<void*, void*> GetClassMemberCalls(std::string key);

    void AddClassMemberPreCalls(std::string key, std::pair<void*, void*> val);
    std::vector<std::pair<void*, void*>> GetClassMemberPreCalls(std::string function_key);

    void AddClassMemberPostCalls(std::string key, std::pair<void*, void*> val);
    std::vector<std::pair<void*, void*>> GetClassMemberPostCalls(std::string function_key);
};

EContext* GetContextByState(lua_State* ctx);

#endif