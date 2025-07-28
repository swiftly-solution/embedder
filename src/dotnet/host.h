#ifndef _embedder_src_dotnet_host_h
#define _embedder_src_dotnet_host_h

#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
#include <string>

#include "../Context.h"

bool InitializeHostFXR(std::string origin_path);
bool InitializeDotNetAPI();
void CloseHostFXR();

int LoadDotnetFile(EContext* ctx, std::string filePath);
void RemoveDotnetFile(EContext* ctx);
void InterpretAsString(void* obj, int type, const char* out, int len);
void* DotnetAllocateContextPointer(int size, int count);
uint64_t GetDotnetRuntimeMemoryUsage(void* context);
void DotnetExecuteFunction(void* ctx, void* pctx);
void DotnetUpdateGlobalStateCleanupLock(bool state);

#endif