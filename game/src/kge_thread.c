#include "kge_thread.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>

#include "kge_util.h"

extern int kge_thread_start(struct kge_thread *thread, const char *sem_name,
        void *(*render_fn)(void *)) {
    thread->sem = sem_open(sem_name, O_CREAT);
    if (thread->sem == SEM_FAILED) {
        kprint("Error opening semaphore %s", strerror(errno));
        return -1;
    }
    int r;
    r = pthread_mutex_init(&thread->mutex, NULL);
    if (r != 0) {
        kprint("Error opening mutex %s", strerror(r));
        sem_close(thread->sem);
        return -1;
    }
    r = pthread_create(&thread->pthread, NULL, render_fn, thread);
    if (r != 0) {
        sem_close(thread->sem);
        pthread_mutex_destroy(&thread->mutex);
        kprint("Error creating pthread: %s", strerror(r));
        return -1;
    }
    return 0;
}

extern void kge_thread_end(struct kge_thread *thread) {
    thread->terminated = true;
    void *value_ptr;
    int r = pthread_join(thread->pthread, &value_ptr);
    if (r != 0) {
        kprint("Error joining  pthread: %s", strerror(r));
    }
}

extern void kge_thread_lock(struct kge_thread *thread) {
    int r = pthread_mutex_lock(&thread->mutex);
    if (r != 0) {
        kprint("Error locking mutex: %s", strerror(r));
    }
}

extern void kge_thread_unlock(struct kge_thread *thread) {
    int r = pthread_mutex_unlock(&thread->mutex);
    if (r != 0) {
        kprint("Error unlocking mutex: %s", strerror(r));
    }
}

extern void kge_thread_wait_for_init(struct kge_thread *thread) {
    while (!thread->initialized) {
        if (sem_wait(thread->sem) != 0) {
            kprint("Error waiting for init: %s", strerror(errno));
            return;
        }
    }
}

extern void kge_thread_signal_init(struct kge_thread *thread) {
    thread->initialized = true;
    if (sem_post(thread->sem) != 0) {
        kprint("Error signalling init: %s", strerror(errno));
        return;
    }
}
