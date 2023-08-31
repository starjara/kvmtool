#ifndef MINI_MINI_H
#define MINI_MINI_H

#include "mini/mini.h"
#include "mini/mini-config.h"
#include "kvm/kvm-config.h"
#include "kvm/kvm-arch.h"

#include <stdbool.h>
#include <linux/types.h>


struct mini {
//    struct kvm_arch     arch;
    struct mini_config   cfg;
    int     sys_fd;
    int     vm_fd;

//    u32     mem_slots;
//    u64     mem_size;
    void    *ram_start;
    u64     ram_pagesize;

	struct mutex		mem_banks_lock;
	struct list_head	mem_banks;
};

struct mini *mini__new(void);
int mini__get_vm_type(struct mini *mini);
void mini__arch_init(struct mini *mini);
void mini__init_ram (struct mini *mini);
//int mini__register_mem(struct mini *mini,  u64 guest_phys, u64 size, void *userspace_addr, enum kvm_mem_type type);

/*
static inline int mini__register_ram(struct mini *mini, u64 guest_phys, u64 size,
				    void *userspace_addr)
{
	return mini__register_mem(mini, guest_phys, size, userspace_addr, KVM_MEM_TYPE_RAM);
}
*/

#endif
