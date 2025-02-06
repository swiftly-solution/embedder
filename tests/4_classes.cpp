#include "catch_amalgamated.hpp"
#include "../src/Embedder.h"

class SomeClass
{
public:
    SomeClass(int a) { m_b = a; fprintf(stdout, "Constructor called: %d\n", a); }

    void printA() { fprintf(stdout, "%d\n", m_b); }

    int m_b;
private:
    int m_a;
};

TEST_CASE("Lua Class", "[classes]")
{
    fprintf(stdout, "------------- Lua Class -------------\n");

    EContext* ctx = new EContext(ContextKinds::Lua);

    GetGlobalNamespace(ctx)
        .beginClass<SomeClass>("SomeClass")
            .addConstructor<int>()
            .addFunction("printIt", &SomeClass::printA)
            .addProperty("b", &SomeClass::m_b)
        .endClass();

    const char* s = "local cls = SomeClass(69); local cls2 = SomeClass(420); cls2:printIt(); cls2.b = 69420; cls2:printIt(); cls:printIt(); print(cls.b)";

    REQUIRE(ctx->RunCode(s) == 0);

    delete ctx;
}