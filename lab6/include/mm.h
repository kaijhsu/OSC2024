#ifndef	_MM_H
#define	_MM_H

// 4096 bytes 4KB
#define PAGE_SIZE (1 << 12)

// Memory Layout
// Please reference to linker script

#define HEAP_START             0x2000
#define HEAP_MAX               0x40000
#define LOW_MEMORY             0x80000
#define KERNEL_START    (char*)0x80000

#define USER_PROCESS_ADDR   (char*)0x800000
#define USER_PROCESS_SP     (char*)0x900000

#define uint64_t unsigned long 
#define uint32_t unsigned int
#define uint16_t unsigned short
#define uint8_t  unsigned char
#define int64_t  long
#define int32_t  int
#define int16_t  short
#define int8_t   char


#ifndef __ASSEMBLER__

#include "stddef.h"
#include "mmu.h"

void  memzero(unsigned long src, unsigned long n);
void  memcopy(void* src, void *dest, size_t bytes);

void* simple_malloc(unsigned int bytes);
void* malloc(uint32_t bytes);
int   mfree(void* ptr);

int   mframe_init();
int   mframe_reserve(char* begin, char *end);
char* mframe_alloc(unsigned int bytes);
int   mframe_free(char *ptr);
void  mframe_dump();

uint64_t mbegin_get();
uint64_t mend_get();

#endif

#endif  /*_MM_H */

