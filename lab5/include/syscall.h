#ifndef _SYSCALL_H__
#define _SYSCALL_H__

#include "stddef.h"
#include "mm.h"

/**
 * syscall_hander(): check argument, redirect to correspond syscall
 * @sp: frame pointer
*/
void syscall_handler(uint64_t* sp, void* pc);

#endif