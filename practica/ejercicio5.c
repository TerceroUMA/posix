//gcc -o main ejercicio5.c -pthread -lpthread -lrt && sudo taskset -c 0 ./main && rm ./main

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

#define PRIORIDAD_BATERIA 4
#define PRIORIDAD_TELEFONO 3
#define PRIORIDAD_EVENTOS 2
#define PRIORIDAD_MONITORIZACION 1

void* monitorizacion_func() {

  printf("Hola monitorizacion_func\n");

  return NULL;
}

void* bateria_func() {

  printf("Hola bateria_func\n");

  return NULL;
}

void* event_func() {

  printf("Hola event_func\n");

  return NULL;
}

void* telefono_func() {

  printf("Hola telefono_func\n");

  return NULL;
}

int main() {

  mlockall(MCL_CURRENT || MCL_FUTURE);

  srand(time(0));
  int random = rand() % 100 + 1;

  int policy;
  pthread_t hebraMain = pthread_self();
  struct sched_param param;
  pthread_getschedparam(hebraMain, &policy, &param);
  param.sched_priority = 100;
  pthread_setschedparam(hebraMain, policy, &param);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);

  pthread_t bateria, eventos, telefono, monitorizacion;
  param.sched_priority = PRIORIDAD_MONITORIZACION;
  pthread_create(&bateria, &attr, bateria_func, NULL);
  param.sched_priority = PRIORIDAD_EVENTOS;
  pthread_create(&eventos, &attr, event_func, NULL);
  param.sched_priority = PRIORIDAD_TELEFONO;
  pthread_create(&telefono, &attr, telefono_func, NULL);
  param.sched_priority = PRIORIDAD_BATERIA;
  pthread_create(&monitorizacion, &attr, monitorizacion_func, NULL);

  pthread_join(monitorizacion , NULL);
  pthread_join(eventos, NULL);
  pthread_join(telefono, NULL);
  pthread_join(bateria, NULL);
}