#include "dynlib.h"

#ifdef _WIN32
void* load_library(const char_t* path)
{
    HMODULE h = ::LoadLibraryW(path);
    return (void*)h;
}

void* get_export(void* h, const char* name)
{
    void* f = ::GetProcAddress((HMODULE)h, name);
    return f;
}

void unload_library(void* lib)
{
    if (!lib) return;
    ::FreeLibrary((HMODULE)lib);
}
#else
void* load_library(const char_t* path)
{
    void* h = dlopen(path, RTLD_LAZY | RTLD_LOCAL);
    return h;
}
void* get_export(void* h, const char* name)
{
    void* f = dlsym(h, name);
    return f;
}
void unload_library(void* lib)
{
    if (!lib) return;
    dlclose(lib);
}
#endif