#include <assert.h>
#include <glib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>

#include "util/test_util.h"

/*
 * The purpose of this program is to benchmark various options for thread creation and producer/consumer queues
 * The consumer may stall the producer when the queue becomes unbearable.
 */
GThreadPool **pools;
int backend_count;

void backend_thread(gpointer data_p, gpointer backend_id);

/*
 * Each backend may have its own number of threads it works best
 */
void register_backends(int backend_count, int threads_per_backend) {
  pools = (GThreadPool **)malloc(sizeof(GThreadPool *) * backend_count);
  GError *error;
  for (int i = 0; i < backend_count; i++) {
    pools[i] = g_thread_pool_new(backend_thread, (gpointer)(size_t)i, threads_per_backend, 1, &error);
  }
}

void unregister_backends(int backend_count) {
  for (int i = 0; i < backend_count; i++) {
    g_thread_pool_free(pools[i], 0, 1);
  }
}

typedef struct {
  _Atomic(size_t) pending_ops;
  _Atomic(int) waiting;

  //pthread_spinlock_t spinlock;  // protect access to pending_ops first
  GMutex mutex;
  GCond done_condition;
} io_request_status_t;

typedef struct {
  int num;

  io_request_status_t *parent;
} io_work_t;

void backend_thread(gpointer data_p, gpointer backend_id) {
  io_work_t *data = (io_work_t *)data_p;
  //printf("Thread: %zd processes: %zd \n", (size_t) backend_id, (size_t) data->num);
  // cleanup:
  io_request_status_t *status = data->parent;

  //pthread_spin_lock(& status->spinlock);
  size_t pending = atomic_fetch_sub(&status->pending_ops, 1);
  //printf("%d\n", status->pending_ops);
  assert(status->pending_ops >= 0);
  if (status->pending_ops == 0) {
    // mutex is very costly, therefore, wrap it behind atomics
    g_mutex_lock(&status->mutex);
    int waiting = atomic_fetch_add(&status->waiting, 0);
    if (waiting) {
      g_cond_signal(&status->done_condition);
    }
    g_mutex_unlock(&status->mutex);
  }
  //pthread_spin_unlock(& status->spinlock);
  free(data);
}

void startIO(long work, io_request_status_t *status) {
  // for now each IO starts one fragment per backend
  // add the number of outstanding requests
  GError *error;

  // now enqueue the operations
  for (int i = 0; i < backend_count; i++) {
    io_work_t *task = (io_work_t *)malloc(sizeof(io_work_t));
    task->num = i;
    task->parent = status;
    g_thread_pool_push(pools[i], task, &error);
  }
}

/*
 * Until all jobs are done
 */
void wait(io_request_status_t *status) {
  g_mutex_lock(&status->mutex);
  size_t now = atomic_fetch_sub(&status->pending_ops, 0);
  if (now > 0) {
    atomic_fetch_add(&status->waiting, 1);
    g_cond_wait(&status->done_condition, &status->mutex);
  }
  g_mutex_unlock(&status->mutex);
}

void write_dataset(long iterations) {
  // this is a blocking I/O
  io_request_status_t status;

  g_mutex_init(&status.mutex);
  g_cond_init(&status.done_condition);
  //pthread_spin_init(& status.spinlock, 0);

  // atomically set the number of ops that need to be done.
  status.pending_ops = ATOMIC_VAR_INIT(backend_count * iterations);
  status.waiting = ATOMIC_VAR_INIT(0);

  // TODO evtl. check if we must stall due to short in memory

  // breakdown the dataset into iteration fragments:
  for (long i = 0; i < iterations; i++) {
    startIO(i, &status);
  }
  wait(&status);
  //pthread_spin_destroy( & status.spinlock );
  g_cond_clear(&status.done_condition);
  g_mutex_clear(&status.mutex);
}

int main(int argc, char **argv) {
  if (argc != 4) {
    printf("Syntax: %s [backends#] [threads per backend#] [Iterations#]\n", argv[0]);
    exit(1);
  }
  backend_count = atoi(argv[1]);
  int threads_per_backend = atoi(argv[2]);
  long iterations = atol(argv[3]);
  register_backends(backend_count, threads_per_backend);

  timer t;
  start_timer(&t);
  write_dataset(iterations);
  double delta = stop_timer(t);
  printf("Runtime: %f Speed: %f iter/s -- With 4KByte Blocks per iter: %.1f MiB/s\n", delta, delta / iterations, 4096.0 / (delta / iterations) / 1024 / 1024);

  unregister_backends(backend_count);

  printf("[OK]\n");
}
