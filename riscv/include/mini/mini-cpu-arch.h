#ifndef MINI__MINI_CPU_ARCH_H
#define MINI__MINI_CPU_ARCH_H

#include <linux/mini.h>
#include <pthread.h>
#include <stdbool.h>

#include "mini/mini.h"

struct mini_cpu {
	pthread_t	thread;

	unsigned long   cpu_id;

	unsigned long	riscv_xlen;
	unsigned long	riscv_isa;
	unsigned long	riscv_timebase;

	struct mini	*mini;
	int		vcpu_fd;
	struct kvm_run	*mini_run;
	struct mini_cpu_task	*task;

	u8		is_running;
	u8		paused;
	u8		needs_nmi;

	struct mini_coalesced_mmio_ring	*ring;
};

#endif // MINI__MINI_CPU_ARCH_H
