#include "shell.h"
#include "m_string.h"
#include "devicetree.h"

typedef struct cmds_t {
	char* name;
    char* desc;
	int (*funct)(int, char **);
} cmds_t;

int hello(int argc, char *argv[]){
    uart_printf("Hello World!\n");
    return 0;
}

int read(int argc, char *argv[]){
    if(argc < 2){
        uart_printf("Usage: r <hex addr>\n");
        return -1;
    }
    char *addr = (void *)(long)m_htoi(argv[1]); // addr = 0x80000, *addr = value in 0x80000;
    uart_send_hex((unsigned int *)&addr);
    uart_printf(" ");
    uart_send_hex((unsigned int *)addr);
    uart_printf("\n");
    return 0;
}

int write(int argc, char *argv[]){
    if(argc < 3){
        uart_printf("Usage: w <addr> <bytes>");
        return -1;
    }
    char *addr = (void *)(long)m_htoi(argv[1]);
    int bytes = m_atoi(argv[2]);
    unsigned int check_sum = 0;
    for(int i=0; i<bytes; ++i){
        addr[i] = uart_recv_raw();
        check_sum += addr[i];
    }
    uart_printf("%s %s %s %d\n", argv[0], argv[1], argv[2], check_sum);
    return 0;
}

int jump(int argc, char *argv[]){
    if(argc < 2){
        uart_printf("Usage: jump <addr>");
    }
    int addr = m_htoi(argv[1]);
    void (*target)(char *) = (void *)(long)addr;
    target(devicetree_addr);
    return 0;
}

int check(int argc, char *argv[]){
    uart_printf("devicetree_addr: ");
    uart_send_hex(&devicetree_addr);
    uart_printf("\n");
    return 0;
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
        
        if(argc){
            if(0 == m_strcmp(argv[0], "hello"))
                hello(argc, argv);
            else if(0 == m_strcmp(argv[0], "w"))
                write(argc, argv);
            else if(0 == m_strcmp(argv[0], "r"))
                read(argc, argv);
            else if(0 == m_strcmp(argv[0], "jump"))
                jump(argc, argv);
            else if(0 == m_strcmp(argv[0], "check"))
                check(argc, argv);
            else if(0 == m_strcmp(argv[0], "reboot"))
                reboot(argc, argv);
            else if(0 == m_strcmp(argv[0], "help"))
                uart_printf("help:  print help menu\n"
                            "hello: print Hello World!\n"
                            "reboot: reboot\n"
                            "jump:  jump to <addr>\n"
                            "w: write <addr> <bytes>\n"
                            "r: read <addr>\n"
                            );
            else
                uart_printf("Unknown command: %s\n", argv[0]);
        }   
	}
    return 0;
}