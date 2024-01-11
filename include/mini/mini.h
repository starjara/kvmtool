#ifndef MINI__MINI_H
#define MINI__MINI_H

#include "kvm/mutex.h"

#include "mini/mini.h"
//#include "mini/mini-arch.h"
#include "kvm/kvm-arch.h"
#include "mini/mini-config.h"

#include <stdbool.h>
#include <linux/types.h>
#include <linux/list.h>

#define MINI_BINARY_NAME		"lkvm"

enum mini_mem_type {
	MINI_MEM_TYPE_RAM	= 1 << 0,
	MINI_MEM_TYPE_DEVICE	= 1 << 1,
	MINI_MEM_TYPE_RESERVED	= 1 << 2,
	MINI_MEM_TYPE_READONLY	= 1 << 3,

	MINI_MEM_TYPE_ALL	= MINI_MEM_TYPE_RAM
				| MINI_MEM_TYPE_DEVICE
				| MINI_MEM_TYPE_RESERVED
				| MINI_MEM_TYPE_READONLY
};

struct mini_mem_bank {
	struct list_head	list;
	u64			guest_phys_addr;
	void			*host_addr;
	u64			size;
	enum mini_mem_type	type;
	u32			slot;
};

struct mini {
    struct kvm_arch     arch;
    struct mini_config   cfg;
    int     sys_fd;
    int     vm_fd;

	int			nrcpus;		/* Number of cpus to run */
	struct mini_cpu		**cpus;

    u32     mem_slots;
	u64     ram_size;	/* Guest memory size, in bytes */

    void    *ram_start;
    u64     ram_pagesize;

	struct mutex		mem_banks_lock;
	struct list_head	mem_banks;
};

struct mini *mini__new(void);
int mini__get_vm_type(struct mini *mini);
void mini__arch_init(struct mini *mini);
void mini__init_ram (struct mini *mini);
int mini__register_mem(struct mini *mini,  u64 guest_phys, u64 size, void *userspace_addr, enum mini_mem_type type);

static inline int mini__register_ram(struct mini *mini, u64 guest_phys, u64 size,
				    void *userspace_addr)
{
	return mini__register_mem(mini, guest_phys, size, userspace_addr, MINI_MEM_TYPE_RAM);
}

#define add_read_only_(type, str)					\
	(((type) & MINI_MEM_TYPE_READONLY) ? str " (read-only)" : str)
static inline const char *mini_mem_type_to_string(enum mini_mem_type type)
{
	switch (type & ~MINI_MEM_TYPE_READONLY) {
	case MINI_MEM_TYPE_ALL:
		return "(all)";
	case MINI_MEM_TYPE_RAM:
		return add_read_only_(type, "RAM");
	case MINI_MEM_TYPE_DEVICE:
		return add_read_only_(type, "device");
	case MINI_MEM_TYPE_RESERVED:
		return add_read_only_(type, "reserved");
	}

	return "???";
}
#endif
