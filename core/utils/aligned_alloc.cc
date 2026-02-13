#include "aligned_alloc.h"
#include <cstdlib>

void* aligned_malloc(std::size_t size, std::size_t alignment){
    void* ptr = nullptr;
    if(posix_memalign(&ptr, alignment, size)!=0){
        return nullptr;
    }
    return ptr;
}

void aligned_free(void* ptr){
    free(ptr);
}