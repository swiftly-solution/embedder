#include "../Engine.h"
#include "../CHelpers.h"
#include "../Helpers.h"

#include <regex>

int LuaFunctionCallback(lua_State *L)
{
    std::string str_key = lua_tostring(L, lua_upvalueindex(1));
    auto ctx = GetContextByState(L);

    FunctionContext fctx(str_key, ctx->GetKind(), ctx);
    FunctionContext *fptr = &fctx;
    
    auto functionPreCalls = ctx->GetFunctionPreCalls(str_key);
    auto functionPostCalls = ctx->GetFunctionPostCalls(str_key);
    bool stopExecution = false;
    
    for (void* func : functionPreCalls) {
        reinterpret_cast<ScriptingFunctionCallback>(func)(fptr);
        if (fctx.ShouldStopExecution())
        {
            stopExecution = true;
            break;
        }
    }

    if (!stopExecution) {
        void *func = ctx->GetFunctionCall(str_key);
        if (func) {
            ScriptingFunctionCallback cb = reinterpret_cast<ScriptingFunctionCallback>(func);
            cb(fptr);
        }
        
        for (void* func : functionPostCalls) {
            reinterpret_cast<ScriptingFunctionCallback>(func)(fptr);
            if (fctx.ShouldStopExecution()) break;
        }
    }

    int hasResult = (int)fctx.HasResult();
    if (hasResult != 0) fctx.pushLuaResult();
    return hasResult;
}

JSValue JSFunctionCallback(JSContext *L, JSValue this_val, int argc, JSValue *argv, int magic, JSValue *func_data)
{
    auto ctx = GetContextByState(L);
    std::string str_key = Stack<std::string>::getJS(ctx, func_data[0]);
    FunctionContext fctx(str_key, ctx->GetKind(), ctx, argv, argc);
    FunctionContext *fptr = &fctx;

    auto functionPreCalls = ctx->GetFunctionPreCalls(str_key);
    auto functionPostCalls = ctx->GetFunctionPostCalls(str_key);
    bool stopExecution = false;

    for (void* func : functionPreCalls) {
        reinterpret_cast<ScriptingFunctionCallback>(func)(fptr);
        if (fctx.ShouldStopExecution())
        {
            stopExecution = true;
            break;
        }
    }

    if (!stopExecution) {
        void *func = ctx->GetFunctionCall(str_key);
        if (func) {
            ScriptingFunctionCallback cb = reinterpret_cast<ScriptingFunctionCallback>(func);
            cb(fptr);
        }
    
        for (void* func : functionPostCalls) {
            reinterpret_cast<ScriptingFunctionCallback>(func)(fptr);
            if (fctx.ShouldStopExecution()) break;
        }
    }

    if (fctx.HasResult()) return fctx.pushJSResult();
    return JS_UNDEFINED;
}

void AddScriptingFunction(EContext *ctx, std::string namespace_path, std::string function_name, ScriptingFunctionCallback callback)
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

        auto func_key = namespace_path + " " + function_name;
        ctx->AddFunctionCall(func_key, reinterpret_cast<void *>(callback));

        lua_pushstring(L, func_key.c_str());
        lua_pushcclosure(L, LuaFunctionCallback, 1);
        rawsetfield(L, -2, function_name.c_str());

        lua_pop(L, pop_values);
    }
    else if (ctx->GetKind() == ContextKinds::JavaScript)
    {
        auto L = ctx->GetJSState();
        auto ns = JS_GetGlobalObject(L);
        auto gns = ns;

        if (namespace_path != "_G")
        {
            auto paths = str_split(namespace_path, ".");
            for (auto path : paths)
            {
                if (path == "_G")
                    break;

                JSAtom at = JS_ValueToAtom(L, Stack<std::string>::pushJS(ctx, path));

                if (JS_HasProperty(L, ns, at))
                {
                    ns = JS_GetProperty(L, ns, at);
                }
                else
                {
                    auto val = JS_NewObject(L);
                    JS_SetProperty(L, ns, at, val);
                    ns = val;
                }

                JS_FreeAtom(L, at);
            }
        }

        auto func_key = namespace_path + " " + function_name;
        ctx->AddFunctionCall(func_key, reinterpret_cast<void *>(callback));

        std::vector<JSValue> vals = {Stack<std::string>::pushJS(ctx, func_key)};
        JS_SetPropertyStr(L, ns, function_name.c_str(), JS_NewCFunctionData(L, JSFunctionCallback, 0, 1, 1, vals.data()));

        JS_FreeValue(L, gns);
    }
}

void AddScriptingFunctionPre(EContext *ctx, std::string namespace_path, std::string function_name, ScriptingFunctionCallback callback)
{
    auto func_key = namespace_path + " " + function_name;
    ctx->AddFunctionPreCall(func_key, reinterpret_cast<void *>(callback));
}

void AddScriptingFunctionPost(EContext *ctx, std::string namespace_path, std::string function_name, ScriptingFunctionCallback callback)
{
    auto func_key = namespace_path + " " + function_name;
    ctx->AddFunctionPostCall(func_key, reinterpret_cast<void *>(callback));
}