#ifndef KTHREAD_H
#define KTHREAD_H

#include <stdbool.h>

#include <pthread.h>
#include <semaphore.h>

struct kthread {
    void *arg;
    bool terminated;
    bool initialized;
    pthread_mutex_t mutex;
    sem_t *sem;

    pthread_t pthread;
};

extern int kthread_start(struct kthread *kthread, const char *sem_name,
        void *(*render_fn)(void *));
extern void kthread_end(struct kthread *kthread);

extern void kthread_lock(struct kthread *kthread);
extern void kthread_unlock(struct kthread *kthread);

extern void kthread_wait_for_init(struct kthread *kthread);
extern void kthread_signal_init(struct kthread *kthread);

#endif
