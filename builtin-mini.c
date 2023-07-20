#include <kvm/util.h>
#include <kvm/kvm-cmd.h>
#include <kvm/builtin-mini.h>
#include <kvm/kvm.h>
#include "kvm/kvm-cpu.h"
#include "kvm/parse-options.h"
#include "kvm/read-write.h"
#include "kvm/builtin-debug.h"

#include <linux/err.h>
#include <linux/list.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>

__thread struct kvm_cpu *current_kvm_mini_cpu;

static int mem;
static const char *instance_name;

static const char * const mini_usage[] = {
    "lkvm mini -m [size] -n [name]",
    NULL
};

static const struct option mini_options[] = {
    OPT_GROUP("General options:"),
    OPT_INTEGER('m', "memory", &mem, "Memory size"),
    OPT_STRING('n', "name", &instance_name, "name", "Instance name"),
    OPT_BOOLEAN('\0', "debug", &do_debug_print, "Enable debug messages"),
    OPT_END()
};

static void parse_mini_options(struct kvm *kvm, int argc, const char **argv)
{
	while (argc != 0) {
		argc = parse_options(argc, argv, mini_options, mini_usage,
				PARSE_OPT_STOP_AT_NON_OPTION);
		if (argc != 0)
			kvm_mini_help();
    }

    kvm->cfg.ram_size = mem << 20;
    kvm->cfg.guest_name = instance_name;
}

void kvm_mini_help(void)
{
	printf("\n%s mini creates a small size guest virtual address space.\n"
		, KVM_BINARY_NAME);
	usage_with_options(mini_usage, mini_options);
}

static int kvm_struct_init (struct kvm *kvm)
{
    kvm->cfg.ram_addr = kvm__arch_default_ram_address();
    kvm->cfg.dev = DEFAULT_KVM_DEV;
    kvm->sys_fd = open(kvm->cfg.dev, O_RDWR);
    kvm->cfg.nrcpus = 1;

    if(ioctl(kvm->sys_fd, KVM_GET_API_VERSION, 0) != KVM_API_VERSION) {
        pr_err("KVM_API_VERSION ioctl");
        return -1;
    }

	kvm->vm_fd = ioctl(kvm->sys_fd, KVM_CREATE_VM, kvm__get_vm_type(kvm));
	if (kvm->vm_fd < 0) {
		pr_err("KVM_CREATE_VM ioctl");
        return -1;
    }

    kvm__arch_init(kvm);
    
    INIT_LIST_HEAD(&kvm->mem_banks);
    kvm__init_ram(kvm);

    return 0;
}

static struct kvm *kvm_cmd_mini_init (int argc, const char **argv) 
{
    struct kvm *kvm = kvm__new();
    
    if (IS_ERR(kvm))
            return kvm;

    parse_mini_options(kvm, argc, argv);

    if(kvm_struct_init(kvm) != 0) {
        free(kvm);
        return kvm;
    }

    if(kvm_cpu__init(kvm) != 0) {
        free(kvm);
        return kvm;
    }

    return kvm;
}

static void kvm_mini__reset_vcpu(struct kvm_cpu *vcpu)
{
	struct kvm *kvm = vcpu->kvm;
	struct kvm_mp_state mp_state;
	struct kvm_one_reg reg;
	unsigned long data;

	if (ioctl(vcpu->vcpu_fd, KVM_GET_MP_STATE, &mp_state) < 0)
		die_perror("KVM_GET_MP_STATE failed");

	reg.addr = (unsigned long)&data;

	data	= vcpu->cpu_id;
	reg.id	= RISCV_CORE_REG(regs.a0);
	if (ioctl(vcpu->vcpu_fd, KVM_SET_ONE_REG, &reg) < 0)
		die_perror("KVM_SET_ONE_REG failed (a0)");

	data	= kvm->arch.dtb_guest_start;
	reg.id	= RISCV_CORE_REG(regs.a1);
	if (ioctl(vcpu->vcpu_fd, KVM_SET_ONE_REG, &reg) < 0)
		die_perror("KVM_SET_ONE_REG failed (a1)");
}

static int kvm_mini__start(struct kvm_cpu *cpu)
{
    kvm_mini__reset_vcpu(cpu);

    while(cpu->is_running) {
        printf("cpu ruuning\n");
        kvm_cpu__show_registers(cpu);
        cpu->is_running = false;
    }

    return 0;
}

static void *kvm_mini_thread(void *arg)
{
    char name[16];

    current_kvm_mini_cpu = arg;

    sprintf(name, "kvm-cpu-%lu",  current_kvm_mini_cpu->cpu_id);

	kvm__set_thread_name(name);

    kvm_mini__start(current_kvm_mini_cpu);

	return (void *) (intptr_t) 0;

}

static int kvm_boot_mini(struct kvm *kvm) 
{
    int ret = 0;

    printf("kvm_boot_mini\n");

    pthread_create(&kvm->cpus[0]->thread, NULL, kvm_mini_thread, kvm->cpus[0]);

    pthread_join(kvm->cpus[0]->thread, NULL);

    return ret;
}

int kvm_cmd_mini(int argc, const char **argv, const char *prefix)
{
    int r = 0;

    static struct kvm *kvm;

    if(kvm == NULL) {
        kvm = kvm_cmd_mini_init(argc, argv);
        if (IS_ERR(kvm))
            return PTR_ERR(kvm);

        printf("vm create complete\n");
        printf("name : %s\n", kvm->cfg.guest_name);
        printf("size : %lld\n", kvm->cfg.ram_size);
    }
    kvm_boot_mini(kvm);


    /*
    struct kvm_one_reg reg;
    unsigned long data = 0x80e05;

    kvm_cpu__show_registers(kvm->cpus[0]);

    reg.addr = (unsigned long)&data;
    reg.id = RISCV_CSR_REG(satp);
	ioctl(kvm->cpus[0]->vcpu_fd, KVM_SET_ONE_REG, &reg);
    kvm_cpu__show_registers(kvm->cpus[0]);
    */

    printf("main done\n");


    /*
    struct kvm_one_reg reg;
    unsigned long satp = 0;

    reg.id = KVM_REG_RISCV_CSR_REG(satp);
    reg.addr = (unsigned long)&satp;

    ioctl(kvm->cpus[0]->vcpu_fd, KVM_GET_ONE_REG, &reg);
    pr_debug("reg : 0x%llx, addr : 0x%llx, val : 0x%lx\n", reg.id, reg.addr, satp);
    */

    while(1);

    return r;

}
