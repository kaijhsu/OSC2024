#include "peripherals/devicetree.h"
#include "m_string.h"
#include "mini_uart.h"

fdt_header* devicetree_ptr = 0;

int set_devicetree_addr(fdt_header *addr){
    devicetree_ptr = addr;
    return 0;
}

char* fdt_align(char *p, int len){
    int tail = len % 4;
    int pad = (4-tail)%4;
    return p + len + pad;
}

unsigned int read_bigendian(unsigned int *ptr){
    unsigned int ret = 0;
    unsigned int num = *ptr;
    for(int i=0; i<4; ++i){
        ret <<= 8;
        ret += num & 0x000000FF;
        num >>= 8;
    }
    return ret;
}

unsigned long long read_bigendian64(unsigned long long *ptr){
    unsigned long long ret = 0;
    unsigned long long num = *ptr;
    for(int i=0; i<8; ++i){
        ret <<= 8;
        ret += num & 0x000000FF;
        num >>= 8;
    }
    return ret;
}

int fdt_get_begin_end(char** begin, char **end){
    if(devicetree_ptr == 0){
        *begin = 0;
        *end = 0;
        return -1;
    }
    *begin = (char*) devicetree_ptr;
    *end = *begin + read_bigendian(&devicetree_ptr->totalsize);
    return 0;
}

int fdt_traverse_mem_reserve(int (*callback)(unsigned long long, unsigned long long)){
    char* dt_addr = (char*) devicetree_ptr;
    char* dt_mem_reserve_addr = dt_addr + read_bigendian(&(devicetree_ptr->off_mem_rsvmap));
    struct fdt_reserve_entry *ptr = (struct fdt_reserve_entry*) dt_mem_reserve_addr;
    while(1){
        unsigned long long addr = read_bigendian64(&(ptr->address));
        unsigned long long size = read_bigendian64(&(ptr->size));
        if(addr == 0 && size == 0)
            break;
        callback(addr, size);
        ptr++;
    }
    return 0;
}

int fdt_traverse(int (*fdt_callback)(char *, char *, char *, unsigned int)){
    char* dt_addr = (char *)devicetree_ptr;
    char* dt_struct_addr =
        dt_addr + read_bigendian(&(devicetree_ptr->off_dt_struct));
    char* dt_strings_addr =
        dt_addr + read_bigendian(&(devicetree_ptr->off_dt_strings));
    
    char* ptr = dt_struct_addr;
    char* nodename;
    while(1){

        unsigned int token = read_bigendian((unsigned int*) ptr);
        ptr += 4;
        
        if(token == FDT_BEGIN_NODE){
            nodename = ptr;
            ptr = fdt_align(ptr, m_strlen(nodename) + 1);
        }
        else if(token == FDT_PROP){
            unsigned int proplen = read_bigendian((unsigned int*) ptr);
            ptr += 4;
            char *propname = dt_strings_addr + read_bigendian((unsigned int*) ptr);
            ptr += 4;
            char *propvalue = ptr;
            ptr = fdt_align(ptr, proplen);
            if(0 == fdt_callback(nodename, propname, propvalue, proplen)) break;
        }
        else if(token == FDT_END_NODE)
            continue;
        else if(token == FDT_NOP)
            continue;
        else if(token == FDT_END)
            break;
        else {
            uart_printf("%s:%d Device Tree: Unknown tag.\n", __FILE__, __LINE__);
            break;
        }
    }
    return 0;
}



