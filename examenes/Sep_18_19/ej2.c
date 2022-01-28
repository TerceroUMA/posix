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

#define PRIORIDAD_VIBRACION 20
#define PRIORIDAD_MOVIMIENTO 30
#define PRIORIDAD_MONITOR 10
#define TIPO_PLANIFICACION SCHED_FIFO

struct dataVibracion {
  int activo;
  pthread_mutex_t mutex;
};

struct dataMovimiento {
  int activo;
  pthread_mutex_t mutex;
};

struct datosMonitor {
  struct dataMovimiento datosMovimiento;
  struct dataVibracion datosVicracion;
};

void* vibracion_func(void* arg) {

  struct dataVibracion* datos = arg;

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

    int number = 1 + (rand() % (100 - 1 + 1));

    if (number > 50) {
      printf("Posible entrada por ventana\n");
      datos->activo = 1;
    } else {
      printf("Todo OK\n");
      datos->activo = 0;
    }
    pthread_mutex_unlock(&datos->mutex);
  }
  
  return NULL;
}

void* movimiento_func(void* arg) {
  
  struct dataMovimiento* datos = arg;

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
    int number = 1 + (rand() % (100 - 1 + 1));

    if (number > 50) {
      printf("Posible presencia\n");
      datos->activo = 1;
    } else {
      printf("Todo OK\n");
      datos->activo = 0;
    }

    pthread_mutex_unlock(&datos->mutex);

  }

  return NULL;
}

void* monitor_func(void* arg) {


  struct datosMonitor* datos = arg;

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

    pthread_mutex_lock(&datos->datosMovimiento.mutex);
    pthread_mutex_lock(&datos->datosVicracion.mutex);
    printf("Datos actuales:\n"); fflush(stdout);
    printf("\t- Vibracion: %d\n", datos->datosVicracion.activo);
    printf("\t- Movimiento: %d\n", datos->datosMovimiento.activo);
    pthread_mutex_unlock(&datos->datosVicracion.mutex);
    pthread_mutex_unlock(&datos->datosMovimiento.mutex);
  }

  return NULL;
}

int main() {

  mlockall(MCL_CURRENT | MCL_FUTURE);

  srand(time(0));

  pthread_t hebra_main = pthread_self();
  int policy;
  struct sched_param param;

  pthread_getschedparam(hebra_main, &policy, &param);
  param.sched_priority = 100;
  pthread_setschedparam(hebra_main, policy, &param);

  struct datosMonitor datosMonitor;
  datosMonitor.datosMovimiento.activo = 0;
  datosMonitor.datosVicracion.activo = 0;

  pthread_mutexattr_t mutex_attr;
  pthread_mutexattr_init(&mutex_attr);
  pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_PROTECT);
  pthread_mutexattr_setprioceiling(&mutex_attr, PRIORIDAD_MOVIMIENTO);

  pthread_mutex_init(&datosMonitor.datosMovimiento.mutex, &mutex_attr);
  pthread_mutex_init(&datosMonitor.datosVicracion.mutex, &mutex_attr);

  pthread_t monitor, sensor_vibracion, sensor_movimiento;


  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr, TIPO_PLANIFICACION);

  param.sched_priority = PRIORIDAD_MONITOR;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&monitor, &attr, monitor_func, &datosMonitor);

  param.sched_priority = PRIORIDAD_VIBRACION;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&sensor_vibracion, &attr, vibracion_func, &datosMonitor.datosVicracion);

  param.sched_priority = PRIORIDAD_MOVIMIENTO;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&sensor_movimiento, &attr, movimiento_func, &datosMonitor.datosMovimiento);
  
  pthread_join(monitor, NULL);
  pthread_join(sensor_vibracion, NULL);
  pthread_join(sensor_movimiento, NULL);

  printf("FIN\n"); fflush(stdout);
  return 0;
}

