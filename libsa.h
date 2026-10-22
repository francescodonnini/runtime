#ifndef SS_RUNTIME_H
#define SS_RUNTIME_H
#include <fcntl.h>
#include <stdint.h>

#include <sys/ioctl.h>

#define IOCTL_SHADOW_MAGIC 0xca

#define IOCTL_SHADOW_ALLOC                                                                         \
    _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_ALLOC_NO, struct ioctl_alloc_params)
#define IOCTL_SHADOW_FORK _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_FORK_NO, struct ioctl_fork_params)
#define IOCTL_SHADOW_FREE _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_FREE_NO, struct ioctl_free_params)
#define IOCTL_SHADOW_MARK_DETACHED                                                                 \
    _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_MARK_DETACHED_NO, struct ioctl_mark_detached)
#define IOCTL_SHADOW_PIVOT                                                                         \
    _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_PIVOT_NO, struct ioctl_pivot_params)
#define IOCTL_SHADOW_PREPARE                                                                       \
    _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_PREPARE_NO, struct ioctl_prepare_params)
#define IOCTL_SHADOW_RECLAIM                                                                       \
    _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_RECLAIM_NO, struct ioctl_reclaim_params)
#define IOCTL_SHADOW_RECLAIM_DEAD                                                                  \
    _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_RECLAIM_NO, struct ioctl_reclaim_dead_params)
#define IOCTL_SHADOW_UNPREPARE                                                                     \
    _IOWR(IOCTL_SHADOW_MAGIC, IOCTL_SHADOW_UNPREPARE_NO, struct ioctl_unprepare_params)

enum {
    IOCTL_SHADOW_ALLOC_NO = 0x70,
    IOCTL_SHADOW_FORK_NO,
    IOCTL_SHADOW_FREE_NO,
    IOCTL_SHADOW_MARK_DETACHED_NO,
    IOCTL_SHADOW_PIVOT_NO,
    IOCTL_SHADOW_PREPARE_NO,
    IOCTL_SHADOW_RECLAIM_NO,
    IOCTL_SHADOW_RECLAIM_DEAD_NO,
    IOCTL_SHADOW_UNPREPARE_NO,
    IOCTL_SHADOW_MAX_NR
};

struct ioctl_empty {
    long error;
};

struct ioctl_alloc_params {
    long error;
    unsigned long stack_base;
};

struct ioctl_fork_params {
    long error;
    unsigned long stack_base;
};

struct ioctl_pivot_params {
    long error;
    unsigned long new_stack_address;
    unsigned long rsp;
};

struct ioctl_prepare_params {
    long error;
    unsigned long stack_base;
};

struct ioctl_reclaim_params {
    long error;
    unsigned long stack_base;
};

struct ioctl_reclaim_dead_params {
    long error;
    unsigned long bases;
    unsigned cap;
    unsigned count;
};

struct ioctl_unprepare_params {
    long error;
    unsigned long stack_base;
};

void *freelist_pop(void);

void freelist_push(void *stack);

long sa_alloc_shadow_stack(void *stack_base);

void *sa_alloc_user_stack(void);

long sa_init(unsigned long start, unsigned long end, unsigned long size);

long sa_claim(unsigned long stack_base);

long sa_free_kernel_stack(void *stack_base);

long sa_free_user_stack(void *stack_base);

long sa_mark_detached(void *stack_base);

long sa_reclaim(void *stack_base);

void sa_reset_after_fork(void *current_stack);
#endif
