#include "catch_amalgamated.hpp"
#include "../src/Embedder.h"

int returnValue(std::vector<int> list, EContext* ctx) {
    for(int i = 0; i < list.size(); i++) {
        fprintf(stdout, "%d: %d\n", i, list[i]);
    }
    fprintf(stdout, "Context: %p\n", ctx);
    return 69;
}

TEST_CASE("Lua Namespaces", "[namespace]")
{
    fprintf(stdout, "------------- Lua Namespace -------------\n");
    EContext* ctx = new EContext(ContextKinds::Lua);

    int var = 6245;

    GetGlobalNamespace(ctx)
        .addConstant("GLOBAL_CONST", 69)
        .beginNamespace("Team")
            .addConstant("None", 0)
            .addConstant("Spectator", 1)
            .addConstant("T", 2)
            .addConstant("CT", 3)
            .addProperty("DynamicVar", &var)
            .addFunction("SomeFunction", returnValue)
        .endNamespace();

    const char* s = "print(\"HELLO\"); print(GLOBAL_CONST); print(Team.Spectator); print(Team.DynamicVar); Team.DynamicVar = 69420; print(Team.DynamicVar); print(Team.SomeFunction({4,5,6246,357,435,234,246,4235,2346,634,634,5}))";
    REQUIRE(ctx->RunCode(s) == 0);

    REQUIRE(var == 69420);

    delete ctx;
}

TEST_CASE("JavaScript Namespaces", "[namespace]")
{
    fprintf(stdout, "------------- JavaScript Namespace -------------\n");
    EContext* ctx = new EContext(ContextKinds::JavaScript);

    int var = 6245;

    GetGlobalNamespace(ctx)
        .addConstant("GLOBAL_CONST", 69)
        .beginNamespace("Team")
            .addConstant("None", 0)
            .addConstant("Spectator", 1)
            .addConstant("T", 2)
            .addConstant("CT", 3)
            .addProperty("DynamicVar", &var)
            .addFunction("SomeFunction", returnValue)
        .endNamespace();

    const char* s = "console.log(\"HELLO\"); console.log(GLOBAL_CONST); console.log(Team.Spectator); console.log(Team.DynamicVar); Team.DynamicVar = 69420; console.log(Team.DynamicVar); console.log(Team.SomeFunction([4,5,6246,357,435,234,246,4235,2346,634,634,5]))";
    REQUIRE(ctx->RunCode(s) == 0);

    REQUIRE(var == 69420);
    
    delete ctx;
}