#ifndef	_P_IRQ_H
#define	_P_IRQ_H

#include "peripherals/base.h"

// From BCM2835 p112
#define IRQ_BASIC_PENDING	(PBASE+0x0000B200)
#define IRQ_PENDING_1		(PBASE+0x0000B204)
#define IRQ_PENDING_2		(PBASE+0x0000B208)
#define ENABLE_IRQS_1		(PBASE+0x0000B210)

#define ENABLE_IRQS_2		(PBASE+0x0000B214)
#define ENABLE_BASIC_IRQS	(PBASE+0x0000B218)
#define DISABLE_IRQS_1		(PBASE+0x0000B21C)
#define DISABLE_IRQS_2		(PBASE+0x0000B220)
#define DISABLE_BASIC_IRQS	(PBASE+0x0000B224)

#define LOCAL_TIMER_IRQ		(1 << 11)

// QA7 Page 16. 
#define CORE0_TIMER_CNT     0x40000040
#define CORE0_MAILBOX_CNT   0x40000050
#define CORE0_IRQ_SRC       0x40000060
#define SYSTEM_TIMER_IRQ_0	(1 << 0)
#define SYSTEM_TIMER_IRQ_1	(1 << 1)
#define SYSTEM_TIMER_IRQ_2	(1 << 2)
#define SYSTEM_TIMER_IRQ_3	(1 << 3)
#define GPU_IRQ             (1 << 8)


// BCM2837 Page 9. Enable mini UART interrupt
#define miniUART_IRQ		(1 << 0)
// BCM2837 Page 113. Enable AUx int
#define AUX_INT			(1 << 29)

#endif  /*_P_IRQ_H */