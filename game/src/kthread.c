#include "kthread.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>

#include "kutil.h"

extern int kthread_start(struct kthread *kthread, const char *sem_name,
        void *(*render_fn)(void *)) {
    kthread->sem = sem_open(sem_name, O_CREAT);
    if (kthread->sem == SEM_FAILED) {
        kprint("Error opening semaphore %s", strerror(errno));
        return -1;
    }
    int r;
    r = pthread_mutex_init(&kthread->mutex, NULL);
    if (r != 0) {
        kprint("Error opening mutex %s", strerror(r));
        sem_close(kthread->sem);
        return -1;
    }
    r = pthread_create(&kthread->pthread, NULL, render_fn, kthread);
    if (r != 0) {
        sem_close(kthread->sem);
        pthread_mutex_destroy(&kthread->mutex);
        kprint("Error creating pthread: %s", strerror(r));
        return -1;
    }
    return 0;
}

extern void kthread_end(struct kthread *kthread) {
    kthread->terminated = true;
    void *value_ptr;
    int r = pthread_join(kthread->pthread, &value_ptr);
    if (r != 0) {
        kprint("Error joining  pthread: %s", strerror(r));
    }
}

extern void kthread_lock(struct kthread *kthread) {
    int r = pthread_mutex_lock(&kthread->mutex);
    if (r != 0) {
        kprint("Error locking mutex: %s", strerror(r));
    }
}

extern void kthread_unlock(struct kthread *kthread) {
    int r = pthread_mutex_unlock(&kthread->mutex);
    if (r != 0) {
        kprint("Error unlocking mutex: %s", strerror(r));
    }
}

extern void kthread_wait_for_init(struct kthread *kthread) {
    while (!kthread->initialized) {
        if (sem_wait(kthread->sem) != 0) {
            kprint("Error waiting for init: %s", strerror(errno));
            return;
        }
    }
}

extern void kthread_signal_init(struct kthread *kthread) {
    kthread->initialized = true;
    if (sem_post(kthread->sem) != 0) {
        kprint("Error signalling init: %s", strerror(errno));
        return;
    }
}
