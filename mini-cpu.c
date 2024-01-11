//#include "kvm/util.h"

#include "mini/mini-cpu.h"
#include "mini/mini.h"

#include <sys/ioctl.h>
#include <sys/eventfd.h>

static int task_eventfd;

int mini_cpu__init(struct mini *mini)
{
	int max_cpus, recommended_cpus, i;

	//max_cpus = mini__max_cpus(mini);
    max_cpus = 1024;
	//recommended_cpus = mini__recommended_cpus(mini);
    recommended_cpus = 4;

    pr_debug("[lkvm] mini_cpu__init");

	if (mini->cfg.nrcpus > max_cpus) {
		printf("  # Limit the number of CPUs to %d\n", max_cpus);
		mini->cfg.nrcpus = max_cpus;
	} else if (mini->cfg.nrcpus > recommended_cpus) {
		printf("  # Warning: The maximum recommended amount of VCPUs"
			" is %d\n", recommended_cpus);
	}

	mini->nrcpus = mini->cfg.nrcpus;

	task_eventfd = eventfd(0, 0);
	if (task_eventfd < 0) {
		pr_warning("Couldn't create task_eventfd");
		return task_eventfd;
	}

	/* Alloc one pointer too many, so array ends up 0-terminated */
	mini->cpus = calloc(mini->nrcpus + 1, sizeof(void *));
	if (!mini->cpus) {
		pr_warning("Couldn't allocate array for %d CPUs", mini->nrcpus);
		return -ENOMEM;
	}

	for (i = 0; i < mini->nrcpus; i++) {
		mini->cpus[i] = mini_cpu__arch_init(mini, i);
		if (!mini->cpus[i]) {
			pr_warning("unable to initialize MINI VCPU");
			goto fail_alloc;
		}
	}

	return 0;

fail_alloc:
	for (i = 0; i < mini->nrcpus; i++)
		free(mini->cpus[i]);
	return -ENOMEM;
}
//base_init(mini_cpu__init);
