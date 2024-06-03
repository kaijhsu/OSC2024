#ifndef __INITRAMFS_H
#define __INITRAMFS_H

#include "m_string.h"
#include "mini_uart.h"
#include "mailbox.h"
#include "peripherals/devicetree.h"
#include "stddef.h"

typedef struct cpio_newc_header {
    char    c_magic[6];
    char    c_ino[8];
    char    c_mode[8];
    char    c_uid[8];
    char    c_gid[8];
    char    c_nlink[8];
    char    c_mtime[8];
    char    c_filesize[8];
    char    c_devmajor[8];
    char    c_devminor[8];
    char    c_rdevmajor[8];
    char    c_rdevminor[8];
    char    c_namesize[8];
    char    c_check[8];
} cpio_newc_header;

typedef struct cpio_path {
    char *name;
    char *data;
    size_t namesize;
    size_t filesize;
    unsigned int mode;
    char *next;
} cpio_path;

#define CPIO_FILE 56
#define CPIO_DIR  52

int cpio_get_start_addr(char **addr);
int cpio_parse(cpio_path*);
cpio_path cpio_search(char* path);
void cpio_ls();


#endif