#include "classes.h"
#include "../Engine.h"
#include "../CHelpers.h"
#include "../Stack.h"

#include <regex>

int LuaMemberCallbackIndex(lua_State* L, std::string str_key)
{
    auto ctx = GetContextByState(L);

    FunctionContext fctx(str_key, ctx->GetKind(), ctx, true, false, true);
    FunctionContext *fptr = &fctx;

    auto splits = str_split(str_key, " ");
    ClassData *data = *(ClassData **)luaL_checkudata(L, 1, splits[0].c_str());

    auto functionPreCalls = ctx->GetClassMemberPreCalls(str_key);
    auto functionPostCalls = ctx->GetClassMemberPostCalls(str_key);
    bool stopExecution = false;

    for (auto func : functionPreCalls)
    {
        reinterpret_cast<ScriptingClassFunctionCallback>(func.first)(fptr, data);
        if (fctx.ShouldStopExecution())
        {
            stopExecution = true;
            break;
        }
    }

    if(!stopExecution) {
        void *func = ctx->GetClassMemberCalls(str_key).first;
        if (func) {
            ScriptingClassFunctionCallback cb = reinterpret_cast<ScriptingClassFunctionCallback>(func);
            cb(fptr, data);
        }

        for (auto func : functionPostCalls)
        {
            reinterpret_cast<ScriptingClassFunctionCallback>(func.first)(fptr, data);
            if (fctx.ShouldStopExecution()) break;
        }
    }

    int hasResult = (int)fctx.HasResult();
    if (hasResult != 0)
    {
        fctx.pushLuaResult();
    }
    return hasResult;
}

int LuaMemberCallbackNewIndex(lua_State* L, std::string str_key)
{
    auto ctx = GetContextByState(L);

    FunctionContext fctx(str_key, ctx->GetKind(), ctx, true, false, true);
    FunctionContext *fptr = &fctx;

    auto splits = str_split(str_key, " ");
    ClassData *data = *(ClassData **)luaL_checkudata(L, 1, splits[0].c_str());

    auto functionPreCalls = ctx->GetClassMemberPreCalls(str_key);
    auto functionPostCalls = ctx->GetClassMemberPostCalls(str_key);
    bool stopExecution = false;

    for (auto func : functionPreCalls)
    {
        reinterpret_cast<ScriptingClassFunctionCallback>(func.second)(fptr, data);
        if (fctx.ShouldStopExecution())
        {
            stopExecution = true;
            break;
        }
    }

    if (!stopExecution) {
        void *func = ctx->GetClassMemberCalls(str_key).second;
        if (func) {
            ScriptingClassFunctionCallback cb = reinterpret_cast<ScriptingClassFunctionCallback>(func);
            cb(fptr, data);
        }
    
        for (auto func : functionPostCalls)
        {
            reinterpret_cast<ScriptingClassFunctionCallback>(func.second)(fptr, data);
            if (fctx.ShouldStopExecution()) break;
        }
    }

    int hasResult = (int)fctx.HasResult();
    if (hasResult != 0)
    {
        fctx.pushLuaResult();
    }
    return hasResult;
}

JSValue JSMemberGetCallback(JSContext *L, JSValue this_val, int argc, JSValue *argv, int magic, JSValue *func_data)
{
    auto ctx = GetContextByState(L);
    std::string str_key = Stack<std::string>::getJS(ctx, func_data[0]);

    FunctionContext fctx(str_key, ctx->GetKind(), ctx, argv, argc);
    FunctionContext *fptr = &fctx;

    ClassData *data = (ClassData *)JS_GetOpaque(this_val, *ctx->GetClassID(str_split(str_key, " ")[0]));

    auto functionPreCalls = ctx->GetClassMemberPreCalls(str_key);
    auto functionPostCalls = ctx->GetClassMemberPostCalls(str_key);
    bool stopExecution = false;

    for (auto func : functionPreCalls)
    {
        reinterpret_cast<ScriptingClassFunctionCallback>(func.first)(fptr, data);
        if (fctx.ShouldStopExecution())
        {
            stopExecution = true;
            break;
        }
    }

    if (!stopExecution) {
        void *func = ctx->GetClassMemberCalls(str_key).first;
        if (func) {
            ScriptingClassFunctionCallback cb = reinterpret_cast<ScriptingClassFunctionCallback>(func);
            cb(fptr, data);
        }
    
        for (auto func : functionPostCalls)
        {
            reinterpret_cast<ScriptingClassFunctionCallback>(func.first)(fptr, data);
            if (fctx.ShouldStopExecution()) break;
        }
    }

    if (fctx.HasResult())
        return fctx.pushJSResult();
    return JS_UNDEFINED;
}

JSValue JSMemberSetCallback(JSContext *L, JSValue this_val, int argc, JSValue *argv, int magic, JSValue *func_data)
{
    auto ctx = GetContextByState(L);
    std::string str_key = Stack<std::string>::getJS(ctx, func_data[0]);

    FunctionContext fctx(str_key, ctx->GetKind(), ctx, argv, argc);
    FunctionContext *fptr = &fctx;

    ClassData *data = (ClassData *)JS_GetOpaque(this_val, *ctx->GetClassID(str_split(str_key, " ")[0]));

    auto functionPreCalls = ctx->GetClassMemberPreCalls(str_key);
    auto functionPostCalls = ctx->GetClassMemberPostCalls(str_key);
    bool stopExecution = false;

    for (auto func : functionPreCalls)
    {
        reinterpret_cast<ScriptingClassFunctionCallback>(func.second)(fptr, data);
        if (fctx.ShouldStopExecution())
        {
            stopExecution = true;
            break;
        }
    }

    if (!stopExecution) {
        void *func = ctx->GetClassMemberCalls(str_key).second;
        if (func) {
            ScriptingClassFunctionCallback cb = reinterpret_cast<ScriptingClassFunctionCallback>(func);
            cb(fptr, data);
        }
    
        for (auto func : functionPostCalls)
        {
            reinterpret_cast<ScriptingClassFunctionCallback>(func.second)(fptr, data);
            if (fctx.ShouldStopExecution()) break;
        }
    }

    if (fctx.HasResult())
        return fctx.pushJSResult();
    return JS_UNDEFINED;
}

void AddScriptingClassMember(EContext *ctx, std::string class_name, std::string member_name, ScriptingClassFunctionCallback callback_get, ScriptingClassFunctionCallback callback_set)
{
    if(ctx->GetKind() == ContextKinds::Lua) {
        auto L = ctx->GetLuaState();

        std::string func_key = class_name + " " + member_name;
        ctx->AddClassMemberCalls(func_key, {reinterpret_cast<void *>(callback_get), reinterpret_cast<void *>(callback_set)});
    } else if(ctx->GetKind() == ContextKinds::JavaScript) {
        auto L = ctx->GetJSState();

        std::string func_key = class_name + " " + member_name;
        ctx->AddClassMemberCalls(func_key, {reinterpret_cast<void *>(callback_get), reinterpret_cast<void *>(callback_set)});

        auto &proto = ctx->GetClassPrototype(class_name);
        JSAtom atom = JS_NewAtom(L, member_name.c_str());

        std::vector<JSValue> vals = {Stack<std::string>::pushJS(ctx, func_key)};
        JS_DefinePropertyGetSet(L, proto, atom, 
            JS_NewCFunctionData(L, JSMemberGetCallback, 0, 0, 1, vals.data()),
            JS_NewCFunctionData(L, JSMemberSetCallback, 1, 0, 1, vals.data()),
            0
        );

        JS_FreeAtom(L, atom);
    }
}

void AddScriptingClassMemberPre(EContext *ctx, std::string class_name, std::string member_name, ScriptingClassFunctionCallback callback_get, ScriptingClassFunctionCallback callback_set)
{
    auto func_key = class_name + " " + member_name;
    ctx->AddClassMemberPreCalls(func_key, {reinterpret_cast<void *>(callback_get), reinterpret_cast<void *>(callback_set)});
}

void AddScriptingClassMemberPost(EContext *ctx, std::string class_name, std::string member_name, ScriptingClassFunctionCallback callback_get, ScriptingClassFunctionCallback callback_set)
{
    auto func_key = class_name + " " + member_name;
    ctx->AddClassMemberPostCalls(func_key, {reinterpret_cast<void *>(callback_get), reinterpret_cast<void *>(callback_set)});
}