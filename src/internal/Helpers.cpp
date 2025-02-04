#include "Helpers.h"

uint32_t JSGetArrayLength(JSContext* ctx, JSValue val)
{
    uint32_t len;
    auto prop = JS_GetPropertyStr(ctx, val, "length");
    JS_ToUint32(ctx, &len, prop);
    JS_FreeValue(ctx, prop);
    return len;
}