# Lab 6 Virtual Memory

在 ARM 架構中，Virtual Address 轉換到 Physical Address 會涉及到多層的轉換。在本次 Lab 中會使用到 4 層 Page。分別是：
- PGD Page Global Directory
- PUD Page Upper Directory
- PMD Page Middle Directory
- PTE Page Table Eentry

Page: 由 PTE entry 指向的一塊 virtual memory
Block: 由 PUD/PMD entry 指向的一塊 virtual memory
Page Frame: 一塊 Physical Memory
Page Table: PGD/PUD/PMD/PTE 
- 在每個 Page Table 中的 entry 都會指向下一層的 block/page，entry 中會儲存 page frame 的 physical address 以及屬性

Entry 的格式
```
Entry of PGD, PUD, PMD which point to a page table

+-----+------------------------------+---------+--+
|     | next level table's phys addr | ignored |11|
+-----+------------------------------+---------+--+
     47                             12         2  0

Entry of PUD, PMD which point to a block

+-----+------------------------------+---------+--+
|     |  block's physical address    |attribute|01|
+-----+------------------------------+---------+--+
     47                              n         2  0

Entry of PTE which points to a page

+-----+------------------------------+---------+--+
|     |  page's physical address     |attribute|11|
+-----+------------------------------+---------+--+
     47                             12         2  0

Invalid entry

+-----+------------------------------+---------+--+
|     |  page's physical address     |attribute|*0|
+-----+------------------------------+---------+--+
     47                             12         2  0
```

Attribute 的格式
- Bits[54] The unprivileged execute-never bit, 如果為 1，不能被 EL0 執行
- Bits[53] The privileged execute-never bit, 如果為 1，不能被 EL1 執行
- Bits[47:n] The physical address the entry point to. Address must be aligned to power of 2.
- Bits[10] The access flag, a page fault is generated if not set.
- Bits[7] 0 for read-write, 1 for read-only.
- Bits[6] 0 for only kernel access, 1 for user/kernel access.
- Bits[4:2] The index to MAIR.
- Bits[1:0] Specify the next level is a block/page, page table, or invalid.


## AArch64 memory layout
在 64bits 的 Arm 機器中，0xFFFF_XXXX_XXXX_XXXX 區段的 Virtual addresss 是給 Kernel Space 使用的，而低位 0x0000_XXXX_XXXX_XXXX 的 Virtual address 則給 User space 使用

在 0xFFFF_XXXX_XXXX_XXXX 中的 Kernel Space Virtual Address 會映射到 0x0 ~ 0x200000 的 physical Address (Kernel 實際的位置)


## Basic 1 在 Kernel Space 中啟用 Virtual Memory

### TCR_EL1
TCR_EL1 設置 Paging 有關的屬性

```c
#define TCR_CONFIG_REGION_48bit (((64 - 48) << 0) | ((64 - 48) << 16))
// 0b0001_0000_0000_0000_0001_0000
#define TCR_CONFIG_4KB ((0b00 << 14) |  (0b10 << 30))
// 0b0000_0000_0000_0000_0000
#define TCR_CONFIG_DEFAULT (TCR_CONFIG_REGION_48bit | TCR_CONFIG_4KB)

ldr x0, = TCR_CONFIG_DEFAULT
msr tcr_el1, x0
```

[TCR_EL1](https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/TCR-EL1--Translation-Control-Register--EL1-)
- b4  = 1
  - TCR_EL1[5:0] = 16 T0SZ
  - The size offset of the memory region addressed by TTBR0_EL1. The region size is $2^{(64-T0SZ)}$ bytes.
  - T0SZ = $2^{48}$
- b20 = 1
  - TCR_EL1[21:16] = 16 T1SZ
  - The size offset of the memory region addressed by TTBR1_EL1. The region size is $2^{(64-T0SZ)}$ bytes.
  - T1SZ = $2^{48}$
- b31 = 1
  - TCR_EL1[31:30] = 0b10 TG1
  - Granule size for the TTBR1_EL1.
  - 0b01 -> 16KB
  - 0b10 -> 4KB
  - 0b11 -> 64KB
- b[15:14] = 0b00 TG0
  - Granule size for the TTBR0_EL1
  - 0b00 - > 4KB
  - 0b01 - > 64KB
  - 0b10 - > 16KB

### MAIR_EL1
MAIR_EL1, Memory Attribute Indirection Register (EL1)
- 記憶體屬性間接 register
- 在 MMU 中，對於不同的記憶體區段可以設置不同的 policy，可以透過更改`MAIR_EL1`設定
- 當 MMU 得到 virtual address 的時候，MMU 會從 page table 得到 index 以及查詢 MAIR 得到 memory attribute，依此得到不同的 access auth 
- [docs](https://developer.arm.com/documentation/ddi0595/2021-06/AArch64-Registers/MAIR-EL1--Memory-Attribute-Indirection-Register--EL1-?lang=en)
- b[8n+7:8n] for n = 0 ~ 7, n 為 index 可以有 8 個值 0～7
- 在 Page Table Entry 中有 3bit [0~7] 會存 MAIR 的 index，MMU 會去看 MAIR register 對應位置的屬性得知該 entry 的屬性
- [GRE 的意思](https://developer.arm.com/documentation/den0024/a/Memory-Ordering/Memory-types/Device-memory)

### Identity Paging
在開啟 MMU 之前，必須先設置好 Kernel 的 Page Table，為了簡化設計使用 idnetity Paging ，這是 virtual address 與 physical address 一樣的意思，virtual address 即是 physical address

### Finer Granularity Paging
- PGD Table 可以指向 256TB 的空間
- PGD Entry 可以指向 512GB 的空間             (1 << 39)
- PUD Entry 可以指向 1GB   的空間 (2MB * 512) (1 << 30)
- PMD Entry 可以指向 2MB   的空間 (4KB * 512) (1 << 21)
- PTE Entry 可以指向 4KB   的空間             (1 << 12)

Raspi3b+ 的 memory size
```
[DEBUG] src/mm.c:307, mframe_get_mem_section: mem_begin end 0x0 0x3C000000
```



## Basic 2 在 User Space 中啟用 Virtual Memory

