#include <kvm/util.h>
#include <kvm/kvm-cmd.h>
#include <kvm/builtin-mini.h>
#include <kvm/kvm.h>
#include "kvm/mini.h"
#include "kvm/kvm-cpu.h"
#include "kvm/parse-options.h"
#include "kvm/read-write.h"
#include "kvm/builtin-debug.h"
#include "kvm/util-init.h"

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

int *a;

static const struct option mini_options[] = {
    OPT_GROUP("General options:"),
    OPT_INTEGER('m', "memory", &mem, "Memory size"),
    OPT_STRING('n', "name", &instance_name, "name", "Instance name"),
    OPT_BOOLEAN('\0', "debug", &do_debug_print, "Enable debug messages"),
    OPT_END()
};

/*
static void parse_mini_options(struct mini *mini, int argc, const char **argv)
{
	while (argc != 0) {
		argc = parse_options(argc, argv, mini_options, mini_usage,
				PARSE_OPT_STOP_AT_NON_OPTION);
		if (argc != 0)
			kvm_mini_help();
    }

    mini->cfg.ram_size = mem << 20;
    mini->cfg.guest_name = instance_name;
}
*/

static void parse_kvm_options(struct kvm *kvm, int argc, const char **argv)
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

/*
static int mini_struct_init (struct mini *mini)
{
    mini->cfg.ram_addr = kvm__arch_default_ram_address();
    mini->cfg.dev = DEFAULT_KVM_DEV;
    mini->sys_fd = open(mini->cfg.dev, O_RDWR);
    mini->cfg.nrcpus = 1;

    printf("[lkvm] mini_struct_init\n");

    if(ioctl(mini->sys_fd, KVM_GET_API_VERSION, 0) != KVM_API_VERSION) {
        pr_err("KVM_API_VERSION ioctl");
        return -1;
    }

	mini->vm_fd = ioctl(mini->sys_fd, KVM_CREATE_VM, mini__get_vm_type(mini));
    
	if (mini->vm_fd < 0) {
		pr_err("KVM_CREATE_VM ioctl");
        return -1;
    }

    mini__arch_init(mini);
    
    INIT_LIST_HEAD(&mini->mem_banks);
    mini__init_ram(mini);



    return 0;
}
*/

static int kvm_struct_init (struct kvm *kvm)
{
    kvm->cfg.ram_addr = kvm__arch_default_ram_address();
    kvm->cfg.dev = DEFAULT_KVM_DEV;
    kvm->sys_fd = open(kvm->cfg.dev, O_RDWR);
    kvm->cfg.nrcpus = 1;

    printf("[lkvm] kvm_struct_init\n");

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

/*
static struct mini *kvm_cmd_mini_init (int argc, const char **argv) 
{
    struct mini *mini = mini__new();
    
    if (IS_ERR(mini))
            return mini;

    parse_mini_options(mini, argc, argv);

    if(mini_struct_init(mini) != 0) {
        free(mini);
        return mini;
    }

   // if(kvm_cpu__init(mini) != 0) {
   //     free(kvm);
   //     return kvm;
   // }

    return mini;
}
*/
static struct kvm *kvm_cmd_struct_init (int argc, const char **argv) 
{
    struct kvm *kvm = kvm__new();
    
    if (IS_ERR(kvm))
            return kvm;

    parse_kvm_options(kvm, argc, argv);

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

    /*
	//data	= kvm->arch.kern_guest_start;
    data = 0x0000000000013e6e;
	reg.id	= RISCV_CORE_REG(regs.pc);
	if (ioctl(vcpu->vcpu_fd, KVM_SET_ONE_REG, &reg) < 0)
		die_perror("KVM_SET_ONE_REG failed (pc)");
    */

	data	= vcpu->cpu_id;
	reg.id	= RISCV_CORE_REG(regs.a0);
	if (ioctl(vcpu->vcpu_fd, KVM_SET_ONE_REG, &reg) < 0)
		die_perror("KVM_SET_ONE_REG failed (a0)");

	data	= kvm->arch.dtb_guest_start;
	reg.id	= RISCV_CORE_REG(regs.a1);
	if (ioctl(vcpu->vcpu_fd, KVM_SET_ONE_REG, &reg) < 0)
		die_perror("KVM_SET_ONE_REG failed (a1)");
}

/*
static int kvm_mini__start(struct kvm_cpu *cpu)
{
    kvm_mini__reset_vcpu(cpu);

    while(cpu->is_running) {
        printf("cpu ruuning\n");
        kvm_cpu__show_registers(cpu);

        a = malloc (sizeof(int) * 10);
        a[0] = 20;
        printf("%p %d %p\n", a, a[0], &a);

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
*/

static int kvm_load_mini(struct kvm *kvm) 
{
    int ret = 0;
    struct kvm_cpu *cur = kvm->cpus[0];

    printf("kvm_boot_mini\n");

    kvm_mini__reset_vcpu(cur);

    a = malloc(sizeof(int));
    *a = 10;
    printf("%p,\t%d\n", a, *a);

	ioctl(cur->vcpu_fd, KVM_INIT_MINI, 0);
    printf("%p,\t%d\n", a, *a);

    char *data = (char *)guest_flat_to_host(kvm, 0x80000000);
    printf("data : 0x%p: %s\n", data, data);
    //pthread_create(&kvm->cpus[0]->thread, NULL, kvm_mini_thread, kvm->cpus[0]);

    //pthread_join(kvm->cpus[0]->thread, NULL);

    return ret;
}

static int kvm_write_mini(struct kvm *kvm)
{
    struct kvm_cpu *cur = kvm->cpus[0];
    struct mini_data data;

    data = (struct mini_data) {
        .gpa = 0x80200000,
        .data = {'z','x','c','v'},
        .len = sizeof(data.data),
    };

    printf("kvm_write_mini\n");

    printf("0x%llx %s %d\n", data.gpa, (char *)data.data, data.len);

    ioctl(cur->vcpu_fd, KVM_WRITE_MINI, &data);
    return 0;
}

//static int kvm_read_mini(struct kvm *kvm, void *addr, void *data, int length)
static int kvm_read_mini(struct kvm *kvm)
{
    struct kvm_cpu *cur = kvm->cpus[0];
    struct mini_data data;

    printf("kvm_read_mini\n");

    data = (struct mini_data) {
        .gpa = 0x80200000,
        .data = {0, },
        .len = sizeof(data.data),
    };

    printf("before : 0x%llx: %s\n", data.gpa, (char *)data.data);
    
    ioctl(cur->vcpu_fd, KVM_READ_MINI, &data);

    printf("after : 0x%llx: %s\n", data.gpa, (char *)data.data);

    int i = 1;
    char dat = (char) data.data[0];
    while (dat != 0) { 
        printf("data : %c\n", dat);
        dat = data.data[i++];
    }
    return 0;
}

int kvm_cmd_mini(int argc, const char **argv, const char *prefix)
{
    int r = 0;

    static struct kvm *kvm;

    if(kvm == NULL) {
        kvm = kvm_cmd_struct_init(argc, argv);
        if (IS_ERR(kvm))
            return PTR_ERR(kvm);

        printf("kvm create complete\n");
        printf("name : %s\n", kvm->cfg.guest_name);
        printf("size : %lld\n", kvm->cfg.ram_size);
        
        //unsigned int *ram_start = (unsigned int*)kvm->ram_start;
        //printf("ramstart : %p : 0x%x\n", ram_start, *ram_start);
    }

    kvm_load_mini(kvm);

    //asm volatile(".word 0x6A10C073\n");   

    kvm_write_mini(kvm);
    kvm_read_mini(kvm);

    int i = 1;
    char *data = (char *)guest_flat_to_host(kvm, 0x80200000);
    printf("data : 0x%p: %s\n", data, data);
    while (*data != 0) { 
        printf("data : 0x%p: %c\n", data, *data);
        data = (char *)guest_flat_to_host(kvm, 0x80200000 + (i++) * sizeof(__u32));
    }

    /*
    static struct mini *mini;
    if(mini == NULL) {
        mini = kvm_cmd_mini_init(argc, argv);
        if (IS_ERR(mini))
            return PTR_ERR(mini);

        printf("mini create complete\n");
        printf("name : %s\n", mini->cfg.guest_name);
        printf("size : %lld\n", mini->cfg.ram_size);
    }
    */



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
