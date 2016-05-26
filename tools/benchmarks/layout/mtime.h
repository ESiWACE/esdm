#include <time.h>


typedef struct timespec timer;

static void start_timer(timer * t1){
  clock_gettime(CLOCK_MONOTONIC, t1);
}

static timer time_diff (struct timespec end, struct timespec start)
{
    struct timespec diff;
    if (end.tv_nsec < start.tv_nsec)
    {
        diff.tv_sec = end.tv_sec - start.tv_sec - 1;
        diff.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        diff.tv_sec = end.tv_sec - start.tv_sec;
        diff.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return diff;
}

static double time_to_double (struct timespec t)
{
    double d = (double)t.tv_nsec;
    d /= 1000000000.0;
    d += (double)t.tv_sec;
    return d;
}


static double stop_timer(timer t1){
  timer end;
  start_timer(& end);
  return time_to_double(time_diff(end, t1));
}

void repeat(void(*func)(void)){
  timer t1;
  double t = 0;
  start_timer(& t1);

  int iter = 0;

  while(t < 10.0){
    iter ++;
    func();
    t = stop_timer(t1);
  }

  printf("%f (iters: %d)\n", t / iter, iter);
}
