#include "invoker.h"
#include "../Helpers.h"
#include "../CHelpers.h"

class EContext;
class ClassData;

std::map<std::type_index, int> typesMap = {
    { typeid(void*), 1 },
    { typeid(ClassData*), 1 },
    { typeid(bool), 2 },
    { typeid(uint8_t), 3 },
    { typeid(int8_t), 4 },
    { typeid(char), 5 },
    { typeid(short), 6 },
    { typeid(unsigned short), 7 },
    { typeid(int), 8 },
    { typeid(unsigned int), 9 },
    { typeid(int64_t), 10 },
    { typeid(uint64_t), 11 },
    { typeid(float), 12 },
    { typeid(double), 13 },
    { typeid(std::string), 14 },
};

void DotNetFunctionCallback(EContext* ctx, CallContext& call_ctx);
void DotNetMemberCallback(EContext* ctx, CallContext& call_ctx);
void DotnetClassCallback(EContext* ctx, CallContext& call_ctx, bool bypassClassCheck = false);

void Dotnet_InvokeNative(CallData& context)
{
    CallContext ctx(context);
    if (context.call_kind == (int)CallKind::Function) return DotNetFunctionCallback(ctx.GetArgument<EContext*>(0), ctx);
    else if (context.call_kind == (int)CallKind::ClassFunction) return DotnetClassCallback(ctx.GetArgument<EContext*>(0), ctx);
    else if (context.call_kind == (int)CallKind::CoreClassFunction) return DotnetClassCallback(ctx.GetArgument<EContext*>(0), ctx, true);
    else if (context.call_kind == (int)CallKind::ClassMember) return DotNetMemberCallback(ctx.GetArgument<EContext*>(0), ctx);
}

std::set<void*> droppedValues;

void Dotnet_ClassDataFinalizer(void* plugin_context, void* instance)
{
    CHelpers::DotNetGCFunction((EContext*)plugin_context, (ClassData*)instance, &droppedValues);
}