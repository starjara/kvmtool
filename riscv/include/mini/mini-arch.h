#ifndef MINI__MINI_ARCH_H 
#define MINI__MINI_ARCH_H 

#include <stdbool.h>
#include <linux/const.h>
#include <linux/sizes.h>
#include <linux/types.h>

#define RISCV_RAM		0x80000000ULL

#define RISCV_LOMAP_MAX_MEMORY	((1ULL << 32) - RISCV_RAM)
#define RISCV_HIMAP_MAX_MEMORY	((1ULL << 40) - RISCV_RAM)

#if __riscv_xlen == 64
#define RISCV_MAX_MEMORY(mini)	RISCV_HIMAP_MAX_MEMORY
#elif __riscv_xlen == 32
#define RISCV_MAX_MEMORY(mini)	RISCV_LOMAP_MAX_MEMORY
#endif

struct mini_arch {
	/*
	 * We may have to align the guest memory for virtio, so keep the
	 * original pointers here for munmap.
	 */
	void	*ram_alloc_start;
	u64	ram_alloc_size;

	/*
	 * Guest addresses for memory layout.
	 */
	u64	memory_guest_start;
	u64	kern_guest_start;
	u64	initrd_guest_start;
	u64	initrd_size;
	u64	dtb_guest_start;
};

#endif 
