#include "Context.h"
#include "Exception.h"
#include "Helpers.h"
#include "CHelpers.h"

#include <set>
#include <filesystem>
#include <regex>

static const luaL_Reg lualibs[] = {
    {"_G", luaopen_base},
    {LUA_TABLIBNAME, luaopen_table},
    {LUA_STRLIBNAME, luaopen_string},
    {LUA_MATHLIBNAME, luaopen_math},
    {LUA_DBLIBNAME, luaopen_debug},
    {LUA_COLIBNAME, luaopen_coroutine},
    {LUA_UTF8LIBNAME, luaopen_utf8},
    {LUA_IOLIBNAME, luaopen_io},
    {LUA_OSLIBNAME, luaopen_os},
    {NULL, NULL},
};

JSRuntime* rt = nullptr;

void CheckAndPopulateRegexFunctions(std::map<std::string, std::vector<void*>>& validCalls, std::map<std::string, std::vector<void*>>& calls, std::map<std::string, void*>& functions, std::string function_key, bool forceRegenerate = false) {
    for (auto it = functions.begin(); it != functions.end(); ++it) {
        try {
            if (std::regex_search(it->first, std::regex(function_key.c_str(), std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::nosubs))) {
                validCalls[it->first].clear();
                for (void* func : calls[function_key]) validCalls[it->first].push_back(func);
            }
        }
        catch (std::regex_error& e) {}
    }
}

void CheckAndPopulateRegexFunctionsPair(std::map<std::string, std::vector<std::pair<void*, void*>>>& validCalls, std::map<std::string, std::vector<std::pair<void*, void*>>>& calls, std::map<std::string, std::pair<void*, void*>>& functions, std::string function_key, bool forceRegenerate = false) {
    for (auto it = functions.begin(); it != functions.end(); ++it) {
        try {
            if (std::regex_search(it->first, std::regex(function_key.c_str(), std::regex_constants::ECMAScript | std::regex_constants::optimize | std::regex_constants::nosubs))) {
                validCalls[it->first].clear();
                for (std::pair<void*, void*> func : calls[function_key]) validCalls[it->first].push_back(func);
            }
        }
        catch (std::regex_error& e) {}
    }
}

EContext::EContext(ContextKinds kind)
{
    m_kind = kind;

    if (kind == ContextKinds::Lua)
    {
        auto state = luaL_newstate();
        m_state = (void*)state;

        const luaL_Reg* lib = lualibs;
        for (; lib->func; lib++)
            RegisterLuaLib(lib->name, lib->func);

        lua_pushglobaltable(state);                 // _G
        lua_pushlightuserdata(state, (void*)this); // _G, ud
        lua_rawsetp(state, -2, getContextKey());    // _G[key] = ud. _G
        lua_pop(state, 1);                          // empty
    }
    else if (kind == ContextKinds::JavaScript)
    {
        JSRuntime* rt = JS_NewRuntime();
        JS_SetMaxStackSize(rt, 0);
        JSContext* ctx = JS_NewContext(rt);

        JSValue global_obj = JS_GetGlobalObject(ctx);
        JSValue console_obj = JS_NewObject(ctx);

        JS_SetPropertyStr(ctx, console_obj, "log",
                          JS_NewCFunction(ctx, CHelpers::js_print_to_console, "log", 1));
        JS_SetPropertyStr(ctx, global_obj, "console", console_obj);

        JS_FreeValue(ctx, global_obj);

        JS_SetContextOpaque(ctx, (void*)this);

        m_state = (void*)ctx;
    }

    EException::Enable(m_state, m_kind);
}

EContext::~EContext()
{
    for (auto it = mappedValues.begin(); it != mappedValues.end(); ++it)
        delete (*it);

    mappedValues.clear();

    if (m_kind == ContextKinds::Lua)
    {
        lua_close((lua_State*)m_state);
    }
    else if (m_kind == ContextKinds::JavaScript)
    {
        JSContext* ctx = (JSContext*)m_state;
        JSRuntime* rt = JS_GetRuntime(ctx);
        JS_FreeContext(ctx);

        if (auto rtopaque = JS_GetRuntimeOpaque(rt); rtopaque != nullptr) {
            delete ((std::set<ClassData*>*)rtopaque);
        }

        JS_FreeRuntime(rt);
    }
}

ContextKinds EContext::GetKind()
{
    return m_kind;
}

void EContext::RegisterLuaLib(const char* libName, lua_CFunction func)
{
    luaL_requiref((lua_State*)m_state, libName, func, 1);
    lua_pop((lua_State*)m_state, 1);
}

int64_t EContext::GetMemoryUsage()
{
    if (m_kind == ContextKinds::Lua)
    {
        int64_t count = lua_gc((lua_State*)m_state, LUA_GCCOUNT, 0);
        count *= 1024;
        count += lua_gc((lua_State*)m_state, LUA_GCCOUNTB, 0);
        return count;
    }
    else if (m_kind == ContextKinds::JavaScript)
    {
        JSMemoryUsage stats;
        JS_ComputeMemoryUsage(JS_GetRuntime((JSContext*)m_state), &stats);
        return stats.memory_used_size;
    }
    else
        return 0;
}

int EContext::RunCode(std::string code)
{
    if (m_kind == ContextKinds::Lua)
    {
        int cd = (luaL_dostring((lua_State*)m_state, code.c_str()));
        if (cd != 0)
            EException::Throw(EException(GetState(), GetKind(), cd));
        return cd;
    }
    else if (m_kind == ContextKinds::JavaScript)
    {
        auto res = JS_Eval((JSContext*)m_state, code.c_str(), code.length(), "runcode.js", JS_EVAL_TYPE_GLOBAL);
        bool isException = JS_IsException(res);
        JS_FreeValue((JSContext*)m_state, res);
        return (int)isException;
    }
    else
        return 0;
}

std::string files_Read(std::string path)
{
    if (!std::filesystem::exists(path))
        return "";

    auto fp = std::fopen(path.c_str(), "rb");
    std::string s;
    std::fseek(fp, 0u, SEEK_END);
    auto size = std::ftell(fp);
    std::fseek(fp, 0u, SEEK_SET);
    s.resize(size);
    std::fread(&s[0], 1u, size, fp);
    std::fclose(fp);
    return s;
}

int EContext::RunFile(std::string path)
{
    if (m_kind == ContextKinds::Lua)
    {
        int cd = (luaL_dofile((lua_State*)m_state, path.c_str()));
        if (cd != 0)
            EException::Throw(EException(GetState(), GetKind(), cd));
        return cd;
    }
    else if (m_kind == ContextKinds::JavaScript)
    {
        std::string code = files_Read(path);
        auto res = JS_Eval((JSContext*)m_state, code.c_str(), code.length(), path.c_str(), JS_EVAL_TYPE_GLOBAL);
        bool isException = JS_IsException(res);
        JS_FreeValue((JSContext*)m_state, res);
        return (int)isException;
    }
    else
        return 0;
}

void EContext::PushValue(EValue* val)
{
    if (mappedValues.find(val) != mappedValues.end())
        return;
    mappedValues.insert(val);
}

void EContext::PopValue(EValue* val)
{
    if (mappedValues.find(val) == mappedValues.end())
        return;
    mappedValues.erase(val);
}

void* EContext::GetState()
{
    return m_state;
}

lua_State* EContext::GetLuaState()
{
    return (lua_State*)m_state;
}

JSContext* EContext::GetJSState()
{
    return (JSContext*)m_state;
}

JSClassID* EContext::GetClassID(std::string className)
{
    if (classIDs.find(className) == classIDs.end())
        classIDs.insert({ className, 0 });

    return &classIDs[className];
}

std::string EContext::GetClsName(JSClassID id)
{
    for (auto it = classIDs.begin(); it != classIDs.end(); ++it)
        if (it->second == id)
            return it->first;

    return "";
}

void EContext::AddFunctionCall(std::string key, void* val)
{
    functionCalls.insert_or_assign(key, val);
}

void* EContext::GetFunctionCall(std::string key)
{
    if (functionCalls.find(key) == functionCalls.end())
        return nullptr;
    return functionCalls[key];
}

void EContext::AddFunctionPreCall(std::string key, void* val)
{
    if (functionPreCalls.find(key) == functionPreCalls.end())
        functionPreCalls.insert({ key, {} });

    functionPreCalls[key].push_back(val);
    CheckAndPopulateRegexFunctions(functionValidPreCalls, functionPreCalls, functionCalls, key, true);
}

std::vector<void*> EContext::GetFunctionPreCalls(std::string str_key)
{
    return functionValidPreCalls[str_key];
}

void EContext::AddFunctionPostCall(std::string key, void* val)
{
    if (functionPostCalls.find(key) == functionPostCalls.end())
        functionPostCalls.insert({ key, {} });

    functionPostCalls[key].push_back(val);
    CheckAndPopulateRegexFunctions(functionValidPostCalls, functionPostCalls, functionCalls, key, true);
}

std::vector<void*> EContext::GetFunctionPostCalls(std::string str_key)
{
    return functionValidPostCalls[str_key];
}

void EContext::AddClassFunctionCalls(std::string key, void* val)
{
    classFunctionCalls.insert_or_assign(key, val);
}

void* EContext::GetClassFunctionCall(std::string key)
{
    if (classFunctionCalls.find(key) == classFunctionCalls.end())
        return nullptr;
    return classFunctionCalls[key];
}

void EContext::AddClassFunctionPreCalls(std::string key, void* val)
{
    if (classFunctionPreCalls.find(key) == classFunctionPreCalls.end())
        classFunctionPreCalls.insert({ key, {} });

    classFunctionPreCalls[key].push_back(val);
    CheckAndPopulateRegexFunctions(classFunctionValidPreCalls, classFunctionPreCalls, classFunctionCalls, key, true);
}

std::vector<void*> EContext::GetClassFunctionPreCalls(std::string func_key)
{
    return classFunctionValidPreCalls[func_key];
}

void EContext::AddClassFunctionPostCalls(std::string key, void* val)
{
    if (classFunctionPostCalls.find(key) == classFunctionPostCalls.end())
        classFunctionPostCalls.insert({ key, {} });

    classFunctionPostCalls[key].push_back(val);
    CheckAndPopulateRegexFunctions(classFunctionValidPostCalls, classFunctionPostCalls, classFunctionCalls, key, true);
}

std::vector<void*> EContext::GetClassFunctionPostCalls(std::string func_key)
{
    return classFunctionValidPostCalls[func_key];
}

void EContext::AddClassMemberCalls(std::string key, std::pair<void*, void*> val)
{
    classMemberCalls.insert_or_assign(key, val);
}

std::pair<void*, void*> EContext::GetClassMemberCalls(std::string key)
{
    if (classMemberCalls.find(key) == classMemberCalls.end())
        return { nullptr, nullptr };
    return classMemberCalls[key];
}

void EContext::AddClassMemberPreCalls(std::string key, std::pair<void*, void*> val)
{
    if (classMemberPreCalls.find(key) == classMemberPreCalls.end())
        classMemberPreCalls.insert({ key, {} });

    classMemberPreCalls[key].push_back(val);
    CheckAndPopulateRegexFunctionsPair(classMemberValidPreCalls, classMemberPreCalls, classMemberCalls, key, true);
}

std::vector<std::pair<void*, void*>> EContext::GetClassMemberPreCalls(std::string func_key)
{
    return classMemberValidPreCalls[func_key];
}

void EContext::AddClassMemberPostCalls(std::string key, std::pair<void*, void*> val)
{
    if (classMemberPostCalls.find(key) == classMemberPostCalls.end())
        classMemberPostCalls.insert({ key, {} });

    classMemberPostCalls[key].push_back(val);
    CheckAndPopulateRegexFunctionsPair(classMemberValidPostCalls, classMemberPostCalls, classMemberCalls, key, true);
}

std::vector<std::pair<void*, void*>> EContext::GetClassMemberPostCalls(std::string func_key)
{
    return classMemberValidPostCalls[func_key];
}

EContext* GetContextByState(JSContext* ctx)
{
    return (EContext*)JS_GetContextOpaque(ctx);
}

EContext* GetContextByState(lua_State* ctx)
{
    lua_pushglobaltable(ctx);
    lua_rawgetp(ctx, -1, getContextKey());
    auto ud = lua_touserdata(ctx, -1);
    lua_pop(ctx, 2);
    return (EContext*)ud;
}