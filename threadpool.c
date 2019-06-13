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
typedef struct function_st {
    void (*task) (void*); // The function to be executed
    void *arg;
    struct function_st* next;
} function_t;

// _threadpool is the internal threadpool structure that is
// cast to type "threadpool" before it given out to callers
typedef struct _threadpool_st {
    int thread_count;
    pthread_t *threads;
    pthread_mutex_t lock;
    pthread_cond_t busy;
    pthread_cond_t empty;
    int function_count;
    function_t* task_head;
    function_t* task_tail;
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

  // allocate thread pool with num_threads_in_pool
  pool->threads = (pthread_t*) malloc(sizeof(pthread_t) * num_threads_in_pool);

  // todo: if(!pool->threads) out of memory blah blah

  pool->thread_count = num_threads_in_pool;
  pool->task_head = NULL;
  pool->task_tail = NULL;
  pthread_mutex_init(&pool->lock, NULL); // todo: (void) ???
  pthread_cond_init(&pool->busy, NULL);
  pthread_cond_init(&pool->empty, NULL);

  int i;
  for(i = 0; i < num_threads_in_pool; i++){
      int error = pthread_create(&(pool->threads[i]), NULL, worker_thread, pool);
      if (error) return NULL; // Thread couldn't be created
  }

  return (threadpool) pool;
}


void dispatch(threadpool from_me, dispatch_fn dispatch_to_here,
	      void *arg) {
  _threadpool *pool = (_threadpool *) from_me;


}

void destroy_threadpool(threadpool destroyme) {
  _threadpool *pool = (_threadpool *) destroyme;

  // add your code here to kill a threadpool
}
