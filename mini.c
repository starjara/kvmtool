#include "mini/mini.h"

#include <linux/mini.h>
#include <linux/list.h>
#include <linux/err.h>

#include <sys/ioctl.h>
#include <asm/unistd.h>

struct mini *mini__new(void)
{
    struct mini *mini = calloc(1, sizeof(*mini));
    if(!mini)
        return ERR_PTR(-ENOMEM);

    mutex_init(&mini->mem_banks_lock);
    mini->sys_fd = -1;
    mini->vm_fd = -1;

    return mini;
}

int mini__register_mem(struct mini *mini, u64 guest_phys, u64 size,
		      void *userspace_addr, enum mini_mem_type type)
{
	struct mini_userspace_memory_region mem;
	struct mini_mem_bank *merged = NULL;
	struct mini_mem_bank *bank;
	struct list_head *prev_entry;
	u32 slot;
	u32 flags = 0;
    int ret;

    pr_debug("[lkvm] mini__register_mem");

	mutex_lock(&mini->mem_banks_lock);
	/* Check for overlap and find first empty slot. */
	slot = 0;
	prev_entry = &mini->mem_banks;
	list_for_each_entry(bank, &mini->mem_banks, list) {
		u64 bank_end = bank->guest_phys_addr + bank->size - 1;
		u64 end = guest_phys + size - 1;
		if (guest_phys > bank_end || end < bank->guest_phys_addr) {
			/*
			 * Keep the banks sorted ascending by slot, so it's
			 * easier for us to find a free slot.
			 */
			if (bank->slot == slot) {
				slot++;
				prev_entry = &bank->list;
			}
			continue;
		}

		/* Merge overlapping reserved regions */
		if (bank->type == MINI_MEM_TYPE_RESERVED &&
		    type == MINI_MEM_TYPE_RESERVED) {
			bank->guest_phys_addr = min(bank->guest_phys_addr, guest_phys);
			bank->size = max(bank_end, end) - bank->guest_phys_addr + 1;

			if (merged) {
				/*
				 * This is at least the second merge, remove
				 * previous result.
				 */
				list_del(&merged->list);
				free(merged);
			}

			guest_phys = bank->guest_phys_addr;
			size = bank->size;
			merged = bank;

			/* Keep checking that we don't overlap another region */
			continue;
		}

		pr_err("%s region [%llx-%llx] would overlap %s region [%llx-%llx]",
		       mini_mem_type_to_string(type), guest_phys, guest_phys + size - 1,
		       mini_mem_type_to_string(bank->type), bank->guest_phys_addr,
		       bank->guest_phys_addr + bank->size - 1);

		ret = -EINVAL;
		goto out;
	}

	if (merged) {
		ret = 0;
		goto out;
	}

	bank = malloc(sizeof(*bank));
	if (!bank) {
		ret = -ENOMEM;
		goto out;
	}

	INIT_LIST_HEAD(&bank->list);
	bank->guest_phys_addr		= guest_phys;
	bank->host_addr			= userspace_addr;
	bank->size			= size;
	bank->type			= type;
	bank->slot			= slot;

    pr_debug("\t[lkvm] guest_phys_addr : 0x%lx, host_addr : 0x%lx, size : 0x%lx", (long)guest_phys, (long)userspace_addr, (long)size);

	if (type & MINI_MEM_TYPE_READONLY)
		flags |= MINI_MEM_READONLY;

	if (type != MINI_MEM_TYPE_RESERVED) {
		mem = (struct mini_userspace_memory_region) {
			.slot			= slot,
			.flags			= flags,
			.guest_phys_addr	= guest_phys,
			.memory_size		= size,
			.userspace_addr		= (unsigned long)userspace_addr,
		};

		ret = ioctl(mini->vm_fd, MINI_SET_USER_MEMORY_REGION, &mem);
		if (ret < 0) {
			ret = -errno;
			goto out;
		}
	}

	list_add(&bank->list, prev_entry);
	mini->mem_slots++;
	ret = 0;

out:
	mutex_unlock(&mini->mem_banks_lock);
	return ret;
}
