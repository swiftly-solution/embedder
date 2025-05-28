#include "classes.h"
#include "../Context.h"
#include "../Engine.h"
#include <regex>

ClassData::ClassData(std::map<std::string, std::any> data, std::string className, EContext* ctx)
{
    m_classData = data;
    m_className = className;
    m_ctx = ctx;
}

ClassData::~ClassData()
{
    std::vector<ClassData**> udatas = GetDataOr<std::vector<ClassData**>>("lua_udatas", std::vector<ClassData**>{});
    for (int i = 0; i < udatas.size(); i++) {
        ClassData** udata = udatas[i];
        (*udata) = nullptr;
    }

    std::vector<JSRuntime*> rts = GetDataOr<std::vector<JSRuntime*>>("js_runtimes", std::vector<JSRuntime*>{});
    for (int i = 0; i < rts.size(); i++) {
        if (JS_GetRuntimeOpaque(rts[i]) == nullptr) {
            auto dt = new std::set<ClassData*>{};
            JS_SetRuntimeOpaque(rts[i], dt);
        }

        auto val = (std::set<ClassData*>*)JS_GetRuntimeOpaque(rts[i]);
        if (!val) continue;

        val->insert(this);
    }

    if (!m_ctx) return;
    std::string str_key = m_className + " ~" + m_className;
    void* func = m_ctx->GetClassFunctionCall(str_key);
    if (!func) return;

    ScriptingClassFunctionCallback cb = reinterpret_cast<ScriptingClassFunctionCallback>(func);
    FunctionContext fctx(str_key, m_ctx->GetKind(), m_ctx, {}, 0);
    FunctionContext* fptr = &fctx;
    ClassData* data = this;
    bool ignoreCustomReturn = false;

    auto functionPreCalls = m_ctx->GetClassFunctionPreCalls(str_key);
    auto functionPostCalls = m_ctx->GetClassFunctionPostCalls(str_key);
    bool stopExecution = false;
    JSValue ret = JS_UNDEFINED;

    for (void* func : functionPreCalls)
    {
        reinterpret_cast<ScriptingClassFunctionCallback>(func)(fptr, data);
        if (fctx.ShouldStopExecution())
        {
            stopExecution = true;
            break;
        }
    }

    if (!stopExecution) {
        cb(fptr, data);

        for (void* func : functionPostCalls)
        {
            reinterpret_cast<ScriptingClassFunctionCallback>(func)(fptr, data);
            if (fctx.ShouldStopExecution()) break;
        }
    }
}

void ClassData::SetData(std::string key, std::any value)
{
    m_classData[key] = value;
}

std::string ClassData::GetClassname()
{
    return m_className;
}

bool ClassData::HasData(std::string key)
{
    return m_classData.find(key) != m_classData.end();
}