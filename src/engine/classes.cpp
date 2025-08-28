#include "../Engine.h"
#include "classes.h"
#include "../CHelpers.h"
#include "../Stack.h"

#include <regex>

int LuaMemberCallbackIndex(lua_State* L, std::string str_key);
int LuaMemberCallbackNewIndex(lua_State* L, std::string str_key);
int LuaClassFunctionCall(lua_State* L);

bool str_startswith(std::string value, std::string starting)
{
    if (value.size() < starting.size())
        return false;
    return std::equal(starting.begin(), starting.end(), value.begin());
}

int LuaClassIndex(lua_State* L)
{
    auto ctx = GetContextByState(L);
    std::string class_name = lua_tostring(L, lua_upvalueindex(1));
    std::string member_name = Stack<std::string>::getLua(ctx, 2);

    std::string str_key = class_name + " " + member_name;
    if (ctx->GetClassFunctionCall(str_key)) {
        lua_pushstring(L, str_key.c_str());
        lua_pushcclosure(L, LuaClassFunctionCall, 1);
        return 1;
    }

    return LuaMemberCallbackIndex(L, str_key);
}

int LuaClassNewIndex(lua_State* L)
{
    auto ctx = GetContextByState(L);
    std::string class_name = lua_tostring(L, lua_upvalueindex(1));
    std::string member_name = Stack<std::string>::getLua(ctx, 2);
    std::string str_key = class_name + " " + member_name;

    return LuaMemberCallbackNewIndex(L, str_key);
}

int LuaClassFunctionCall(lua_State* L)
{
    std::string str_key = lua_tostring(L, lua_upvalueindex(1));
    auto ctx = GetContextByState(L);

    auto splits = str_split(str_key, " ");
    FunctionContext fctx(str_key, ctx->GetKind(), ctx, splits[0] != splits[1], splits[0] == splits[1], false);
    FunctionContext* fptr = &fctx;

    ClassData* data = nullptr;
    bool ignoreCustomReturn = false;

    auto functionPreCalls = ctx->GetClassFunctionPreCalls(str_key);
    auto functionPostCalls = ctx->GetClassFunctionPostCalls(str_key);
    bool stopExecution = false;

    if (splits[0] == splits[1])
    {
        data = new ClassData({}, splits[0], ctx);

        Stack<ClassData*>::pushLua(ctx, data);

        MarkDeleteOnGC(data);

        ignoreCustomReturn = true;
    }
    else
    {
        data = Stack<ClassData*>::getLua(ctx, 1);
    }

    if (!data) return luaL_error(L, "You can't call a member function from a garbage collected variable. Save the variable somewhere before using it.");

    for (void* func : functionPreCalls)
    {
        reinterpret_cast<ScriptingClassFunctionCallback>(func)(fptr, data);
        if (fctx.ShouldStopExecution())
        {
            stopExecution = true;
            break;
        }
    }

    if (!stopExecution) {
        void* func = ctx->GetClassFunctionCall(str_key);
        if (func) {
            ScriptingClassFunctionCallback cb = reinterpret_cast<ScriptingClassFunctionCallback>(func);
            cb(fptr, data);
        }

        for (void* func : functionPostCalls)
        {
            reinterpret_cast<ScriptingClassFunctionCallback>(func)(fptr, data);
            if (fctx.ShouldStopExecution()) break;
        }
    }

    int hasResult = (int)fctx.HasResult();

    if (ignoreCustomReturn) return 1;
    else if (hasResult != 0) fctx.pushLuaResult();
    return hasResult;
}

void DotnetClassCallback(EContext* ctx, CallContext& call_ctx, bool bypassClassCheck)
{
    std::string str_key = call_ctx.GetNamespace() + " " + call_ctx.GetFunction();
    auto splits = str_split(str_key, " ");
    FunctionContext fctx(str_key, ctx->GetKind(), ctx, &call_ctx, true, bypassClassCheck ? true : splits[0] == splits[1]);
    FunctionContext* fptr = &fctx;

    ClassData* data = nullptr;

    auto functionPreCalls = ctx->GetClassFunctionPreCalls(str_key);
    auto functionPostCalls = ctx->GetClassFunctionPostCalls(str_key);
    bool stopExecution = false;

    if (splits[0] == splits[1])
    {
        data = new ClassData({}, splits[0], ctx);
        
        void* classPtr = nullptr;
        int argType = call_ctx.GetArgumentType(1);
        uint64_t argValue = call_ctx.GetArgument<uint64_t>(1);

        switch (argType)
        {
            case 18: // Argument is ClassData*
            {
                classPtr = reinterpret_cast<ClassData*>(argValue)->GetData<void*>("class_ptr");
                break;
            }
            case 14: // Argument is std::string* (hex address)
            {
                classPtr = reinterpret_cast<void*>(std::stoull(*reinterpret_cast<std::string*>(argValue), nullptr, 16));
                break;
            }
        }

        data->SetData("class_ptr", classPtr);
        data->SetData("class_name", splits[0]);

        Stack<ClassData*>::pushDotnet(ctx, &call_ctx, data, true);
        MarkDeleteOnGC(data);
    }
    else
    {
        data = (ClassData*)call_ctx.GetArgument<ClassData*>(1);
    }

    for (void* func : functionPreCalls)
    {
        reinterpret_cast<ScriptingClassFunctionCallback>(func)(fptr, data);
        if (fctx.ShouldStopExecution())
        {
            stopExecution = true;
            break;
        }
    }

    if (!stopExecution) {
        void* func = ctx->GetClassFunctionCall(str_key);
        if (func) {
            ScriptingClassFunctionCallback cb = reinterpret_cast<ScriptingClassFunctionCallback>(func);
            cb(fptr, data);
        }

        for (void* func : functionPostCalls)
        {
            reinterpret_cast<ScriptingClassFunctionCallback>(func)(fptr, data);
            if (fctx.ShouldStopExecution()) break;
        }
    }
}

void AddScriptingClass(EContext* ctx, std::string class_name)
{
    if (ctx->GetKind() == ContextKinds::Lua)
    {
        auto L = ctx->GetLuaState();

        luaL_newmetatable(L, class_name.c_str());

        lua_pushstring(L, class_name.c_str());
        lua_pushcclosure(L, LuaClassIndex, 1);
        rawsetfield(L, -2, "__index");

        lua_pushstring(L, class_name.c_str());
        lua_pushcclosure(L, LuaClassNewIndex, 1);
        rawsetfield(L, -2, "__newindex");

        lua_pushcfunction(L, CHelpers::LuaGCFunction);
        rawsetfield(L, -2, "__gc");

        lua_pop(L, 1);
    }
}

void AddScriptingClassFunction(EContext* ctx, std::string class_name, std::string function_name, ScriptingClassFunctionCallback callback)
{
    std::string func_key = class_name + " " + function_name;
    ctx->AddClassFunctionCalls(func_key, reinterpret_cast<void*>(callback));

    if (ctx->GetKind() == ContextKinds::Lua)
    {
        auto L = ctx->GetLuaState();

        if (function_name == class_name)
        {
            lua_pushstring(L, func_key.c_str());
            lua_pushcclosure(L, LuaClassFunctionCall, 1);
            lua_setglobal(L, class_name.c_str());
        }
        else if (str_startswith(function_name, "__")) {
            luaL_getmetatable(L, class_name.c_str());

            lua_pushstring(L, func_key.c_str());
            lua_pushcclosure(L, LuaClassFunctionCall, 1);
            rawsetfield(L, -2, function_name.c_str());

            lua_pop(L, 1);
        }
    }
}

void AddScriptingClassFunctionPre(EContext* ctx, std::string class_name, std::string function_name, ScriptingClassFunctionCallback callback)
{
    auto func_key = class_name + " " + function_name;
    ctx->AddClassFunctionPreCalls(func_key, reinterpret_cast<void*>(callback));
}

void AddScriptingClassFunctionPost(EContext* ctx, std::string class_name, std::string function_name, ScriptingClassFunctionCallback callback)
{
    auto func_key = class_name + " " + function_name;
    ctx->AddClassFunctionPostCalls(func_key, reinterpret_cast<void*>(callback));
}

EValue CreateScriptingClassInstance(FunctionContext* context, std::string class_name, std::map<std::string, std::any> classdata)
{
    return CreateScriptingClassInstance(context->GetPluginContext(), class_name, classdata);
}

EValue CreateScriptingClassInstance(EContext* context, std::string class_name, std::map<std::string, std::any> classdata)
{
    if (context->GetKind() == ContextKinds::Lua)
    {
        auto L = context->GetLuaState();
        ClassData* data = new ClassData(classdata, class_name, context);
        Stack<ClassData*>::pushLua(context, data);
        MarkDeleteOnGC(data);

        return EValue(context, 0, false);
    }
    else if (context->GetKind() == ContextKinds::Dotnet) {
        auto data = new ClassData(classdata, class_name, context);
        return EValue(context, data, 18);
    }
    else
        return EValue(context);
}