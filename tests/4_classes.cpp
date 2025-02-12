#include "catch_amalgamated.hpp"
#include "../src/Embedder.h"

class SomeClass
{
public:
    SomeClass(int a) { m_b = a; fprintf(stdout, "Constructor called: %d\n", a); }

    void printA(EValue val) { fprintf(stdout, "%d\n", val.isInstance<SomeClass*>()); }

    int m_b;
private:
    int m_a;
};

TEST_CASE("Lua Class", "[classes]")
{
    fprintf(stdout, "------------- Lua Class -------------\n");

    EContext* ctx = new EContext(ContextKinds::Lua);

    BeginClass<SomeClass>("SomeClass", ctx)
        .addConstructor<int>()
        .addFunction("printIt", &SomeClass::printA)
        .addProperty("b", &SomeClass::m_b)
    .endClass();

    const char* s = "local cls = SomeClass(69); local cls2 = SomeClass(420); cls2:printIt(cls2); cls2.b = 69420; cls2:printIt(cls2); cls:printIt(cls2); print(cls.b)";

    REQUIRE(ctx->RunCode(s) == 0);

    delete ctx;
}

TEST_CASE("JavaScript Class", "[classes]")
{
    fprintf(stdout, "------------- JavaScript Class -------------\n");

    EContext* ctx = new EContext(ContextKinds::JavaScript);

    BeginClass<SomeClass>("SomeClass", ctx)
        .addConstructor<int>()
        .addFunction("printIt", &SomeClass::printA)
        .addProperty("b", &SomeClass::m_b)
    .endClass();

    const char* s = "let cls = SomeClass(69); let cls2 = SomeClass(420); cls2.printIt(cls); cls2.b = 69420; cls2.printIt(cls); cls.printIt(cls); console.log(cls.b)";
    REQUIRE(ctx->RunCode(s) == 0);

    delete ctx;
}