//gcc -o main ej2.c -pthread -lpthread -lrt && sudo taskset -c 0 ./main && rm ./main

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>
#include <sys/mman.h>
#include <signal.h>

#define PRIORIDAD_HABITACION_1 3
#define PRIORIDAD_HABITACION_2 2
#define PRIORIDAD_CONTROLADOR 1

struct Habitacion1 {
  pthread_mutex_t mutex;
  int luz;
};

struct Habitacion2 {
  pthread_mutex_t mutex;
  int luz;
};

struct Data {
  struct Habitacion1 habitacion1;
  struct Habitacion2 habitacion2;
};

void* hab1_func(void* arg) {

  struct Habitacion1* datos = arg;

  struct timespec next;
  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1) {

    next.tv_sec += 2;
    next.tv_nsec += 0;

    if (next.tv_nsec >= 1000000000L) {
      next.tv_sec += next.tv_nsec / 1000000000L;
      next.tv_nsec += next.tv_nsec % 1000000000L;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

    pthread_mutex_lock(&datos->mutex);
    int number = rand() % 101;

    if (number < 80) {
      printf("Habitación 1: Activación de la luz artificial\n");
      datos->luz = number;
    } else if (number >= 90) {
      printf("Habitación 1: Activación cortinas\n");
      datos->luz = number;
    } else {
      printf("Habitación 1: Activación de la luz artificial\n");
      datos->luz = number;
    }
    pthread_mutex_unlock(&datos->mutex);
  }

  return NULL;
}

void* hab2_func(void* arg) {

  struct Habitacion2* datos = arg;

  struct timespec next;
  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1) {

    next.tv_sec += 3;
    next.tv_nsec += 0;

    if (next.tv_nsec >= 1000000000L) {
      next.tv_sec += next.tv_nsec / 1000000000L;
      next.tv_nsec += next.tv_nsec % 1000000000L;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

    
    pthread_mutex_lock(&datos->mutex);
    int number = rand() % 101;

    if (number < 80) {
      printf("Habitación 2: Activación de la luz artificial\n");
      datos->luz = number;
    } else if (number >= 90) {
      printf("Habitación 2: Activación cortinas\n");
      datos->luz = number;
    } else {
      printf("Habitación 2: Activación de la luz artificial\n");
      datos->luz = number;
    }
    pthread_mutex_unlock(&datos->mutex);
  }

  return NULL;
}

void* controlador_func(void* arg) {

  struct Data* datos = arg;

  struct timespec next;
  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1) {

    next.tv_sec += 4;
    next.tv_nsec += 0;

    if (next.tv_nsec >= 1000000000L) {
      next.tv_sec += next.tv_nsec / 1000000000L;
      next.tv_nsec += next.tv_nsec % 1000000000L;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);

    pthread_mutex_lock(&datos->habitacion1.mutex);
    pthread_mutex_lock(&datos->habitacion2.mutex);
    printf("Luz en la habitación 1: %d\n", datos->habitacion1.luz);
    printf("Luz en la habitación 2: %d\n", datos->habitacion2.luz);
    pthread_mutex_unlock(&datos->habitacion2.mutex);
    pthread_mutex_unlock(&datos->habitacion1.mutex);
  }
  

  return NULL;
}

int main() {

  mlockall(MCL_CURRENT | MCL_FUTURE);

  srand(time(0));

  pthread_mutexattr_t mutex_attr;
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_PROTECT);
  pthread_mutexattr_setprioceiling(&mutex_attr, PRIORIDAD_HABITACION_1);

  struct Data datos;
  datos.habitacion1.luz = 0;  
  datos.habitacion2.luz = 0;  
  pthread_mutex_init(&datos.habitacion1.mutex, NULL);
  pthread_mutex_init(&datos.habitacion2.mutex, NULL);
  pthread_mutexattr_destroy(&mutex_attr);

  pthread_t hab1, hab2, controlador;

  pthread_t hebraMain = pthread_self();
  int policy;
  struct sched_param param;
  pthread_getschedparam(hebraMain, &policy, &param);
  param.sched_priority = 100;
  pthread_setschedparam(hebraMain, policy, &param);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

  param.sched_priority = PRIORIDAD_CONTROLADOR;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&controlador, NULL, controlador_func, &datos);

  param.sched_priority = PRIORIDAD_HABITACION_1;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&hab1, NULL, hab1_func, &datos.habitacion1);

  param.sched_priority = PRIORIDAD_HABITACION_2;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&hab2, NULL, hab2_func, &datos.habitacion2);

  pthread_join(controlador, NULL);
  pthread_join(hab1, NULL);
  pthread_join(hab2, NULL);

  return 0;
}