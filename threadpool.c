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
typedef struct _function_st {
    void (*task) (void*); // The function to be executed
    void *arg;
    struct _function_st* next;
} _function_t;

// _threadpool is the internal threadpool structure that is
// cast to type "threadpool" before it given out to callers
typedef struct _threadpool_st {
    int max_threads;
    int min_threads;
    int thread_count;
    pthread_t *threads;
    pthread_mutex_t lock;
    int function_count;
    _function_t task_head;
    _function_t task_tail;
} _threadpool;


void *worker_thread(void *args) {
    while (1) {
        // wait for a signal
        // l
        // mark itself as busy
        // run a given function
        //
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

  // add your code here to initialize the newly created threadpool

  return (threadpool) pool;
}


void dispatch(threadpool from_me, dispatch_fn dispatch_to_here,
	      void *arg) {
  _threadpool *pool = (_threadpool *) from_me;

  // add your code here to dispatch a thread
}

void destroy_threadpool(threadpool destroyme) {
  _threadpool *pool = (_threadpool *) destroyme;

  // add your code here to kill a threadpool
}
