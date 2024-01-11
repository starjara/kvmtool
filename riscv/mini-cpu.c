#include "mini/mini.h"
#include "mini/mini-cpu.h"
#include "kvm/virtio.h"

#include <asm/ptrace.h>

struct mini_cpu *mini_cpu__arch_init(struct mini *mini, unsigned long cpu_id)
{
	struct mini_cpu *vcpu;
	u64 timebase = 0;
	unsigned long isa = 0;
	//int coalesced_offset, mmap_size;
	int mmap_size;
	//struct mini_one_reg reg;

    pr_debug("[lkvm] mini_cpu__arch_init");

	vcpu = calloc(1, sizeof(struct mini_cpu));
	if (!vcpu)
		return NULL;

	vcpu->vcpu_fd = ioctl(mini->vm_fd, MINI_CREATE_VCPU, cpu_id);
	if (vcpu->vcpu_fd < 0)
		die_perror("[lkvm] MINI_CREATE_VCPU ioctl");

    /*
	reg.id = RISCV_CONFIG_REG(isa);
	reg.addr = (unsigned long)&isa;
	if (ioctl(vcpu->vcpu_fd, MINI_GET_ONE_REG, &reg) < 0)
		die("MINI_GET_ONE_REG failed (config.isa)");

	reg.id = RISCV_TIMER_REG(frequency);
	reg.addr = (unsigned long)&timebase;
	if (ioctl(vcpu->vcpu_fd, MINI_GET_ONE_REG, &reg) < 0)
		die("MINI_GET_ONE_REG failed (timer.frequency)");
        */

	mmap_size = ioctl(mini->sys_fd, MINI_GET_VCPU_MMAP_SIZE, 0);
	if (mmap_size < 0)
		die_perror("MINI_GET_VCPU_MMAP_SIZE ioctl");

	vcpu->mini_run = mmap(NULL, mmap_size, PROT_RW, MAP_SHARED,
			     vcpu->vcpu_fd, 0);
	if (vcpu->mini_run == MAP_FAILED)
		die("unable to mmap vcpu fd");
    pr_debug("\t[lmini] mmap : %p, size : %d", vcpu->mini_run, mmap_size);

    /*
	coalesced_offset = ioctl(mini->sys_fd, MINI_CHECK_EXTENSION,
				 MINI_CAP_COALESCED_MMIO);
	if (coalesced_offset)
		vcpu->ring = (void *)vcpu->mini_run +
			     (coalesced_offset * PAGE_SIZE);

	reg.id = RISCV_CONFIG_REG(isa);
	reg.addr = (unsigned long)&isa;
	if (ioctl(vcpu->vcpu_fd, MINI_SET_ONE_REG, &reg) < 0)
		die("MINI_SET_ONE_REG failed (config.isa)");
        */

	/* Populate the vcpu structure. */
	vcpu->mini		= mini;
	vcpu->cpu_id		= cpu_id;
	vcpu->riscv_isa		= isa;
	vcpu->riscv_xlen	= __riscv_xlen;
	vcpu->riscv_timebase	= timebase;
	vcpu->is_running	= true;

	return vcpu;
}


