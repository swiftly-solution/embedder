#ifndef _embedder_engine_h
#define _embedder_engine_h

#include "Context.h"
#include "Value.h"
#include <string>
#include "engine/functions.h"
#include "engine/classes.h"
#include "Value.h"

//////////////////////////////////////////////////////////////
/////////////////    Scripting Engine Value    //////////////
////////////////////////////////////////////////////////////

#define ENGINE_VALUE(value) \
    EValue(ctx, value)
#define ENGINE_VALUE_CTX(ctx, value) \
    EValue(ctx, value)

//////////////////////////////////////////////////////////////
/////////////////  Scripting Engine Variables  //////////////
////////////////////////////////////////////////////////////

#define ADD_VARIABLE(ns_path, var_name, value) \
    AddScriptingVariable(ctx, ns_path, var_name, ENGINE_VALUE(value))
#define ADD_VARIABLE_CTX(ctx, ns_path, var_name, value) \
    AddScriptingVariable(ctx, ns_path, var_name, ENGINE_VALUE_CTX(value))
#define ADD_VARIABLES(ns_path, ...) \
    AddScriptingVariables(ctx, ns_path, __VA_ARGS__)
#define ADD_VARIABLES_CTX(ctx, ns_path, ...) \
    AddScriptingVariables(ctx, ns_path, __VA_ARGS__)

void AddScriptingVariable(EContext* ctx, std::string namespace_path, std::string variable_name, EValue value);
void AddScriptingVariables(EContext* ctx, std::string namespace_path, std::map<std::string, EValue> values);

//////////////////////////////////////////////////////////////
/////////////////  Scripting Engine Functions  //////////////
////////////////////////////////////////////////////////////

#define ADD_FUNCTION(function_name, callback) \
    AddScriptingFunction(ctx, "_G", function_name, callback)
#define ADD_FUNCTION_CTX(ctx, function_name, callback) \
    AddScriptingFunction(ctx, "_G", function_name, callback)
#define ADD_FUNCTION_NS(ns_path, function_name, callback) \
    AddScriptingFunction(ctx, ns_path, function_name, callback)
#define ADD_FUNCTION_NS_CTX(ctx, ns_path, function_name, callback) \
    AddScriptingFunction(ctx, ns_path, function_name, callback)
#define ADD_FUNCTION_PRE(function_name, callback) \
    AddScriptingFunctionPre(ctx, "_G", function_name, callback)
#define ADD_FUNCTION_CTX_PRE(ctx, function_name, callback) \
    AddScriptingFunctionPre(ctx, "_G", function_name, callback)
#define ADD_FUNCTION_NS_PRE(ns_path, function_name, callback) \
    AddScriptingFunctionPre(ctx, ns_path, function_name, callback)
#define ADD_FUNCTION_NS_CTX_PRE(ctx, ns_path, function_name, callback) \
    AddScriptingFunctionPre(ctx, ns_path, function_name, callback)
#define ADD_FUNCTION_POST(function_name, callback) \
    AddScriptingFunctionPost(ctx, "_G", function_name, callback)
#define ADD_FUNCTION_CTX_POST(ctx, function_name, callback) \
    AddScriptingFunctionPost(ctx, "_G", function_name, callback)
#define ADD_FUNCTION_NS_POST(ns_path, function_name, callback) \
    AddScriptingFunctionPost(ctx, ns_path, function_name, callback)
#define ADD_FUNCTION_NS_CTX_POST(ctx, ns_path, function_name, callback) \
    AddScriptingFunctionPost(ctx, ns_path, function_name, callback)

typedef void (*ScriptingFunctionCallback)(FunctionContext*);

void AddScriptingFunction(EContext* ctx, std::string namespace_path, std::string function_name, ScriptingFunctionCallback callback);
void AddScriptingFunctionPre(EContext* ctx, std::string namespace_path, std::string function_name, ScriptingFunctionCallback callback);
void AddScriptingFunctionPost(EContext* ctx, std::string namespace_path, std::string function_name, ScriptingFunctionCallback callback);

//////////////////////////////////////////////////////////////
/////////////////  Scripting Class Functions   //////////////
////////////////////////////////////////////////////////////

#define ADD_CLASS(class_name) \
    AddScriptingClass(ctx, class_name)
#define ADD_CLASS_CTX(ctx, class_name) \
    AddScriptingClass(ctx, class_name)
#define ADD_CLASS_FUNCTION(class_name, function_name, ...) \
    AddScriptingClassFunction(ctx, class_name, function_name, __VA_ARGS__)
#define ADD_CLASS_FUNCTION_CTX(ctx, class_name, function_name, ...) \
    AddScriptingClassFunction(ctx, class_name, function_name, __VA_ARGS__)
#define ADD_CLASS_FUNCTION_PRE(class_name, function_name, ...) \
    AddScriptingClassFunctionPre(ctx, class_name, function_name, __VA_ARGS__)
#define ADD_CLASS_FUNCTION_PRE_CTX(ctx, class_name, function_name, ...) \
    AddScriptingClassFunctionPre(ctx, class_name, function_name, __VA_ARGS__)
#define ADD_CLASS_FUNCTION_POST(class_name, function_name, ...) \
    AddScriptingClassFunctionPost(ctx, class_name, function_name, __VA_ARGS__)
#define ADD_CLASS_FUNCTION_POST_CTX(ctx, class_name, function_name, ...) \
    AddScriptingClassFunctionPost(ctx, class_name, function_name, __VA_ARGS__)

typedef void (*ScriptingClassFunctionCallback)(FunctionContext*, ClassData*);

void AddScriptingClass(EContext* ctx, std::string class_name);
void AddScriptingClassFunction(EContext* ctx, std::string class_name, std::string function_name, ScriptingClassFunctionCallback callback);
void AddScriptingClassFunctionPre(EContext* ctx, std::string class_name, std::string function_name, ScriptingClassFunctionCallback callback);
void AddScriptingClassFunctionPost(EContext* ctx, std::string class_name, std::string function_name, ScriptingClassFunctionCallback callback);

//////////////////////////////////////////////////////////////
/////////////////   Scripting Class Instance   //////////////
////////////////////////////////////////////////////////////

#define MAKE_CLASS_INSTANCE(class_name, ...) \
    CreateScriptingClassInstance(context, class_name, __VA_ARGS__)
#define MAKE_CLASS_INSTANCE_CTX(ctx, class_name, ...) \
    CreateScriptingClassInstance(ctx, class_name, __VA_ARGS__)

EValue CreateScriptingClassInstance(FunctionContext* context, std::string class_name, std::map<std::string, std::any> classdata);
EValue CreateScriptingClassInstance(EContext* context, std::string class_name, std::map<std::string, std::any> classdata);

//////////////////////////////////////////////////////////////
/////////////////   Scripting Class Members    //////////////
////////////////////////////////////////////////////////////

#define ADD_CLASS_MEMBER(class_name, member_name, callback_get, callback_set) \
    AddScriptingClassMember(ctx, class_name, member_name, callback_get, callback_set)
#define ADD_CLASS_MEMBER_CTX(ctx, class_name, member_name, callback_get, callback_set) \
    AddScriptingClassMember(ctx, class_name, member_name, callback_get, callback_set)
#define ADD_CLASS_MEMBER_PRE(class_name, member_name, callback_get, callback_set) \
    AddScriptingClassMemberPre(ctx, class_name, member_name, callback_get, callback_set)
#define ADD_CLASS_MEMBER_PRE_CTX(ctx, class_name, member_name, callback_get, callback_set) \
    AddScriptingClassMemberPre(ctx, class_name, member_name, callback_get, callback_set)
#define ADD_CLASS_MEMBER_POST(class_name, member_name, callback_get, callback_set) \
    AddScriptingClassMemberPost(ctx, class_name, member_name, callback_get, callback_set)
#define ADD_CLASS_MEMBER_POST_CTX(ctx, class_name, member_name, callback_get, callback_set) \
    AddScriptingClassMemberPost(ctx, class_name, member_name, callback_get, callback_set)
#define ADD_CLASS_MEMBER_READONLY(class_name, member_name, callback_get) \
    AddScriptingClassMember(ctx, class_name, member_name, callback_get, [](FunctionContext *context, ClassData *data) -> void {})
#define ADD_CLASS_MEMBER_READONLY_CTX(ctx, class_name, member_name, callback_get) \
    AddScriptingClassMember(ctx, class_name, member_name, callback_get, [](FunctionContext *context, ClassData *data) -> void {})

void AddScriptingClassMember(EContext* ctx, std::string class_name, std::string member_name, ScriptingClassFunctionCallback callback_get, ScriptingClassFunctionCallback callback_set);
void AddScriptingClassMemberPre(EContext* ctx, std::string class_name, std::string member_name, ScriptingClassFunctionCallback callback_get, ScriptingClassFunctionCallback callback_set);
void AddScriptingClassMemberPost(EContext* ctx, std::string class_name, std::string member_name, ScriptingClassFunctionCallback callback_get, ScriptingClassFunctionCallback callback_set);

#endif