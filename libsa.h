#ifndef SS_RUNTIME_H
#define SS_RUNTIME_H
#include <fcntl.h>
#include <stdint.h>

#include <sys/ioctl.h>

#define IOCTL_SHADOW_MAGIC 0xca
#define IOCTL_SHADOW_REQ   _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_REQ_NO, struct ioctl_mem_params)
#define IOCTL_SHADOW_FREE  _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_FREE_NO, struct ioctl_mem_params)
#define IOCTL_SHADOW_FORK  _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_FORK_NO, struct ioctl_fork_params)
#define STACK_START        (0x100000000000UL)
#define STACK_END          (0x200000000000UL)
#define STACK_SIZE         (8UL * 1024 * 1024)

enum {
    IOCTL_SHADOW_REQ_NO = 0x70,
    IOCTL_SHADOW_FREE_NO,
    IOCTL_SHADOW_FORK_NO,
    IOCTL_SHADOW_MAX_NR
};

struct ioctl_mem_params {
    long error;
    unsigned long addr;
};

struct ioctl_fork_params {
    long error;
    int p_tgid;
    int p_pid;
};

long sa_alloc(void *stack_base);

void *sa_alloc_stack(void);

long sa_init(unsigned long start, unsigned long end);

long sa_free_kernel_stack(void *stack_base);

long sa_free_user_stack(void *stack_base, unsigned long stack_size);

#endif
