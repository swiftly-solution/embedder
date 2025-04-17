#ifndef _embedder_internal_gc_h
#define _embedder_internal_gc_h

void MarkDeleteOnGC(void* ptr);
bool CheckAndPopDeleteOnGC(void* ptr);
bool ShouldDeleteOnGC(void* ptr);

#endif