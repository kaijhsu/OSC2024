#include "kernel.h"

#define init_system(name, fptr) ({ 				\
	uart_printf(name" start... ...\n");		\
	if(fptr()){ 							\
		uart_printf(name" start... ... "STYLE("fail!\nSTALL!\n", FONT_RED)); \
		while(1);							\
	} 										\
	uart_printf(name" start... ... "STYLE("success!", FONT_GREEN)"\n"); \
})

int kernel_main(fdt_header *devicetree_ptr) {
	set_devicetree_addr(devicetree_ptr);
	
	uart_init();
	uart_printf("Kernel wake up uart... ..."STYLE("success!", FONT_GREEN)"\n");
	init_system("MMU system inilization", mmu_init);
	
	set_exception_vector_table();
	
	init_system("Buddy system inilization", mframe_init);
	
	mmu_finer_granularity();

	timer_init();
	uart_printf("Timer system initializes... ... "STYLE("success!", FONT_GREEN)"\n");


	set_interrupt_el1(1);
	uart_printf("Interrput mechanism opening... ... "STYLE("success!", FONT_GREEN)"\n");
	
	// init_system("Thread system inilization", thread_init);




	shell_loop();
	return 0;
}