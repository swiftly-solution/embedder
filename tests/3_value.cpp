#include "catch_amalgamated.hpp"
#include "../src/Embedder.h"

EValue returnValue(EValue val1, EValue val2, EValue callback) {
    callback(val1, val2);

    fprintf(stdout, "%d, %d, %d\n", val1.isFunction(), val2.isBool(), val1.isNumber());

    int firstval = val1.cast<int>();
    int secondval = val2.cast<int>();

    fprintf(stdout, "%d, %d, %d\n", firstval - secondval, firstval, secondval);

    EValue val(val1.getContext(), 69);
    return val;
}

TEST_CASE("Lua Value", "[value]")
{
    fprintf(stdout, "------------- Lua Value -------------\n");

    EContext* ctx = new EContext(ContextKinds::Lua);

    GetGlobalNamespace(ctx).addFunction("retval", returnValue);

    const char* s = "print(retval(3, 4, function(a, b) print(a + b, a, b) end))";
    REQUIRE(ctx->RunCode(s) == 0);
    REQUIRE(EValue::getGlobal(ctx, "retval").isFunction() == true);

    delete ctx;
}

TEST_CASE("JavaScript Value", "[value]")
{
    fprintf(stdout, "------------- JavaScript Value -------------\n");

    EContext* ctx = new EContext(ContextKinds::JavaScript);

    GetGlobalNamespace(ctx).addFunction("retval", returnValue);

    const char* s = "console.log(retval(3, 4, (a, b) => { console.log(a + b, a, b) }))";
    REQUIRE(ctx->RunCode(s) == 0);

    REQUIRE(EValue::getGlobal(ctx, "retval").isFunction() == true);

    delete ctx;
}