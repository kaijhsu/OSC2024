#ifndef _MMU_H__
#define _MMU_H__

#include "mini_uart.h"
#include "mm.h"

#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

/* MAIR explaination
*  MAIR_EL1 is a register splited into 8 values
*  MAIR, page attribute, points to MAIR_EL1 register
*/
#define MAIR_DEVICE_nGnRnE 0b00000000
#define MAIR_NORMAL_NOCACHE 0b01000100
#define MAIR_IDX_DEVICE 0
#define MAIR_IDX_NORMAL 1

#define PD_TABLE 1
#define PD_BLOCK 0
#define PD_ACCESS (1 << 10)
#define BOOT_PGD_ATTR (PD_TABLE) << 1 | 0b1
#define BOOT_PUD_ATTR (PD_ACCESS | (MAIR_IDX_DEVICE << 2) | PD_BLOCK << 1 | 1)

/* Page Entry Format */
typedef struct PENTRY_t {
    uint32_t valid        : 1;  // b[0]     valid bit
    uint32_t table        : 1;  // b[1]     1 === Page table, 0 === Block
    uint32_t mair_index   : 3;  // b[4:2]   MAIR index
    uint32_t attr_2       : 1;  // b[5]
    uint32_t user_access  : 1;  // b[6]     0 kernel, 1 user/kernel
    uint32_t read_only    : 1;  // b[7]     0 === r/w, 1 === r
    uint32_t attr_1       : 2;  // b[9:8]
    uint32_t access       : 1;  // b[10]    access flag
    uint32_t attr_0       : 1;  // b[11]    un-used for lab
    uint64_t addr_page    : 36; // b[47:12] Physical address the entry point to (4KB per unit), shift left 12 to get real physical address
    uint32_t up_attr_1    : 5;  // b[52:48]
    uint32_t non_exec_el1 : 1;  // b[53]    el1 Unpriviledge execute-never bit
    uint32_t non_exec_el0 : 1;  // b[54]    el0 Unpriviledge execute-never bit
    uint32_t up_attr_0    : 9;  // b[63:55] un-used for lab
} PENTRY;

#define NEW_PENTRY(valid, table, mair_idx, user_access, read_only, access, addr, non_exec_el1, non_exec_el0) \
    (PENTRY) {valid, table, mair_idx, 0, user_access, read_only, 0, access, 0, (uint64_t)addr >> 12, 0, non_exec_el1, non_exec_el1, 0}

#define PENTRY_ADDR(pentry_ptr) (((uint64_t)pentry_ptr->addr_page) << 12)

#define va_offset 0xFFFF000000000000
// 512 GB
#define PGD_ENTRY_SIZE (1 << 39)
// 1 GB
#define PUD_ENTRY_SIZE (1 << 30)
// 2 MB
#define PMD_ENTRY_SIZE (1 << 21)
// 4KB
#define PTE_ENTRY_SIZE (1 << 12)


uint64_t va_kernel(uint64_t addr);



/*
* mmu_init() - init of mmu
* set mmu_open to 1 
*/
int mmu_init();

/* mmu_finer_granularity() - set three-level translation (2MB) */
int mmu_finer_granularity();


/*
* mmu_open()
*/

#endif