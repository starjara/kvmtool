#include "mini/mini.h"
#include <kvm/util.h>
#include <kvm/kvm-cmd.h>
#include <kvm/builtin-mini.h>
//#include "kvm/kvm.h"
#include "mini/mini-cpu.h"
#include "kvm/parse-options.h"
//#include "kvm/read-write.h"
//#include "kvm/builtin-debug.h"
//#include "kvm/util-init.h"

#include <linux/mini.h>

#include <linux/err.h>
#include <linux/list.h>

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>

<<<<<<< Updated upstream
#include <sys/eventfd.h>
=======
#define HLVX_HU(dest, addr)					\
	INSN_R(OPCODE_SYSTEM, FUNC3(4), FUNC7(50),		\
	       RD(dest), RS1(addr), __RS2(3))

#define HLV_W(dest, addr)					\
	INSN_R(OPCODE_SYSTEM, FUNC3(4), FUNC7(52),		\
	       RD(dest), RS1(addr), __RS2(0))
>>>>>>> Stashed changes

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

static void parse_kvm_options(struct mini *mini, int argc, const char **argv)
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

void kvm_mini_help(void)
{
	printf("\n%s mini creates a small size guest virtual address space.\n"
		, MINI_BINARY_NAME);
	usage_with_options(mini_usage, mini_options);
}

static int mini_struct_init (struct mini *mini)
{
    //mini->cfg.ram_addr = kvm__arch_default_ram_address();
    mini->cfg.ram_addr = 0;
    mini->cfg.dev = DEFAULT_MINI_DEV;
    mini->sys_fd = open(mini->cfg.dev, O_RDWR);
    mini->cfg.nrcpus = 1;

    pr_debug("[lkvm] mini_struct_init\n");

	mini->vm_fd = ioctl(mini->sys_fd, MINI_CREATE_VM, 0);
    
	if (mini->vm_fd < 0) {
		pr_err("MINI_CREATE_VM ioctl");
        return -1;
    }

    mini__arch_init(mini);
    
    INIT_LIST_HEAD(&mini->mem_banks);
    mini__init_ram(mini);

    return 0;
}

/*
static struct mini *kvm_cmd_mini_init (int argc, const char **argv) 
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
static struct mini *mini_cmd_struct_init (int argc, const char **argv) 
{
    struct mini *mini= mini__new();
    
    if (IS_ERR(mini))
            return mini;

    parse_kvm_options(mini, argc, argv);

    if(mini_struct_init(mini) != 0) {
        free(mini);
        return NULL;
    }

    if(mini_cpu__init(mini) != 0) {
        free(mini);
        return NULL;
    }

    return mini;
}

/*
static void kvm_mini__reset_vcpu(struct kvm_cpu *vcpu)
{
	struct kvm *kvm = vcpu->kvm;
	struct kvm_mp_state mp_state;
	struct kvm_one_reg reg;
	unsigned long data;

	if (ioctl(vcpu->vcpu_fd, KVM_GET_MP_STATE, &mp_state) < 0)
		die_perror("KVM_GET_MP_STATE failed");

	reg.addr = (unsigned long)&data;

	//data	= kvm->arch.kern_guest_start;
    //data = 0x0000000000013e6e;
	//reg.id	= RISCV_CORE_REG(regs.pc);
	//if (ioctl(vcpu->vcpu_fd, KVM_SET_ONE_REG, &reg) < 0)
	//	die_perror("KVM_SET_ONE_REG failed (pc)");

	data	= vcpu->cpu_id;
	reg.id	= RISCV_CORE_REG(regs.a0);
	if (ioctl(vcpu->vcpu_fd, KVM_SET_ONE_REG, &reg) < 0)
		die_perror("KVM_SET_ONE_REG failed (a0)");

	data	= kvm->arch.dtb_guest_start;
	reg.id	= RISCV_CORE_REG(regs.a1);
	if (ioctl(vcpu->vcpu_fd, KVM_SET_ONE_REG, &reg) < 0)
		die_perror("KVM_SET_ONE_REG failed (a1)");
}
*/

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

/*
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

static int kvm_fin_mini(struct kvm *kvm)
{
    int ret = 0;
    struct kvm_cpu *cur = kvm->cpus[0];

    ioctl(cur->vcpu_fd, KVM_FIN_MINI, 0);
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
*/

int kvm_cmd_mini(int argc, const char **argv, const char *prefix)
{
    int r = 0;

    static struct mini *mini;

    if(mini == NULL) {
        mini = mini_cmd_struct_init(argc, argv);
        if (IS_ERR(mini))
            return PTR_ERR(mini);

        printf("mini create complete\n");
        printf("name : %s\n", mini->cfg.guest_name);
        printf("size : %lld\n", mini->cfg.ram_size);
        
        //unsigned int *ram_start = (unsigned int*)mini->ram_start;
        //printf("ramstart : %p : 0x%x\n", ram_start, *ram_start);
    }

    struct mini_cpu *cur = mini->cpus[0];

    printf("vm : 0x%x\n", mini->vm_fd);
    printf("vcpu : 0x%x\n", cur->vcpu_fd);
    ioctl(mini->vm_fd, MINI_ENTER, 0);
    ioctl(cur->vcpu_fd, MINI_ALLOC, 0);

    printf("Hello world\n");

    /*
    kvm_load_mini(kvm);

    //asm volatile(".word 0x6A10C073\n");   


    kvm_write_mini(kvm);
    kvm_read_mini(kvm);
    */

    /*
    int i = 1;
    char *data = (char *)guest_flat_to_host(kvm, 0x80200000);
    printf("data : 0x%p: %s\n", data, data);
    while (*data != 0) { 
        printf("data : 0x%p: %c\n", data, *data);
        data = (char *)guest_flat_to_host(kvm, 0x80200000 + (i++) * sizeof(__u32));
    }
    */

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

    /*
    unsigned long val, guest_addr;
    guest_addr = 0x80000000;
    pr_debug("val : 0x%lx\n", val);
    asm volatile(HLV_D(%[val], %[addr]) :[val] "=&r" (val): [addr] "r" (guest_addr) );
    pr_debug("val : 0x%lx\n", val);
    */

    /*
    // HLV_W
    clock_t c0 = clock();
    asm volatile(".word 0x68094F73\n");   
    clock_t c1 = clock();
    // HLV_D
    //asm volatile(".word 0x6C094F73\n");   

    printf("main done\n");
    printf("ms: %f %ld\n", (c1-c0) * 1000. / CLOCKS_PER_SEC, c1-c0);
    */


    //ioctl(kvm->cpus[0]->vcpu_fd, KVM_ENTER_MINI, 0);
    //ioctl(kvm->cpus[0]->vcpu_fd, KVM_EXIT_MINI, 0);
    
    /*
    int *addr = (int *)guest_flat_to_host(kvm, 0x80200000);
    int dest = 0;
    c0 = clock();
    dest = *addr;
    c1 = clock();
    printf("ms : %f %d\n", (c1-c0) * 1000. / CLOCKS_PER_SEC, dest);
    */
    

    /*
    struct kvm_one_reg reg;
    unsigned long satp = 0;

    reg.id = KVM_REG_RISCV_CSR_REG(satp);
    reg.addr = (unsigned long)&satp;

    ioctl(kvm->cpus[0]->vcpu_fd, KVM_GET_ONE_REG, &reg);
    pr_debug("reg : 0x%llx, addr : 0x%llx, val : 0x%lx\n", reg.id, reg.addr, satp);
    */

    register unsigned long taddr asm("a0") = 0x80000000;
    register unsigned long ttmp asm("a1");
    register unsigned long val asm("t0") = 0xFF;
    register unsigned long addr asm("t2") = 0x80000000;

    //printf("before 0x%lx 0x%lx\n", taddr, val);

    asm volatile ("\n"
        ".option push\n"
        ".option norvc\n"
        "add %[ttmp], %[taddr], 0\n"
        //
    //#ifdef CONFIG_64BIT
        /*
        * HLV.D %[val], (%[addr])
        * HLV.D t0, (t2)
        * 0110110 00000 00111 100 00101 1110011
        */
    //    ".word 0x6c03c2f3\n"
    //#else
        /*
        * HLV.W %[val], (%[addr])
        * HLV.W t0, (t2)
        * 0110100 00000 00111 100 00101 1110011
        */
        ".word 0x6A53C073\n" // HSV.W (t2), t0
        ".word 0x6803c2f3\n" // HLV.W t0, (t2)
       
    //#endif
    //
        ".option pop"
        : [val] "+r" (val),
        [taddr] "+&r" (taddr), [ttmp] "+&r" (ttmp)
        : [addr] "r" (addr) : "memory");

    printf("after taddr %lx val %lx\n", taddr, val);

    ioctl(mini->vm_fd, MINI_EXIT, 0);

    //asm volatile ("\n"
    //    ".option push\n"
    //    ".option norvc\n"
    //    "add %[ttmp], %[taddr], 0\n"
    //#ifdef CONFIG_64BIT
        /*
        * HLV.D %[val], (%[addr])
        * HLV.D t0, (t2)
        * 0110110 00000 00111 100 00101 1110011
        */
    //    ".word 0x6c03c2f3\n"
    //#else
        /*
        * HLV.W %[val], (%[addr])
        * HLV.W t0, (t2)
        * 0110100 00000 00111 100 00101 1110011
        */
    //    ".word 0x6803c2f3\n"
    //#endif
    //    ".option pop"
    //    : [val] "=&r" (val),
    //    [taddr] "+&r" (taddr), [ttmp] "+&r" (ttmp)
    //    : [addr] "r" (addr) : "memory");
    //printf("after %lx %lx\n", taddr, val);
    //ioctl(kvm->cpus[0]->vcpu_fd, KVM_EXIT_MINI, 0);

    return r;

}
