#include "host.h"
#include "dynlib.h"
#include "strconv.h"
#include "invoker.h"

#include <string.h>
#include <iostream>

hostfxr_initialize_for_runtime_config_fn _initialize_for_runtime_config = nullptr;
hostfxr_get_runtime_delegate_fn _get_runtime_delegate = nullptr;
hostfxr_close_fn _close = nullptr;
load_assembly_and_get_function_pointer_fn _load_assembly_and_get_function_pointer = nullptr;
hostfxr_set_runtime_property_value_fn _set_runtime_prop_value = nullptr;
hostfxr_handle fxrcxt;

void* GetDotnetPointer(int kind);

typedef int(CORECLR_DELEGATE_CALLTYPE* load_file_fn)(void* context, const char* filePath, int len);
typedef void(CORECLR_DELEGATE_CALLTYPE* remove_file_fn)(void* context);
typedef void(CORECLR_DELEGATE_CALLTYPE* interpret_as_string_fn)(void* object, int type, const char* out, int len);
typedef void* (CORECLR_DELEGATE_CALLTYPE* allocate_pointer_fn)(int size, int count);
typedef uint64_t(CORECLR_DELEGATE_CALLTYPE* get_plugin_memory_fn)(void* context);
typedef void(CORECLR_DELEGATE_CALLTYPE* execute_function_fn)(void* ctx, void* pctx);
typedef void(CORECLR_DELEGATE_CALLTYPE* state_fn)(int state);

load_file_fn loadFile = nullptr;
interpret_as_string_fn interpretAsString = nullptr;
remove_file_fn removeFile = nullptr;
allocate_pointer_fn allocatePointer = nullptr;
get_plugin_memory_fn getMemory = nullptr;
execute_function_fn execFunction = nullptr;
state_fn set_state = nullptr;

void* hostfxr_lib = nullptr;

#ifdef _WIN32
char_t dotnet_path[1024];
#else
char dotnet_path[1024];
#endif

#ifdef _WIN32
std::wstring widenedOriginPath;
#else
std::string widenedOriginPath;
#endif

bool InitializeHostFXR(std::string origin_path) {
#ifdef _WIN32
    widenedOriginPath = StringWide(origin_path);
#else
    widenedOriginPath = origin_path;
#endif

    hostfxr_lib = load_library(WIN_LIN(L"hostfxr.dll", "libhostfxr.so"));
    if (!hostfxr_lib) return false;

    _initialize_for_runtime_config = (hostfxr_initialize_for_runtime_config_fn)get_export(hostfxr_lib, "hostfxr_initialize_for_runtime_config");
    if (!_initialize_for_runtime_config) return false;

    _get_runtime_delegate = (hostfxr_get_runtime_delegate_fn)get_export(hostfxr_lib, "hostfxr_get_runtime_delegate");
    if (!_get_runtime_delegate) return false;

    _close = (hostfxr_close_fn)get_export(hostfxr_lib, "hostfxr_close");
    if (!_close) return false;

    _set_runtime_prop_value = (hostfxr_set_runtime_property_value_fn)get_export(hostfxr_lib, "hostfxr_set_runtime_property_value");
    if (!_set_runtime_prop_value) return false;

    hostfxr_initialize_parameters params;
    params.size = sizeof(hostfxr_initialize_parameters);
#ifdef _WIN32
    std::wstring path = widenedOriginPath + L"addons\\swiftly\\bin\\managed\\dotnet";
#else
    std::string path = widenedOriginPath + "addons/swiftly/bin/managed/dotnet";
#endif

    memcpy(dotnet_path, path.c_str(), path.size() >= 1024 ? 1023 : path.size());

    params.dotnet_root = dotnet_path;

    int returnCode = _initialize_for_runtime_config((widenedOriginPath + WIN_LIN(L"addons\\swiftly\\bin\\managed\\SwiftlyS2.runtime.json", "addons/swiftly/bin/managed/SwiftlyS2.runtimeconfig.json")).c_str(), &params, &fxrcxt);
    if (returnCode != 0) {
        _close(fxrcxt);
        return false;
    }

    _set_runtime_prop_value(fxrcxt, WIN_LIN(L"APP_CONTEXT_BASE_DIRECTORY", "APP_CONTEXT_BASE_DIRECTORY"), WIN_LIN(path.c_str(), dotnet_path));

    returnCode = _get_runtime_delegate(fxrcxt, hdt_load_assembly_and_get_function_pointer, (void**)&_load_assembly_and_get_function_pointer);
    if (returnCode != 0 || (void*)_load_assembly_and_get_function_pointer == nullptr) {
        _close(fxrcxt);
        return false;
    }

    return true;
}

bool InitializeDotNetAPI() {
    typedef void(CORECLR_DELEGATE_CALLTYPE* custom_loader_fn)(void* invokeNative, void* finalizer);
    static custom_loader_fn custom_loader = nullptr;

    if (custom_loader == nullptr) {
        int returnCode = _load_assembly_and_get_function_pointer(
            (widenedOriginPath + WIN_LIN(L"addons\\swiftly\\bin\\managed\\SwiftlyS2.dll", "addons/swiftly/bin/managed/SwiftlyS2.dll")).c_str(),
            STR("SwiftlyS2.Entrypoint, SwiftlyS2"), STR("Start"), UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&custom_loader
        );

        if (returnCode != 0 || (void*)custom_loader == nullptr) {
            return false;
        }

        custom_loader(reinterpret_cast<void*>(Dotnet_InvokeNative), reinterpret_cast<void*>(Dotnet_ClassDataFinalizer));
    }

    return true;
}

void CloseHostFXR() {
    if (fxrcxt && _close) _close(fxrcxt);
    unload_library(hostfxr_lib);
}

int LoadDotnetFile(EContext* ctx, std::string filePath)
{
    if (loadFile == nullptr) {
        if (_load_assembly_and_get_function_pointer) {
            int returnCode = _load_assembly_and_get_function_pointer(
                (widenedOriginPath + WIN_LIN(L"addons\\swiftly\\bin\\managed\\SwiftlyS2.dll", "addons/swiftly/bin/managed/SwiftlyS2.dll")).c_str(),
                STR("SwiftlyS2.Entrypoint, SwiftlyS2"), STR("LoadFile"), UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&loadFile
            );

            if (returnCode != 0 || loadFile == nullptr) return 1;
        }
        else {
            loadFile = (load_file_fn)GetDotnetPointer(1);
        }
    }

    return loadFile(ctx, filePath.c_str(), (int)filePath.size());
}

void InterpretAsString(void* obj, int type, const char* out, int len)
{
    if (interpretAsString == nullptr) {
        if (_load_assembly_and_get_function_pointer) {
            int returnCode = _load_assembly_and_get_function_pointer(
                (widenedOriginPath + WIN_LIN(L"addons\\swiftly\\bin\\managed\\SwiftlyS2.dll", "addons/swiftly/bin/managed/SwiftlyS2.dll")).c_str(),
                STR("SwiftlyS2.Entrypoint, SwiftlyS2"), STR("InterpretAsString"), UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&interpretAsString
            );

            if (returnCode != 0 || interpretAsString == nullptr) return;
        }
        else {
            interpretAsString = (interpret_as_string_fn)GetDotnetPointer(2);
        }
    }

    interpretAsString(obj, type, out, len);
}

void RemoveDotnetFile(EContext* ctx)
{
    if (removeFile == nullptr) {
        if (_load_assembly_and_get_function_pointer) {
            int returnCode = _load_assembly_and_get_function_pointer(
                (widenedOriginPath + WIN_LIN(L"addons\\swiftly\\bin\\managed\\SwiftlyS2.dll", "addons/swiftly/bin/managed/SwiftlyS2.dll")).c_str(),
                STR("SwiftlyS2.Entrypoint, SwiftlyS2"), STR("RemoveFile"), UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&removeFile
            );

            if (returnCode != 0 || removeFile == nullptr) return;
        }
        else {
            removeFile = (remove_file_fn)GetDotnetPointer(3);
        }
    }

    removeFile(ctx);
}

void* DotnetAllocateContextPointer(int size, int count)
{
    if (allocatePointer == nullptr) {
        if (_load_assembly_and_get_function_pointer) {
            int returnCode = _load_assembly_and_get_function_pointer(
                (widenedOriginPath + WIN_LIN(L"addons\\swiftly\\bin\\managed\\SwiftlyS2.dll", "addons/swiftly/bin/managed/SwiftlyS2.dll")).c_str(),
                STR("SwiftlyS2.Entrypoint, SwiftlyS2"), STR("AllocateContextPointer"), UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&allocatePointer
            );

            if (returnCode != 0 || allocatePointer == nullptr) return nullptr;
        }
        else {
            allocatePointer = (allocate_pointer_fn)GetDotnetPointer(4);
        }
    }

    return allocatePointer(size, count);
}

uint64_t GetDotnetRuntimeMemoryUsage(void* context)
{
    if (getMemory == nullptr) {
        if (_load_assembly_and_get_function_pointer) {
            int returnCode = _load_assembly_and_get_function_pointer(
                (widenedOriginPath + WIN_LIN(L"addons\\swiftly\\bin\\managed\\SwiftlyS2.dll", "addons/swiftly/bin/managed/SwiftlyS2.dll")).c_str(),
                STR("SwiftlyS2.Entrypoint, SwiftlyS2"), STR("GetPluginMemoryUsage"), UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&getMemory
            );

            if (returnCode != 0 || getMemory == nullptr) return 0;
        }
        else {
            getMemory = (get_plugin_memory_fn)GetDotnetPointer(5);
        }
    }

    return getMemory(context);
}

void DotnetExecuteFunction(void* ctx, void* pctx)
{
    if (execFunction == nullptr) {
        if (_load_assembly_and_get_function_pointer) {
            int returnCode = _load_assembly_and_get_function_pointer(
                (widenedOriginPath + WIN_LIN(L"addons\\swiftly\\bin\\managed\\SwiftlyS2.dll", "addons/swiftly/bin/managed/SwiftlyS2.dll")).c_str(),
                STR("SwiftlyS2.Entrypoint, SwiftlyS2"), STR("ExecuteFunction"), UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&execFunction
            );

            if (returnCode != 0 || execFunction == nullptr) return;
        }
        else {
            execFunction = (execute_function_fn)GetDotnetPointer(6);
        }
    }

    return execFunction(ctx, pctx);
}

void DotnetUpdateGlobalStateCleanupLock(bool state)
{
    if (set_state == nullptr) {
        if (_load_assembly_and_get_function_pointer) {
            int returnCode = _load_assembly_and_get_function_pointer(
                (widenedOriginPath + WIN_LIN(L"addons\\swiftly\\bin\\managed\\SwiftlyS2.dll", "addons/swiftly/bin/managed/SwiftlyS2.dll")).c_str(),
                STR("SwiftlyS2.Entrypoint, SwiftlyS2"), STR("UpdateGlobalStateCleanupLock"), UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&set_state
            );

            if (returnCode != 0 || (void*)set_state == nullptr) return;
        }
        else {
            set_state = (state_fn)GetDotnetPointer(7);
        }
    }

    set_state((int)state);
}