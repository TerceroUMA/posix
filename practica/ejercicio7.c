//gcc -o main ejercicio7.c -pthread -lpthread -lrt && sudo taskset -c 0 ./main && rm ./main

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

pthread_mutex_t mutex1;
pthread_mutex_t mutex2;
pthread_mutex_t mutex3;

int contador1 = 0;
int contador2 = 0;

void espera_activa(time_t seg) {

  time_t tiempo = time(0) + seg;
  while(time(0) < tiempo);

}

void* tarea_a_func() {

  pthread_mutex_lock(&mutex1);
  contador1++;
  printf("A -> Contador 1: %d\n", contador1);
  pthread_mutex_unlock(&mutex1);

  return NULL;
}

void* tarea_m_func() {

  sleep(2);

  pthread_mutex_lock(&mutex2);
  pthread_mutex_lock(&mutex3);
  contador2++;
  printf("M -> Contador 2: %d\n", contador2);
  pthread_mutex_unlock(&mutex3);
  pthread_mutex_unlock(&mutex2);

  return NULL;
}

void* tarea_b_func() {

  sleep(1);

  pthread_mutex_lock(&mutex3);
  espera_activa(3);
  pthread_mutex_lock(&mutex2);
  contador2++;
  printf("B -> Contador 2: %d\n", contador2);
  pthread_mutex_unlock(&mutex2);
  pthread_mutex_unlock(&mutex3);

  return NULL;
}

int main() {

  mlockall(MCL_CURRENT | MCL_FUTURE);

  pthread_mutexattr_t attr_mutex;
  pthread_mutexattr_init(&attr_mutex);

  pthread_mutexattr_setprotocol(&attr_mutex, PTHREAD_PRIO_PROTECT);
  pthread_mutexattr_setprioceiling(&attr_mutex, 50);
  
  pthread_mutex_init(&mutex1, NULL);
  pthread_mutex_init(&mutex2, &attr_mutex);
  pthread_mutex_init(&mutex3, &attr_mutex);

  pthread_mutexattr_destroy(&attr_mutex);

  pthread_t tarea_a, tarea_m, tarea_b;

  pthread_t hebraMain = pthread_self();
  struct sched_param param;
  int policy;
  pthread_getschedparam(hebraMain, &policy, &param);
  param.sched_priority = 100;
  pthread_setschedparam(hebraMain, policy, &param);

  pthread_attr_t attr;
  pthread_attr_init(&attr);

  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

  param.sched_priority = 75;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tarea_a, &attr, tarea_a_func, NULL);
  param.sched_priority = 50;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tarea_m, &attr, tarea_m_func, NULL);
  param.sched_priority = 25;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tarea_b, &attr, tarea_b_func, NULL);

  pthread_join(tarea_a, NULL);
  pthread_join(tarea_m, NULL);
  pthread_join(tarea_b, NULL);

  pthread_attr_destroy(&attr);

  return 0;
}