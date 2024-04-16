#ifndef	_MM_H
#define	_MM_H

// 4096 bytes 4KB
#define PAGE_SIZE (1 << 12)

// Memory Layout
// Please reference to linker script

#define HEAP_START             0x1000
#define HEAP_MAX               (char*)0x40000
#define LOW_MEMORY             0x80000
#define KERNEL_START    (char*)0x80000

#define USER_PROCESS_ADDR   (char*)0x800000
#define USER_PROCESS_SP     (char*)0x900000


#ifndef __ASSEMBLER__
void  memzero(unsigned long src, unsigned long n);
void* simple_malloc(unsigned int bytes);



int   mframe_init();
int   mframe_reserve(char* begin, char *end);
char* mframe_alloc(unsigned int bytes);
int   mframe_free(char *ptr);
void  mframe_dump();

#endif

#endif  /*_MM_H */

