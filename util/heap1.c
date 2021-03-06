
#include <main.h>
#include "heap1.h"

#if 0
#pragma message "BYTE_ALIGN:" STR(BYTE_ALIGN)
#pragma message "BYTE_ALIGN_MASK:" #BYTE_ALIGN_MASK
#pragma message "HEAPSZ:" STR(HEAPSZ)
#endif

#ifdef __arm__
extern unsigned char _ebss[];
extern unsigned char _estack[];
#else
unsigned char myheap1[HEAPSZ];
#define _ebss &myheap1[0]
#define _estack &myheap1[HEAPSZ]
#endif
static size_t aligned_heap = (size_t)(_ebss);

#ifndef MAXSTR
#define MAXSTR 1024
#pragma message "MAXSTR:" STR(MAXSTR)
#endif

void* mymalloc(size_t sz)
{
    void *ptr = 0;
    if(aligned_heap & BYTE_ALIGN_MASK)
        aligned_heap = (aligned_heap | BYTE_ALIGN_MASK) + 1;
    if((aligned_heap + sz) < (size_t)(_estack))
    {
        ptr = (void*)aligned_heap;
        aligned_heap += sz;
    }
    if(aligned_heap & BYTE_ALIGN_MASK)
        aligned_heap = (aligned_heap | BYTE_ALIGN_MASK) + 1;
    return ptr;
}

char* mystrdup(char *str)
{
    size_t len = mystrnlen(str, MAXSTR);
    char *ptr = mymalloc(len + 1);
    if(ptr)
    {
        mystrncpy(ptr, str, MAXSTR);
        return ptr;
    }
    return 0;
}

void* mycalloc(size_t nmemb, size_t sz)
{
    sz *= nmemb;
    uint8_t *ptr = (uint8_t*)mymalloc(sz + 1);
    if(!ptr)
        return 0;
    while(sz--)
        ptr[sz] = 0;
    return (void*)ptr;
}

void myfree(void *ptr)
{
    aligned_heap = (size_t)(ptr);
}

size_t mymemory(void)
{
    return (size_t)(_estack) - aligned_heap;
}

