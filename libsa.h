#ifndef SS_RUNTIME_H
#define SS_RUNTIME_H
#include "ioctl.h"

#include <fcntl.h>
#include <stdint.h>

#include <sys/ioctl.h>

void *freelist_pop(void);

void freelist_push(void *stack);

long sa_alloc_shadow_stack(void *stack_base);

void *sa_alloc_user_stack(void);

long sa_init(unsigned long start, unsigned long end, unsigned long size);

long sa_claim(unsigned long stack_base);

long sa_free_kernel_stack(void *stack_base);

void sa_free_user_stack(void *stack_base);

long sa_mark_detached(void *stack_base);

long sa_reclaim(void *stack_base);

void sa_reset_after_fork(void *current_stack);
#endif
