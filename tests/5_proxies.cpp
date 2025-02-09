#include "catch_amalgamated.hpp"
#include "../src/Embedder.h"

class SomeCls
{
public:
    SomeCls() = default;
};

int luaCustomIndex(lua_State* L)
{
    auto key = EValue::fromLuaStack(GetContextByState(L), 2).cast<std::string>();
    fprintf(stdout, "Access Key: %s, %p\n", key.c_str(), EValue::fromLuaStack(GetContextByState(L), 1).cast<SomeCls*>());
    return 0;
}

int newluaCustomIndex(lua_State* L)
{
    auto key = EValue::fromLuaStack(GetContextByState(L), 2).cast<std::string>();
    auto value = EValue::fromLuaStack(GetContextByState(L), 3).cast<int>();
    fprintf(stdout, "Set Key: %s = %d\n", key.c_str(), value);
    return 0;
}

TEST_CASE("Lua Proxy", "[proxies]")
{
    fprintf(stdout, "------------- Lua Proxy -------------\n");

    EContext* ctx = new EContext(ContextKinds::Lua);

    GetGlobalNamespace(ctx)
        .beginClass<SomeCls>("SomeCls")
            .addConstructor<>()
            .addLuaCustomIndex(luaCustomIndex, newluaCustomIndex)
        .endClass();

    const char* s = "local cls = SomeCls(); print(cls.b); cls.b = 69420;";

    REQUIRE(ctx->RunCode(s) == 0);

    delete ctx;
}

JSValue jsIndexProxy(JSContext *ctx, JSValue this_val, int argc, JSValue *argv)
{
    auto key = EValue::fromJSStack(GetContextByState(ctx), argv[1]).cast<std::string>();
    fprintf(stdout, "Access Key: %s, %p\n", key.c_str(), EValue::fromJSStack(GetContextByState(ctx), argv[2]).cast<SomeCls*>());
    return JS_NULL;
}

JSValue jsNewIndexProxy(JSContext *ctx, JSValue this_val, int argc, JSValue *argv)
{
    auto key = EValue::fromJSStack(GetContextByState(ctx), argv[1]).cast<std::string>();
    auto value = EValue::fromJSStack(GetContextByState(ctx), argv[2]).cast<int>();
    fprintf(stdout, "Set Key: %s = %d\n", key.c_str(), value);
    return JS_NULL;
}

TEST_CASE("JavaScript Proxy", "[proxies]")
{
    fprintf(stdout, "------------- JavaScript Proxy -------------\n");

    EContext* ctx = new EContext(ContextKinds::JavaScript);

    GetGlobalNamespace(ctx)
        .beginClass<SomeCls>("SomeCls")
            .addConstructor<>()
            .addJSCustomIndex(jsIndexProxy, jsNewIndexProxy)
        .endClass();

    const char* s = "let cls = SomeCls(); console.log(cls.b); cls.b = 69420;";

    REQUIRE(ctx->RunCode(s) == 0);

    delete ctx;
}