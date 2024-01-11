
#ifndef MINI__MINI_CPU_H
#define MINI__MINI_CPU_H

#include "mini/mini-cpu-arch.h"
#include <stdbool.h>

struct mini_cpu_task {
	void (*func)(struct mini_cpu *vcpu, void *data);
	void *data;
};

int mini_cpu__init(struct mini *mini);
struct mini_cpu *mini_cpu__arch_init(struct mini *mini, unsigned long cpu_id);
/*
int mini_cpu__exit(struct mini *mini);
void mini_cpu__delete(struct mini_cpu *vcpu);
void mini_cpu__reset_vcpu(struct mini_cpu *vcpu);
void mini_cpu__setup_cpuid(struct mini_cpu *vcpu);
void mini_cpu__enable_singlestep(struct mini_cpu *vcpu);
void mini_cpu__run(struct mini_cpu *vcpu);
int mini_cpu__start(struct mini_cpu *cpu);
bool mini_cpu__handle_exit(struct mini_cpu *vcpu);
int mini_cpu__get_endianness(struct mini_cpu *vcpu);

int mini_cpu__get_debug_fd(void);
void mini_cpu__set_debug_fd(int fd);
void mini_cpu__show_code(struct mini_cpu *vcpu);
void mini_cpu__show_registers(struct mini_cpu *vcpu);
void mini_cpu__show_page_tables(struct mini_cpu *vcpu);
void mini_cpu__arch_nmi(struct mini_cpu *cpu);
void mini_cpu__run_on_all_cpus(struct mini *mini, struct mini_cpu_task *task);
*/

#endif /* MINI__MINI_CPU_H */
