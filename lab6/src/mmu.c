#include "mmu.h"


#define PGD_PTR ((PENTRY *)0x0)
#define PUD_PTR ((PENTRY *)0x1000)

void mmu_set_entry(uint64_t* entry_addr, void* physical_addr, uint64_t attribute){
    // check
    if((uint64_t) physical_addr & 0xFFF){
        kprintf("invalid entry_addr\n");
        while(1);
    }
    *entry_addr = (uint64_t)physical_addr | attribute;
}

int mmu_init_ret = -7;

uint64_t va_kernel(uint64_t ptr){
    return (mmu_init_ret == 0)? va_offset + ptr: ptr;
}

int mmu_init(){
    // prevent init twice
    if(mmu_init_ret != -7)
        return mmu_init_ret;

    // store lr for later indirect jump
    uint64_t lr_to_kernel_main;
    asm volatile("mov %0, lr\n\t":"=r"(lr_to_kernel_main));

    // settting up TCR_EL1
    uint64_t tcr_el1 = TCR_CONFIG_DEFAULT;
    tcr_el1 &= 0x00000000FFFFFFFF;
    asm volatile("msr tcr_el1, %0"::"r" (tcr_el1));

    // setting up MAIR_EL1
    uint64_t mair_el1 = 
        (MAIR_DEVICE_nGnRnE << (MAIR_IDX_DEVICE << 3)) |
        (MAIR_NORMAL_NOCACHE << (MAIR_IDX_NORMAL << 3));

    asm volatile("msr mair_el1, %0"::"r" (mair_el1));

    // Setting up kernel Page Table
    // A page claim 0x1000 size (4KB)
    // Let pgd place in 0x0 -> 0x1000
    // Let pud place in 0x1000 -> 0x2000
    PENTRY *pgd_ptr = PGD_PTR;
    PENTRY *pud_ptr = PUD_PTR;
    // the first entry of PGD -> PUD

    pgd_ptr[0] = NEW_PENTRY(1, PD_TABLE, MAIR_IDX_NORMAL, 0, 0, 1, (void*)pud_ptr, 0, 0);
    pud_ptr[0] = NEW_PENTRY(1, PD_BLOCK, MAIR_IDX_NORMAL, 0, 0, 1, (void*)0x0, 0, 0);
    pud_ptr[1] = NEW_PENTRY(1, PD_BLOCK, MAIR_IDX_DEVICE, 0, 0, 1, (void*)PUD_ENTRY_SIZE, 0, 0);

    asm volatile(
        "msr ttbr0_el1, %0\n\t"
        "msr ttbr1_el1, %0\n\t"
        ::"r"(pgd_ptr)
    );

    // enable MMU
    kprintf("Enabling mmu... ...");
    uint64_t sctlr_el1;
    asm volatile("mrs %0, sctlr_el1\n\t":"=r"(sctlr_el1));
    sctlr_el1 |= 0x1;
    asm volatile("msr sctlr_el1, %0\n\t"::"r"(sctlr_el1));
    uart_printf(STYLE("success\n", FONT_GREEN));

    // reset sp to va
    mmu_init_ret = 0;
    uint64_t va = va_offset;

    // indirect jump to virtual address
    // change stack pointer to virtual address
    uint64_t lr = lr_to_kernel_main + va_offset;
    asm volatile("add lr, lr, %0\n\t"
                 "add sp, sp, %0\n\t"
                 "br  %1"
                 ::"r"(va), "r"(lr));
    return mmu_init_ret = 0;
}

int mmu_finer_granularity(){
    uint64_t mend = mend_get();

    PENTRY* pud_ptr = (PENTRY*)PENTRY_ADDR(PGD_PTR);
    
    PENTRY* pmd_ptr = (PENTRY *)((uint64_t)mframe_alloc(PAGE_SIZE) + va_offset);
    for(int i=0; i<512; ++i){
        uint8_t mair_idx = ((i * PMD_ENTRY_SIZE) >= mend)? MAIR_IDX_DEVICE: MAIR_IDX_NORMAL;
        pmd_ptr[i] = NEW_PENTRY(1, PD_BLOCK, mair_idx, 0, 0, 1, i * PMD_ENTRY_SIZE, 0, 0);
    }
    pud_ptr->addr_page = (uint64_t)pmd_ptr >> 12;
    pud_ptr->table = PD_TABLE;
    return 0;
}