//gcc -o main ej1.c -pthread -lpthread -lrt && sudo taskset -c 0 ./main && rm ./main

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

#define SIG_COCHE  (SIGRTMIN)
#define SIG_CAMION (SIGRTMIN + 1)
#define SIG_SALIDA (SIGRTMIN + 2)

sigset_t sigset;

struct parkingData {
  int coches;
  int camiones;
};

void* tarea_coche() {

  timer_t timerId;
  struct timespec periodo = { 2, 0 };
  struct sigevent sige;
  sige.sigev_notify = SIGEV_SIGNAL;
  sige.sigev_signo = SIG_COCHE;
  sige.sigev_value.sival_ptr = &timerId;

  timer_create(CLOCK_MONOTONIC, &sige, &timerId);

  struct itimerspec its;
  its.it_interval = periodo;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1;
  timer_settime(timerId, 0, &its, NULL);

  return NULL;

}

void* tarea_camion() {

  struct timespec periodo = { 3, 0 };
  struct itimerspec its;
  timer_t timerId;
  struct sigevent sigev;

  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIG_CAMION;
  sigev.sigev_value.sival_ptr = &timerId;

  timer_create(CLOCK_MONOTONIC, &sigev, &timerId);

  its.it_interval = periodo;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1;

  timer_settime(timerId, 0, &its, NULL);

  return NULL;

}

void* tarea_parking(void* arg) {

  struct parkingData* data = arg;
  int info;
  sigset_t sigset;

  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_COCHE);
  sigaddset(&sigset, SIG_CAMION);

  timer_t timerId;
  struct timespec periodo = { 2, 0 };
  struct itimerspec its;
  struct sigevent sigev;
  
  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIG_SALIDA;
  sigev.sigev_value.sival_ptr = &timerId;

  timer_create(CLOCK_MONOTONIC, &sigev, &timerId);

  its.it_interval = periodo;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1 ;

  timer_settime(timerId, 0, &its, NULL);

  while(1) {
    sigwait(&sigset, &info);

    if (info == SIG_COCHE) {
      printf("Coche entra\n");
      data->coches++;
      printf("Hay:\n\t- Coches: %d\n\t- Camiones:%d\n", data->coches, data->camiones); fflush(stdout);
    } else if (info == SIG_CAMION) {
      printf("Camión entra\n");
      data->camiones++;
      printf("Hay:\n\t- Coches: %d\n\t- Camiones:%d\n", data->coches, data->camiones); fflush(stdout);
    } else {
      printf("Problemas\n");
    }
  }

  return NULL;

}

void* tarea_salida(void* arg) {

  struct parkingData* data = arg;

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_SALIDA);
  int info;

  while (1) {

    sigwait(&sigset, &info);

    if (info == SIG_SALIDA) {

      int number = 0 + (rand() % (1 - 0 + 1));

      if (number == 0 && data->coches > 0) {
        printf("Coche sale\n");
        data->coches--;    
        printf("Hay:\n\t- Coches: %d\n\t- Camiones:%d\n", data->coches, data->camiones); fflush(stdout);
      } else if (number == 1 && data->camiones > 0) {
        printf("Camión sale\n");
        data->camiones--;
        printf("Hay:\n\t- Coches: %d\n\t- Camiones:%d\n", data->coches, data->camiones); fflush(stdout);
      }

    } else {
      printf("Problemas\n");
    }

  }

  return NULL;
}

int main() {

  mlockall(MCL_CURRENT | MCL_FUTURE);

  srand(time(0));

  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_COCHE);
  sigaddset(&sigset, SIG_CAMION);
  sigaddset(&sigset, SIG_SALIDA);
  pthread_sigmask(SIG_BLOCK, &sigset, NULL);

  pthread_t hebraMain = pthread_self();
  int policy;
  struct sched_param param;
  pthread_getschedparam(hebraMain, &policy, &param);
  param.sched_priority = 100;
  pthread_setschedparam(hebraMain, policy, &param);

  pthread_t coche, camion, parking, salida;

  struct parkingData pData;
  pData.camiones = 0;
  pData.coches = 0;

  pthread_create(&coche, NULL, tarea_coche, NULL);
  pthread_create(&camion, NULL, tarea_camion, NULL);
  pthread_create(&parking, NULL, tarea_parking, &pData);
  pthread_create(&salida, NULL, tarea_salida, &pData);

  pthread_join(coche, NULL);
  pthread_join(camion, NULL);
  pthread_join(parking, NULL);
  pthread_join(salida, NULL);

  printf("\nFIN\n");

  return 0;
}