#ifndef __LINUX_MINI_H
#define __LINUX_MINI_H

#include <linux/const.h>
#include <linux/types.h>
#include <linux/compiler.h>
#include <linux/ioctl.h>

#define MINI_API_VERSION 1

#define MINI_MEM_READONLY	(1UL << 1)

#define MINIIO  0xF3

#define MINI_GET_API_VERSION       _IO(MINIIO,   0x00)
#define MINI_CREATE_VM      _IO(MINIIO, 0x01)
#define MINI_DEST_VM        _IO(MINIIO, 0x02)

#define MINI_GET_VCPU_MMAP_SIZE    _IO(MINIIO,   0x04) /* in bytes */

#define MINI_ALLOC          _IO(MINIIO, 0x05)
#define MINI_FREE           _IO(MINIIO, 0x06)

#define MINI_ENTER          _IO(MINIIO, 0x09)
#define MINI_EXIT           _IO(MINIIO, 0x0a)

#define MINI_CREATE_VCPU           _IO(MINIIO,   0x41)

#define MINI_SET_USER_MEMORY_REGION _IOW(MINIIO, 0x46, \
					struct mini_userspace_memory_region)

struct mini_userspace_memory_region {
	__u32 slot;
	__u32 flags;
	__u64 guest_phys_addr;
	__u64 memory_size; /* bytes */
	__u64 userspace_addr; /* start of the userspace allocated memory */
};

#endif
