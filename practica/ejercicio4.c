//gcc -o main ejercicio4.c -pthread -lpthread -lrt && sudo taskset -c 0 ./main && rm ./main

#include <stdlib.h>
#include <stddef.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sched.h>
#include <pthread.h>
#include <sys/mman.h>
#include <signal.h>

pthread_mutex_t mutex;
int contador = 0;
sigset_t seniales;

void* tareaA() {

  return  NULL;
}

void* tareaB() {

  return  NULL;
}

int main() {

  mlockall(MCL_CURRENT | MCL_FUTURE);

  pthread_mutex_init(&mutex, NULL);

  sigemptyset(&seniales);
  sigaddset(&seniales, SIGUSR1);
  sigaddset(&seniales, SIGUSR2);
  pthread_sigmask(SIG_BLOCK, &seniales, NULL);

  int policy = 0;
  pthread_t mainHebra = pthread_self();
  struct sched_param param;
  pthread_getschedparam(mainHebra, &policy, &param);
  param.sched_priority = 100;
  pthread_setschedparam(mainHebra, policy, &param);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

  pthread_t tareaHebraA, tareaHebraB;
  param.sched_priority = 24;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tareaHebraA, &attr, tareaA, NULL);
  param.sched_priority = 24;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tareaHebraA, &attr, tareaB, NULL);

  pthread_join(tareaHebraA, NULL);
  pthread_join(tareaHebraB, NULL);

  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex);
}