#include "libsa.h"

#include <errno.h>
#include <stdatomic.h>
#include <stdio.h>

#include <sys/mman.h>

extern int shadow_fd;

static atomic_ulong next;

long sa_alloc(void *stack_base) {
    struct ioctl_mem_params req = {.addr = (unsigned long)stack_base};
    long err = ioctl(shadow_fd, IOCTL_SHADOW_REQ, &req);
    if (err < 0) {
        perror("ioctl");
        return err;
    }

    return req.error;
}

void *sa_alloc_stack(void) {
    unsigned long n = atomic_fetch_add(&next, STACK_SIZE);
    if (n >= STACK_END) {
        return NULL;
    }

    void *addr = (void *)n;
    int err = mprotect(addr, STACK_SIZE, PROT_READ | PROT_WRITE);
    if (err) {
        perror("mprotect failed");
        return NULL;
    }

    err = madvise(addr, STACK_SIZE, MADV_DONTNEED);
    if (err) {
        perror("madvise failed");
        return NULL;
    }

    return addr;
}

long sa_init(unsigned long start, unsigned long end) {
    if (!start || end >= start) {
        return -EINVAL;
    }

    if (mmap((void *)start, end - start, PROT_NONE,
             MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE | MAP_NORESERVE, -1,
             0) == MAP_FAILED) {
        perror("mmap failed");
        return errno;
    }

    atomic_store(&next, start);
    return 0;
}

long sa_free_kernel_stack(void *stack_base) {
    struct ioctl_mem_params req = {.addr = (unsigned long)stack_base};
    long err = ioctl(shadow_fd, IOCTL_SHADOW_FREE, &req);
    if (err) {
        perror("ioctl");
        return err;
    }

    return req.error;
}

long sa_free_user_stack(void *stack_base, unsigned long stack_size) {
    if (munmap(stack_base, stack_size)) {
        perror("munmap");
    }

    return errno;
}
