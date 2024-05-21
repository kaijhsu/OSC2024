#include "mini_uart.h"
#include "shell.h"
#include "peripherals/devicetree.h"
#include "utils.h"
#include "timer.h"
#include "mm.h"
#include "exception.h"
#include "thread.h"

#define init_system(name, fptr) ({ 				\
	uart_printf(name" start... ...\n");		\
	if(fptr()){ 							\
		uart_printf(name" start... ... "STYLE("fail", FONT_RED)); \
		return -1;							\
	} 										\
	uart_printf(name" start... ... "STYLE("success", FONT_GREEN)"\n"); \
})

int kernel_main(fdt_header *devicetree_ptr) {
	
	set_devicetree_addr(devicetree_ptr);
	set_exception_vector_table();
	
	uart_init();
	
	timer_init();
	uart_printf("Timer system initializes... ... "STYLE("success", FONT_GREEN)"\n");

	init_system("Buddy system inilization", mframe_init);

	set_interrupt_el1(1);
	uart_printf("Interrput mechanism opening... ... "STYLE("success", FONT_GREEN)"\n");
	
	init_system("Thread system inilization", thread_init);

	shell_loop();
	return 0;
}