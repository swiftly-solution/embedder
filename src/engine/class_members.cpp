#include "classes.h"
#include "../Engine.h"
#include "../CHelpers.h"
#include "../Stack.h"

#include <regex>

int LuaMemberCallbackIndex(lua_State* L, std::string str_key)
{
    auto ctx = GetContextByState(L);

    FunctionContext fctx(str_key, ctx->GetKind(), ctx, true, false, true);
    FunctionContext* fptr = &fctx;

    auto splits = str_split(str_key, " ");
    ClassData* data = Stack<ClassData*>::getLua(ctx, 1);
    if (!data) return luaL_error(L, "You can't get a member value from a garbage collected variable. Save the variable somewhere before using it.");

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
        void* func = ctx->GetClassMemberCalls(str_key).first;
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
    FunctionContext* fptr = &fctx;

    auto splits = str_split(str_key, " ");
    ClassData* data = Stack<ClassData*>::getLua(ctx, 1);
    if (!data) return luaL_error(L, "You can't set a member value from a garbage collected variable. Save the variable somewhere before using it.");

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
        void* func = ctx->GetClassMemberCalls(str_key).second;
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

JSValue JSClassMemberIndex(JSContext* L, std::string str_key, JSValue* argv)
{
    auto ctx = GetContextByState(L);

    FunctionContext fctx(str_key, ctx->GetKind(), ctx, argv, 0);
    FunctionContext* fptr = &fctx;

    ClassData* data = Stack<ClassData*>::getJS(ctx, argv[2]);

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
        void* func = ctx->GetClassMemberCalls(str_key).first;
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

JSValue JSClassMemberNewIndex(JSContext* L, std::string str_key, JSValue* argv)
{
    auto ctx = GetContextByState(L);

    JSValue args[1] = { argv[2] };
    FunctionContext fctx(str_key, ctx->GetKind(), ctx, args, 1);
    FunctionContext* fptr = &fctx;

    ClassData* data = Stack<ClassData*>::getJS(ctx, argv[3]);

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
        void* func = ctx->GetClassMemberCalls(str_key).second;
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
    return Stack<bool>::pushJS(ctx, true);
}

void AddScriptingClassMember(EContext* ctx, std::string class_name, std::string member_name, ScriptingClassFunctionCallback callback_get, ScriptingClassFunctionCallback callback_set)
{
    std::string func_key = class_name + " " + member_name;
    ctx->AddClassMemberCalls(func_key, { reinterpret_cast<void*>(callback_get), reinterpret_cast<void*>(callback_set) });
}

void AddScriptingClassMemberPre(EContext* ctx, std::string class_name, std::string member_name, ScriptingClassFunctionCallback callback_get, ScriptingClassFunctionCallback callback_set)
{
    auto func_key = class_name + " " + member_name;
    ctx->AddClassMemberPreCalls(func_key, { reinterpret_cast<void*>(callback_get), reinterpret_cast<void*>(callback_set) });
}

void AddScriptingClassMemberPost(EContext* ctx, std::string class_name, std::string member_name, ScriptingClassFunctionCallback callback_get, ScriptingClassFunctionCallback callback_set)
{
    auto func_key = class_name + " " + member_name;
    ctx->AddClassMemberPostCalls(func_key, { reinterpret_cast<void*>(callback_get), reinterpret_cast<void*>(callback_set) });
}