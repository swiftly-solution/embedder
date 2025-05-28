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

JSValue JSClassCallback(JSContext* L, JSValue this_val, int argc, JSValue* argv, int magic, JSValue* func_data);
JSValue JSClassMemberIndex(JSContext* L, std::string str_key, JSValue* argv);
JSValue JSClassMemberNewIndex(JSContext* L, std::string str_key, JSValue* argv);

JSValue JSClassIndex(JSContext* L, JSValue this_val, int argc, JSValue* argv)
{
    auto ctx = GetContextByState(L);

    auto classData = Stack<ClassData*>::getJS(ctx, argv[2]);
    if (!classData) return JS_ThrowInternalError(L, "You can't access a member from a garbage collected variable. Save the variable somewhere before using it.");

    std::string class_name = classData->GetClassname();
    std::string member_name = Stack<std::string>::getJS(ctx, argv[1]);

    std::string str_key = class_name + " " + member_name;
    if (ctx->GetClassFunctionCall(str_key)) {
        JSValue v[1] = { Stack<std::string>::pushJS(ctx, str_key) };
        JSValue func = JS_NewCFunctionData(L, JSClassCallback, 0, 1, 1, (JSValue*)(v));
        return func;
    }

    return JSClassMemberIndex(L, str_key, argv);
}

JSValue JSClassNewIndex(JSContext* L, JSValue this_val, int argc, JSValue* argv)
{
    auto ctx = GetContextByState(L);

    auto classData = Stack<ClassData*>::getJS(ctx, argv[3]);
    if (!classData) return JS_ThrowInternalError(L, "You can't set a member from a garbage collected variable. Save the variable somewhere before using it.");

    std::string class_name = classData->GetClassname();
    std::string member_name = Stack<std::string>::getJS(ctx, argv[1]);
    std::string str_key = class_name + " " + member_name;

    return JSClassMemberNewIndex(L, str_key, argv);
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

JSValue JSClassCallback(JSContext* L, JSValue this_val, int argc, JSValue* argv, int magic, JSValue* func_data)
{
    auto ctx = GetContextByState(L);
    std::string str_key = Stack<std::string>::getJS(ctx, func_data[0]);

    FunctionContext fctx(str_key, ctx->GetKind(), ctx, argv, argc);
    FunctionContext* fptr = &fctx;
    ClassData* data = nullptr;
    bool ignoreCustomReturn = false;

    auto functionPreCalls = ctx->GetClassFunctionPreCalls(str_key);
    auto functionPostCalls = ctx->GetClassFunctionPostCalls(str_key);
    bool stopExecution = false;
    JSValue ret = JS_UNDEFINED;

    auto splits = str_split(str_key, " ");
    if (splits[0] == splits[1])
    {
        data = new ClassData({}, splits[0], ctx);
        ret = Stack<ClassData*>::pushJS(ctx, data);

        if (JS_IsException(ret)) {
            delete data;
            return ret;
        }
        MarkDeleteOnGC(data);
        ignoreCustomReturn = true;
    }
    else
    {
        data = (ClassData*)JS_GetOpaque(this_val, *ctx->GetClassID(splits[0]));
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

    if (ignoreCustomReturn) return ret;
    else if (fctx.HasResult()) return fctx.pushJSResult();
    else return JS_UNDEFINED;
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
    else if (ctx->GetKind() == ContextKinds::JavaScript)
    {
        auto L = ctx->GetJSState();
        auto& classID = *ctx->GetClassID(class_name);
        if (classID != 0)
            return;

        JSRuntime* rt = JS_GetRuntime(L);
        JS_NewClassID(rt, &classID);

        JSClassDef class_def = {
            class_name.c_str() };
        class_def.finalizer = CHelpers::JSGCFunction;

        JS_NewClass(rt, classID, &class_def);
    }
}

void AddScriptingClassFunction(EContext* ctx, std::string class_name, std::string function_name, ScriptingClassFunctionCallback callback)
{
    if (ctx->GetKind() == ContextKinds::Lua)
    {
        auto L = ctx->GetLuaState();

        std::string func_key = class_name + " " + function_name;
        ctx->AddClassFunctionCalls(func_key, reinterpret_cast<void*>(callback));

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
    else if (ctx->GetKind() == ContextKinds::JavaScript)
    {
        auto L = ctx->GetJSState();

        std::string func_key = class_name + " " + function_name;
        ctx->AddClassFunctionCalls(func_key, reinterpret_cast<void*>(callback));

        if (function_name == class_name)
        {
            auto ns = JS_GetGlobalObject(L);
            std::vector<JSValue> vals = { Stack<std::string>::pushJS(ctx, func_key) };
            JS_SetPropertyStr(L, ns, class_name.c_str(), JS_NewCFunctionData(L, JSClassCallback, 0, 1, 1, vals.data()));
            JS_FreeValue(L, ns);
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
    else if (context->GetKind() == ContextKinds::JavaScript)
    {
        auto L = context->GetJSState();
        auto data = new ClassData(classdata, class_name, context);
        auto ret = Stack<ClassData*>::pushJS(context, data);
        if (JS_IsException(ret)) {
            delete data;
            return EValue(context);
        }

        MarkDeleteOnGC(data);
        EValue v(context, ret);
        JS_FreeValue(L, ret);
        return v;
    }
    else
        return EValue(context);
}