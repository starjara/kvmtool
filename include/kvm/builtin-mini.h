#ifndef KVM__BUILTIN_MINI_H
#define KVM__BUILTIN_MINI_H

#include <kvm/util.h>

int kvm_cmd_mini(int argc, const char **argv, const char *prefix);
void kvm_mini_help(void) NORETURN;
//int kvm_setup_create_new(const char *guestfs_name);
//void kvm_setup_resolv(const char *guestfs_name);

#endif
