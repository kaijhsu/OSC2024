#ifndef _KENREL_H__
#define _KERNEL_H__

#include "mini_uart.h"
#include "shell.h"
#include "peripherals/devicetree.h"
#include "utils.h"
#include "timer.h"
#include "mm.h"
#include "exception.h"
#include "thread.h"
#include "mmu.h"


int kernel_main(fdt_header*);

#endif