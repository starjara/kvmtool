#include "mini/mini.h"
#include "kvm/kvm.h"
#include "kvm/util.h"

#include <linux/kernel.h>
#include <linux/mini.h>
#include <linux/sizes.h>

void mini__arch_init(struct mini *mini)
{
    pr_debug("[lkvm] mini__arch_init");

	mini->ram_size = min(mini->cfg.ram_size, (u64)RISCV_MAX_MEMORY(mini));
	mini->arch.ram_alloc_size = mini->ram_size + SZ_1;
    mini->arch.ram_alloc_start = mmap_anon_or_hugetlbfs_mini(mini, 
            mini->cfg.hugetlbfs_path,
            mini->arch.ram_alloc_size);

    pr_debug("\t[lkvm] mini->ram_size: 0x%lx", (unsigned long int)mini->ram_size);
    pr_debug("\t[lkvm] mini->arch.ram_alloc_start : 0x%lx", (unsigned long int)mini->arch.ram_alloc_start);
    pr_debug("\t[lkvm] mini->arch.ram_alloc_size : 0x%lx", (unsigned long int)mini->arch.ram_alloc_size);

    if (mini->arch.ram_alloc_start == MAP_FAILED) {
        die("Failed to map %lld bytes for guest memory(%d)",
                mini->arch.ram_alloc_size, errno);                  
    }

	mini->ram_start = (void *)ALIGN((unsigned long)mini->arch.ram_alloc_start, SZ_1);

    pr_debug("\t[lkvm] mini->ram_start : 0x%lx", (unsigned long)mini->ram_start);

    madvise(mini->arch.ram_alloc_start, mini->arch.ram_alloc_size,
            MADV_MERGEABLE);
    madvise(mini->arch.ram_alloc_start, mini->arch.ram_alloc_size,
            MADV_HUGEPAGE);

}

void mini__init_ram (struct mini *mini)
{
	int err;
	u64 phys_start, phys_size;
	void *host_mem;

	phys_start	= RISCV_RAM;
	phys_size	= mini->ram_size;
	host_mem	= mini->ram_start;

    pr_debug("[lkvm] mini__init_ram");
	err = mini__register_ram(mini, phys_start, phys_size, host_mem);
	if (err)
		die("Failed to register %lld bytes of memory at physical "
		    "address 0x%llx [err %d]", phys_size, phys_start, err);

	mini->arch.memory_guest_start = phys_start;

    pr_debug("\t[lkvm] mini->arch.memory_guest_start : 0x%llx", mini->arch.memory_guest_start);
}
