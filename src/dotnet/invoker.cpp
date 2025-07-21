#include "invoker.h"

class EContext;

std::map<std::type_index, int> typesMap = {
    { typeid(void*), 1 },
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

void Dotnet_InvokeNative(CallData& context)
{
    CallContext ctx(context);
    if (context.call_kind == (int)CallKind::Function) return DotNetFunctionCallback(ctx.GetArgument<EContext*>(0), ctx);
}