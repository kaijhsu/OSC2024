#include "mm.h"
#include "mini_uart.h"
#include "peripherals/devicetree.h"
#include "m_string.h"
#include "initramfs.h"

extern char _heap_begin;
char* __heap_ptr = (char* )&_heap_begin;

void* simple_malloc(unsigned int bytes){
    if(__heap_ptr + bytes >= HEAP_MAX){
        debug("heap buffer overflow");
        return (void *)0;
    }
    void *ret = __heap_ptr;
    __heap_ptr += bytes;
    return ret;
}

void* malloc(uint32_t bytes){
    // TODO: implement dynamic mfree
    return simple_malloc(bytes);
}

int mfree(void* ptr){
    // TODO: implement mfree
    return 0;
}

void memcopy(void* src, void *dest, size_t bytes){
   uint8_t *s = (uint8_t*) src;
   uint8_t *d = (uint8_t*) dest;
   while(bytes--) *d++ = *s++;
   return ;
}

extern char _mframe_begin;
unsigned char* const mframe = (unsigned char*)&_mframe_begin;
char* mbegin = (char*)0;
char* mend   = (char*)0x40000000;
// mframe[i][7:6] status bit
// mframe[i][5:0] At least usable size in shift bit 
#define MFRAME_FREE     0
#define MFRAME_DIVIDE   1
#define MFRAME_USED     2
#define MFRAME_RESERVE  3

unsigned char* get_parent(unsigned char* mptr){
    return (mptr == mframe)? (unsigned char*)0: mframe+(mptr-mframe-1)/2;
}

unsigned char* get_left(unsigned char* mptr){
    return mframe+2*(mptr-mframe)+1;
}

unsigned char* get_right(unsigned char* mptr){
    return mframe+2*(mptr-mframe)+2;
}

unsigned char get_shift(unsigned char* mptr){
    return 0b111111 & *mptr;
}

unsigned char get_status(unsigned char* mptr){
    return ((*mptr) >> 6) & 0b11;
}

char* get_mid(char* begin, char* end){
    return (char*)(((unsigned long)begin + (unsigned long)end) / 2);
}

void set_mframe(unsigned char* mptr, unsigned char status, unsigned char shift){
    *mptr = ((status & 0b11) << 6) | (shift & 0b111111);  
}

void update_mframe(unsigned char* mptr){
    unsigned char* left = get_left(mptr);
    unsigned char* right = get_right(mptr);
    unsigned char left_status = get_status(left);
    unsigned char right_status = get_status(right);
    unsigned char left_shift = get_shift(left);
    unsigned char right_shift = get_shift(right);

    if(left_status == MFRAME_FREE && right_status == MFRAME_FREE)
        set_mframe(mptr, MFRAME_FREE, left_shift + 1);
    else{
        unsigned char new_shift = 0;
        if(left_status == MFRAME_FREE || left_status == MFRAME_DIVIDE)
            new_shift = left_shift;
        if(right_status == MFRAME_FREE || right_status == MFRAME_DIVIDE)
            new_shift = (right_shift > new_shift)? right_shift: new_shift;
        set_mframe(mptr, MFRAME_DIVIDE, new_shift);
    }
}


char* mframe_alloc_iter(unsigned int bytes, unsigned char* mptr, char* mbegin, char* mend){
    unsigned char status = get_status(mptr);
    unsigned char shift = get_shift(mptr);
    unsigned long size = 1 << shift;
    char* ret_ptr = (char*) 0;

    if(status == MFRAME_FREE){
        if(bytes > size){
            debug("not enough space");
            return ret_ptr;
        }
        else if(bytes > size/2 || size <= PAGE_SIZE){
            set_mframe(mptr, MFRAME_USED, get_shift(mptr));
            ret_ptr = mbegin;
        }
        else{
            unsigned char* left = get_left(mptr);
            unsigned char* right = get_right(mptr);
            set_mframe(mptr, MFRAME_DIVIDE, shift-1);
            set_mframe(left, MFRAME_FREE, shift-1);
            set_mframe(right, MFRAME_FREE, shift-1);
            ret_ptr =  mframe_alloc_iter(bytes, left, mbegin, get_mid(mbegin, mend));
            update_mframe(mptr);
        }
    }
    else if(status == MFRAME_DIVIDE){ 
        if(bytes > size){
            debug();
            return (char*) 0;
        }
        
        unsigned char* left = get_left(mptr);
        unsigned char* right = get_right(mptr);
        unsigned char left_status = get_status(left);
        unsigned char right_status = get_status(right);
        unsigned long left_size = 1 << get_shift(left);
        unsigned long right_size = 1 << get_shift(right);

        if(left_status == MFRAME_USED || left_status == MFRAME_RESERVE)
            left_size = 0;
        if(right_status == MFRAME_USED || right_status == MFRAME_RESERVE)
            right_size = 0;
        // debug("left size: %d, right size: %d bytes: %d", left_size ,right_size, bytes);

        char* mid = get_mid(mbegin, mend);

        if(left_size > bytes && right_size > bytes){
            if(left_size <= right_size)
                ret_ptr = mframe_alloc_iter(bytes, left, mbegin, mid);
            else
                ret_ptr = mframe_alloc_iter(bytes, right, mid, mend);
        }
        else{
            if(bytes < left_size)
                ret_ptr = mframe_alloc_iter(bytes, left, mbegin, mid);
            else if (bytes < right_size)
                ret_ptr = mframe_alloc_iter(bytes, right, mid, mend);
            else 
                debug("into wrong mframe");
        }
        update_mframe(mptr);
    }
    if(ret_ptr == 0){
        debug("Already in used mbegin: 0x%x mend: 0x%x", mbegin, mend);
    }
    return ret_ptr;
}

char* mframe_alloc(unsigned int bytes){
    char* ret = mframe_alloc_iter(bytes, mframe, mbegin, mend);
    // kprintf("return %x -> %x\n", ret, ret+bytes);
    // mframe_dump(-1);
    return ret;
}

int mframe_free_iter(char* addr, unsigned char* mptr, char* mbegin, char* mend){
    unsigned char status = get_status(mptr);
    char* mid = get_mid(mbegin, mend);
    if(status == MFRAME_FREE){
        debug("this addr 0x%x hasn't been used", addr);
        return -1;
    }
    else if(status == MFRAME_DIVIDE){
        unsigned char* left = get_left(mptr);
        unsigned char* right = get_right(mptr);
        if(addr < mid)
            mframe_free_iter(addr, left, mbegin, mid);
        else
            mframe_free_iter(addr, right, mid, mend);
        
        update_mframe(mptr);
        return 0;
    }
    else if(status == MFRAME_USED){
        set_mframe(mptr, MFRAME_FREE, get_shift(mptr));
        return 0;
    }
    else if(status == MFRAME_RESERVE){
        debug("this addr 0x%x is reserved", addr);
        return -1;
    }
    debug("never go here");
    return -1;
}

int mframe_free(char* addr){
    return mframe_free_iter(addr, mframe, mbegin, mend);
}

int mframe_reserve_iter(char* begin, char* end, unsigned char* mptr, char* mbegin, char* mend){
    unsigned long reserve_size = end - begin;
    unsigned char shift = get_shift(mptr);
    unsigned long size = (1 << shift);


    unsigned char status = get_status(mptr);
    if(status == MFRAME_FREE){
        set_mframe(get_left(mptr), MFRAME_FREE, shift - 1);
        set_mframe(get_right(mptr), MFRAME_FREE, shift - 1);
    }
    else if(status == MFRAME_DIVIDE){
        if(reserve_size > size)
            debug("no enough space");    
    }
    else if(status == MFRAME_USED){
        debug("this node has been used");
        return 0;
    }
    else if(status == MFRAME_RESERVE){
        debug("already reserve");
        return 0;
    }

    if(reserve_size > (size >> 1) || PAGE_SIZE >= size){
        // reserve interval claim this mframe 
        debug("reserve 0x%x 0x%x", mbegin, mend);
        set_mframe(mptr, MFRAME_RESERVE, get_shift(mptr));
        return 0;
    }

    char* mid = get_mid(mbegin, mend);
    unsigned char* left  = get_left(mptr);
    unsigned char* right = get_right(mptr);
    if(end <= mid)          // on left side
        mframe_reserve_iter(begin, end, left, mbegin, mid);
    else if(begin >= mid)   // on right side
        mframe_reserve_iter(begin, end, right, mid, mend);
    else{                   // across middle
        mframe_reserve_iter(begin, mid, left, mbegin, mid);
        mframe_reserve_iter(mid, end, right, mid, mend);
    } 

    update_mframe(mptr);
    return 0;
}

int mframe_reserve(char* begin, char* end){
    if(begin > end){
        debug("begin > end");
        return -1;
    }
    end = (end > mend)? mend: end;
    begin = (begin < mbegin)? mbegin: begin;
    return mframe_reserve_iter(begin, end, mframe, mbegin, mend);
}

// enable filter if non-negetive value
void mframe_dump_iter(unsigned char* mptr, char* begin, char* end, int depth, int filter){
    unsigned char status = get_status(mptr);
    // for(int i=0; i<depth; ++i) uart_send((i%2)?' ':'|');
    if(filter < 0)
        uart_printf("%d", depth);
    if(status == MFRAME_FREE){
        if(filter < 0 || status == filter)
            uart_printf("\t["STYLE("FREE", FONT_GREEN)"] shift: %d 0x%x to 0x%x\n", get_shift(mptr), begin, end);
        return;
    }
    else if(status == MFRAME_DIVIDE){
        if(filter < 0 || status == filter)
            uart_printf("\t["STYLE("DIVI", FONT_BLUE)"] shift: %d 0x%x to 0x%x\n", get_shift(mptr), begin, end);
        mframe_dump_iter(get_left(mptr), begin, get_mid(begin, end), depth + 1, filter);
        mframe_dump_iter(get_right(mptr), get_mid(begin, end), end, depth + 1, filter);
        return;
    }
    else if(status == MFRAME_USED){
        if(filter < 0 || status == filter)
            uart_printf("\t["STYLE("USED", FONT_RED)"] shift: %d 0x%x to 0x%x\n", get_shift(mptr), begin, end);
        return;
    }
    else if(status == MFRAME_RESERVE){
        if(filter < 0 || status == filter)
            uart_printf("\t["STYLE("RESV", FONT_RED)"] shift: %d 0x%x to 0x%x\n", get_shift(mptr), begin, end);
        return;
    }
    debug("never go here");
}

void mframe_dump(int filter){
    return mframe_dump_iter(mframe, mbegin, mend, 0, filter);
}


char* mem_begin = (char*)0;
char* mem_end   = (char*)0x3C000000;

int mframe_get_mem_section(char* nodename, char* propname, char* propvalue, unsigned int proplen){
    if(0 == m_strcmp(nodename, "memory@0") && 0 == m_strcmp(propname, "reg")){
        unsigned int* ptr = (unsigned int *)propvalue;
        mem_begin = (char*)(long)read_bigendian(ptr);
        mem_end = (char*)(long)read_bigendian(ptr+1);
        debug("mem_begin end %x %x", mem_begin, mem_end);
        return 0;
    }
    return -1;
}

int mframe_get_mem_reservation(unsigned long long addr, unsigned long long size){
    uart_printf("%l %l\n", addr, size);
    return 0;
}

int mframe_init(){
    fdt_traverse(mframe_get_mem_section);
    unsigned long shift = 1;
    unsigned long size = (unsigned long)(mem_end - mem_begin);
    // aligned size to next power of 2
    while((1<<shift) < size) shift++;
    mframe[0] = shift;
    mbegin = mem_begin;
    mend   = mbegin + (1<<shift);
    debug("mframe init section 0x%x 0x%x", mbegin, mend);
    
    // reserve for unexisted memory (truncate)
    debug("reserve for unexisted memory 0x%x 0x%x", mem_end, mend);
    mframe_reserve((char*)mem_end, mend);

    // compute mframe size
    unsigned long mframe_size = (size / PAGE_SIZE) << 1;
    char* mframe_end = (char*)mframe + mframe_size;
    
    // reserve 0x0 to 0x1000 spin tables
    // reserve 0x1000 to 0x80000 heap and stack
    // reserve 0x80000 to 0x8xxxx kernel code
    // reserve 0x8xxxx to mframe_end 
    debug("reserve for spin table, heap, stack, kernel, mframe 0x%x 0x%x", 0, mframe_end);
    mframe_reserve((char*)0, mframe_end);

    // reserve for iniramfs
    cpio_path cpio;
    if(0 == cpio_get_start_addr(&cpio.next)){
        char *cpio_begin = cpio.next;
        while(0 == cpio_parse(&cpio));
        char *cpio_end = cpio.next;
        debug("reserve for iniramfs 0x%x 0x%x", cpio_begin, cpio_end);
        mframe_reserve(cpio_begin, cpio_end);
    }
    

    // reserve for device tree
    char* dt_begin = 0;
    char* dt_end = 0;
    if(0 == fdt_get_begin_end(&dt_begin, &dt_end)){
        debug("reserve for device tree 0x%x 0x%x", dt_begin, dt_end);
        mframe_reserve(dt_begin, dt_end);
    }


    // mframe_dump(-1);
    return 0;
}