#include "libsa.h"

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <immintrin.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/mman.h>
#include <sys/syscall.h>

#include <asm/prctl.h>

struct trampoline_args {
    void *(*fn)(void *);
    void *fn_arg;
    void *stack_base;
    size_t stack_size;
};

typedef pid_t (*real_fork_t)(void);
typedef int (*real_pthread_create_t)(pthread_t *, const pthread_attr_t *, void *(*)(void *),
                                     void *);

static real_fork_t real_fork;
static real_pthread_create_t real_pthread_create;

static pthread_key_t cleanup_key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;
static int shadow_fd = -1;

static void thread_cleanup(void *stack_base) {
    if (!stack_base) {
        return;
    }

    sa_free_kernel_stack(stack_base);
    sa_free_user_stack(stack_base, STACK_SIZE);
}

static inline void make_cleanup_key(void) {
    pthread_key_create(&cleanup_key, thread_cleanup);
}

static __attribute__((constructor)) void main_thread_init(void) {
    if (sa_init(STACK_START + STACK_SIZE, STACK_END)) {
        exit(EXIT_FAILURE);
    }

    shadow_fd = open("/dev/shadowstack", O_RDWR | O_CLOEXEC);
    if (shadow_fd < 0) {
        perror("cannot open shadowstack device");
        exit(EXIT_FAILURE);
    }

    void *sym = dlsym(RTLD_NEXT, "fork");
    memcpy((void *)&real_fork, (void *)&sym, sizeof(sym));
    sym = dlsym(RTLD_NEXT, "pthread_create");
    memcpy((void *)&real_pthread_create, (void *)&sym, sizeof(sym));
    pthread_once(&key_once, make_cleanup_key);
    // lo stack viene spostato tramite una procedura assembly per il momento
    // quindi serve un modo per risalire all'indirizzo
}

static void *thread_trampoline(void *arg) {
    struct trampoline_args t = *(struct trampoline_args *)arg;
    free(arg);

    sa_alloc(t.stack_base);
    void *ret = t.fn(t.fn_arg);
    sa_free_kernel_stack(t.stack_base);
    return ret;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *user_attr,
                   void *(*start_routine)(void *), void *arg) {
    pthread_once(&key_once, make_cleanup_key);

    pthread_attr_t attr;
    if (user_attr) {
        attr = *user_attr;
    } else {
        pthread_attr_init(&attr);
    }

    void *stack_base = sa_alloc_stack();
    if (!stack_base) {
        if (!user_attr) {
            pthread_attr_destroy(&attr);
        }

        return EAGAIN;
    }

    if (pthread_attr_setstack(&attr, stack_base, STACK_SIZE) != 0) {
        sa_free_user_stack(stack_base, STACK_SIZE);
        if (!user_attr) {
            pthread_attr_destroy(&attr);
        }

        return EINVAL;
    }

    struct trampoline_args *targs = malloc(sizeof(*targs));
    if (!targs) {
        sa_free_user_stack(stack_base, STACK_SIZE);
        if (!user_attr) {
            pthread_attr_destroy(&attr);
        }

        return ENOMEM;
    }

    targs->fn = start_routine;
    targs->fn_arg = arg;
    targs->stack_base = stack_base;
    targs->stack_size = STACK_SIZE;

    int err = real_pthread_create(thread, &attr, thread_trampoline, targs);
    if (!user_attr) {
        pthread_attr_destroy(&attr);
    }

    if (err != 0) {
        free(targs);
        munmap(stack_base, STACK_SIZE);
    }

    return err;
}

pid_t fork(void) {
    int sy_pipe[2];
    if (pipe2(sy_pipe, O_CLOEXEC) < 0) {
        return -1;
    }

    pid_t p_tgid = getpid();
    pid_t p_pid = (pid_t)syscall(SYS_gettid);
    pid_t pid = real_fork();
    if (!pid) {
        close(sy_pipe[0]);

        struct ioctl_fork_params req = {
            .error = 0,
            .p_pid = p_pid,
            .p_tgid = p_tgid,
        };

        if (shadow_fd >= 0) {
            close(shadow_fd);
        }

        shadow_fd = open("/dev/shadowstack", O_RDWR);
        ioctl(shadow_fd, IOCTL_SHADOW_FORK, &req);

        char done = '1';
        if (write(sy_pipe[1], &done, 1) != 1) {
        }
        close(sy_pipe[1]);
    } else if (pid > 0) {
        close(sy_pipe[1]);

        char done;
        if (read(sy_pipe[0], &done, 1) != 1) {
        }
        close(sy_pipe[0]);
    } else {
        close(sy_pipe[0]);
        close(sy_pipe[1]);
    }

    return pid;
}
