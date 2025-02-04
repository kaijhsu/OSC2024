#ifndef	_MM_H
#define	_MM_H

#define    PAGE_SHIFT   12
#define   TABLE_SHIFT   9
#define SECTION_SHIFT	(PAGE_SHIFT + TABLE_SHIFT)

#define    PAGE_SIZE   	(1 << PAGE_SHIFT)	
#define SECTION_SIZE   	(1 << SECTION_SHIFT)	

#define LOW_MEMORY      (2 * SECTION_SIZE)

#ifndef __ASSEMBLER__

void memzero(unsigned long src, unsigned long n);

extern char _end;
char* __heap_ptr = (char *)&_end;

void* simple_malloc(unsigned int bytes){
    void *ret = __heap_ptr;
    __heap_ptr += bytes;
    return ret;
}

#endif

#endif  /*_MM_H */

