#include "shell.h"
#include "exception.h"
#include "timer.h"
#include "m_string.h"
#include "mm.h"

typedef struct cmd_t {
    char *name;
    char *description;
    int  (* funct)(int, char **);
} cmd_t;




int hello(int argc, char *argv[]){
    uart_printf("Hello World! I'm main kernel!\n");
    return 0;
}

int board_info(int argc, char *argv[]){
    print_board_info();
    return 0;
}

int ls(int argc, char *argv[]){
    cpio_path path;
    if(cpio_get_start_addr(&path.next))
        return -1;
    while(0 == cpio_parse(&path)){
        uart_printf("%s\n", path.name);
    }
    return 0;
}

int cat(int argc, char *argv[]){
    if(argc < 2){
        uart_printf("Usage: cat <filepath>\n");
        return -1;
    }
    cpio_path path;
    if(cpio_get_start_addr(&path.next))
        return -1;
    while(0 == cpio_parse(&path)){
        if(0 == m_strcmp(path.name, argv[1])){
            if(path.mode != CPIO_FILE){
                uart_printf("%s is not a file!\n", path.name);
                return -1;
            }
            for(int i=0; i<path.filesize; ++i)
                uart_printf("%c", path.file[i]);
            uart_printf("\n");
            return 0;
        }
    }
    uart_printf("Not found!: %s\n", argv[1]);
    return -1;
}

int exec(int argc, char *argv[]){
    // LAB3-2 Load user program to memory
    if(argc < 2){
        uart_printf("Usage: exec <filename>\n");
        return -1;
    }
    char* pathname = argv[1];
    cpio_path path;
    if(cpio_get_start_addr(&path.next)){
        debug("");
        return -1;
    }
    while(0 == cpio_parse(&path)){
        if(0 == m_strcmp(path.name, pathname)){
            uart_printf("exec %s\n", pathname);
            char *addr = USER_PROCESS_ADDR;
            for(int i=0; i<path.filesize; ++i)
                addr[i] = path.file[i];
            branch_el1_to_el0(USER_PROCESS_ADDR, USER_PROCESS_SP);
            return 0;
        }
    }
    uart_printf("Not found!: %s\n", pathname);
    return -1;
}

void set(long addr, unsigned int value) {
    volatile unsigned int* point = (unsigned int*)addr;
    *point = value;
}

void reset(int tick) {
    uart_printf("reboot after %d ticks\n", tick);
    set(PM_RSTC, PM_PASSWORD | 0x20);  // full reset
    set(PM_WDOG, PM_PASSWORD | tick);  // number of watchdog tick
}

void cancel_reset() {
    uart_printf("cancel reboot \n");
    set(PM_RSTC, PM_PASSWORD | 0);  // full reset
    set(PM_WDOG, PM_PASSWORD | 0);  // number of watchdog tick
}


int reboot(int argc, char *argv[]){
    unsigned int tick = 10;
    if(argc >= 2)
        tick = m_atoi(argv[1]);
    if(tick < 0)
        cancel_reset();
    else
        reset(tick);
    return 0;
}

void timeout_cb(void *arg){
    char *mesg = (char *)arg;
    uart_printf("\n[TIMEOUT] Mesg: %s, Sec: %d\n", mesg, get_cpu_time());
}

int set_timeout(int argc, char *argv[]){
    if(argc != 3){
        uart_printf("Usage: %s <Mesg> <Second>\n", argv[0]);
        return -1;
    }
    unsigned long long sec = m_atoi(argv[2]);
    char *mesg = simple_malloc(m_strlen(argv[1])+1);
    m_sprintf(mesg, "%s\0", argv[1]);
    if(timer_add(sec, timeout_cb, mesg)){
        debug(""); // add timer fail;
        return -1;
    }
    uart_printf("\n[ADD TIMEOUT] Time: %d Mesg: %s, Sec: %d\n", get_cpu_time(), mesg, sec);
    return 0;
}

int demo_async(int argc, char **argv){
    demo_uart_async();
    return 0;
}

int help(int, char**);

cmd_t cmds[] = {
    {"help",    "print help menu",      help},
    {"hello",   "print Hello World!",   hello},
    {"board",   "print board info",     board_info},
    {"reboot",  "reboot the device",    reboot},
    {"ls",      "list directory",       ls},
    {"cat",     "dump file text",       cat},
    {"exec",    "execute file",         exec},
    {"demo_async","demo async uart",    demo_async},
    {"timeout", "set timeout",          set_timeout}
};

int help(int argc, char *argv[]){
    for(int i=0; i<sizeof(cmds)/sizeof(cmds[0]); ++i)
        uart_printf("%s\t\t%s\n", cmds[i].name, cmds[i].description);
    return 0;
}


int shell_loop(){
    while (1) {	
		char buf[256];
        unsigned int len = 0;
		uart_printf("$ ");
		while(len < 256){
			buf[len] = uart_recv();
            if(buf[len] == 127){
                if(len == 0) continue;
                uart_printf("\b \b");
                len--;
            }
            else{
			    uart_printf("%c", buf[len]);
                len++;
                if(buf[len-1] == '\n'){
                    buf[len-1] = '\0';
				    break;
			    }
            }
		}
        
        unsigned int argc = 0;
        char *argv[32];
        char *next = buf;
        char *token = m_strtok(buf, ' ', &next);
        while(*token != 0){
            argv[argc++] = token;
            token = m_strtok(next, ' ', &next);
        }

        if(argc == 0)
            continue;
        int valid = 0;
        for(int i=0; i<sizeof(cmds)/sizeof(cmds[0]); ++i){
            if(0 == m_strcmp(argv[0], cmds[i].name)){
                cmds[i].funct(argc, argv);
                valid = 1;
                break;
            }
        }
        if(!valid)
            uart_printf("Unknown command: %s\n", argv[0]);
	}
    return 0;
}