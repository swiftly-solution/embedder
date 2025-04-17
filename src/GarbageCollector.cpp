#include "GarbageCollector.h"

#include <set>

std::set<void*> deleteOnGC;

void MarkDeleteOnGC(void* ptr)
{
    deleteOnGC.insert(ptr);
}

bool CheckAndPopDeleteOnGC(void* ptr)
{
    if(deleteOnGC.find(ptr) == deleteOnGC.end()) return false;

    deleteOnGC.erase(ptr);
    return true;
}

bool ShouldDeleteOnGC(void* ptr)
{
    return deleteOnGC.find(ptr) != deleteOnGC.end();
}