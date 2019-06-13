/**
 * threadpool.c
 *
 * This file will contain your implementation of a threadpool.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "threadpool.h"

// A queue of functions to be executed
typedef struct task_st {
    void (*function) (void*); // The function to be executed
    void *arg;
    struct task_st* next;
} task_t;

// _threadpool is the internal threadpool structure that is
// cast to type "threadpool" before it given out to callers
typedef struct _threadpool_st {
    pthread_t *threads;
    pthread_mutex_t lock;
    pthread_cond_t occupied;
    pthread_cond_t empty;
    int task_count;
    task_t* task_head;
    task_t* task_tail;
} _threadpool;


void *worker_thread(void *args) {

    _threadpool *pool = (_threadpool *) args;

    while (1) {
        pthread_mutex_lock(&(pool->lock));

        while(pool->task_count == 0){
            pthread_mutex_unlock(&(pool->lock));
            pthread_cond_wait(&(pool->occupied), &(pool->lock));
        }

        task_t *current_task = pool->task_head;
        pool->task_count--;

        if(pool->task_count == 0){
            pool->task_head = NULL;
            pool->task_tail = NULL;
        }else pool->task_head = current_task->next;

        if(pool->task_count == 0){
            pthread_cond_signal(&(pool->empty));
        }

        pthread_mutex_unlock(&(pool->lock));
        (current_task->function) (current_task->arg);
        free(current_task);
    }
}


threadpool create_threadpool(int num_threads_in_pool) {
    _threadpool *pool;

    // sanity check the argument
    if ((num_threads_in_pool <= 0) || (num_threads_in_pool > MAXT_IN_POOL))
        return NULL;

    pool = (_threadpool *) malloc(sizeof(_threadpool));
    if (pool == NULL) {
        fprintf(stderr, "Out of memory creating a new threadpool!\n");
        return NULL;
    }

    // allocate thread pool with num_threads_in_pool
    pool->threads = (pthread_t *) malloc(sizeof(pthread_t) * num_threads_in_pool);

    if (!(pool->threads)) return NULL;

    pool->task_head = NULL;
    pool->task_tail = NULL;
    pthread_mutex_init(&(pool->lock), NULL);
    pthread_cond_init(&(pool->occupied), NULL);
    pthread_cond_init(&(pool->empty), NULL);

    for(int i = 0; i < num_threads_in_pool; i++){
        int error = pthread_create(&(pool->threads[i]), NULL, worker_thread, pool);
        if (error) return NULL; // Thread couldn't be created
    }

    return (threadpool) pool;
}


void dispatch(threadpool from_me, dispatch_fn dispatch_to_here,
	      void *arg) {
    _threadpool *pool = (_threadpool *) from_me;
    task_t *current_task;

    current_task = (task_t*) malloc(sizeof(task_t));

    if (current_task == NULL) return; // Unable to create task struct

    current_task->function = dispatch_to_here;
    current_task->arg = arg;
    current_task->next = NULL;

    pthread_mutex_lock(&(pool->lock));

    if (pool->task_count == 0){
        pool->task_head = current_task;
        pool->task_tail = current_task;
        pthread_cond_signal(&(pool->occupied));
    }else{
        pool->task_tail->next = current_task;
        pool->task_tail = current_task;
    }
    pool->task_count++;

    pthread_mutex_unlock(&(pool->lock));
}

void destroy_threadpool(threadpool destroyme) {
    _threadpool *pool = (_threadpool *) destroyme;

    pthread_mutex_destroy(&(pool->lock));
    pthread_cond_destroy(&(pool->empty));
    pthread_cond_destroy(&(pool->occupied));
    free(pool->threads);
    free(pool);
}
