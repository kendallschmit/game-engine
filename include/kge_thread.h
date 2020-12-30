#ifndef KGE_THREAD_H
#define KGE_THREAD_H

#include <stdbool.h>

#include <pthread.h>
#include <semaphore.h>

struct kge_thread {
    void *arg;
    bool terminated;
    bool initialized;
    pthread_mutex_t mutex;
    sem_t *sem;

    pthread_t pthread;
};

int kge_thread_start(struct kge_thread *thread, const char *sem_name,
        void *(*fn)(void *));
void kge_thread_end(struct kge_thread *thread);

void kge_thread_lock(struct kge_thread *thread);
void kge_thread_unlock(struct kge_thread *thread);

void kge_thread_wait_for_init(struct kge_thread *thread);
void kge_thread_signal_init(struct kge_thread *thread);

#endif
