#include "mini/mini.h"

#include <linux/err.h>

#include <unistd.h>

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
