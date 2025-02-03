#include "catch_amalgamated.hpp"
#include "../src/Embedder.h"

TEST_CASE("Generate Lua Contexts", "[context]") {
    EContext* ctx = new EContext(ContextKinds::Lua);

    fprintf(stdout, "------------- Lua Context -------------\n");
    fprintf(stdout, "Memory usage: %llu bytes, %.2f kilobytes, %.2f megabytes\n", ctx->GetMemoryUsage(), ctx->GetMemoryUsage() / 1024.0f, ctx->GetMemoryUsage() / 1024.0f / 1024.0f);
    fprintf(stdout, "Context Kind: %d\n", ctx->GetKind());
    fprintf(stdout, "State: %p\n", ctx->GetState());

    REQUIRE(ctx->GetKind() == ContextKinds::Lua);

    delete ctx;
}

TEST_CASE("Generate JavaScript Contexts", "[context]") {
    EContext* ctx = new EContext(ContextKinds::JavaScript);

    fprintf(stdout, "------------- JavaScript Context -------------\n");
    fprintf(stdout, "Memory usage: %llu bytes, %.2f kilobytes, %.2f megabytes\n", ctx->GetMemoryUsage(), ctx->GetMemoryUsage() / 1024.0f, ctx->GetMemoryUsage() / 1024.0f / 1024.0f);
    fprintf(stdout, "Context Kind: %d\n", ctx->GetKind());
    fprintf(stdout, "State: %p\n", ctx->GetState());

    REQUIRE(ctx->GetKind() == ContextKinds::JavaScript);

    delete ctx;
}