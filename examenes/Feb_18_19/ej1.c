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

#define SIG_ABRIR (SIGRTMIN)
#define SIG_CERRAR (SIGRTMIN + 1)

sigset_t sigset;

struct Data {
  pthread_mutex_t mutex;
  int agua;
};

void* controlador_func(void* arg) {

  struct Data* datos = arg;
  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_ABRIR);
  sigaddset(&sigset, SIG_CERRAR);

  int info;

  while(1) {
    sigwait(&sigset, &info);
    int number = rand() % 101;

    if (info == SIG_ABRIR) {
      if (number < 60) {
        pthread_mutex_lock(&datos->mutex);
        printf("Abriendo válvula\n"); fflush(stdout);
        datos->agua++;
        printf("Agua: %d\n", datos->agua);
        pthread_mutex_unlock(&datos->mutex);
      }
    } else if (info == SIG_CERRAR) {
      if (number < 40) {
        pthread_mutex_lock(&datos->mutex);
        printf("Cerrando válvula\n"); fflush(stdout);
        datos->agua--;
        printf("Agua: %d\n", datos->agua);
        pthread_mutex_unlock(&datos->mutex);
      }
    } else {
      printf("Error\n"); fflush(stdout);
    }
  }

  return NULL;
  
}

void* abrir_func(void* arg) {

  struct Data* datos = arg;

  timer_t timerId;
  struct timespec periodo = { 1, 0 };
  struct sigevent sigev;
  struct itimerspec its;

  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIG_ABRIR;
  sigev.sigev_value.sival_ptr = &timerId;

  timer_create(CLOCK_MONOTONIC, &sigev, &timerId);

  its.it_interval = periodo;
  its.it_value.tv_sec = 1;
  its.it_value.tv_nsec = 0;

  timer_settime(timerId, 0, &its, NULL);

  return NULL;

}

void* cerrar_func(void* arg) {

  timer_t timerId;
  struct timespec periodo = { 2, 0 };
  struct itimerspec its;
  struct sigevent sigev;

  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIG_CERRAR;
  sigev.sigev_value.sival_ptr = &timerId;

  timer_create(CLOCK_MONOTONIC, &sigev, &timerId);

  its.it_interval = periodo;
  its.it_value.tv_sec = 1;
  its.it_value.tv_nsec = 0;

  timer_settime(timerId, 0, &its, NULL);

  return NULL;

}

int main() {

  mlockall(MCL_CURRENT | MCL_FUTURE);

  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_ABRIR);
  sigaddset(&sigset, SIG_CERRAR);
  pthread_sigmask(SIG_BLOCK, &sigset, NULL);

  srand(time(0));

  pthread_t hebraMain = pthread_self();
  int policy;
  struct sched_param param;
  pthread_getschedparam(hebraMain, &policy, &param);
  param.sched_priority = 100;
  pthread_setschedparam(hebraMain, policy, &param);

  struct Data datos;
  datos.agua = 0;
  pthread_mutex_init(&datos.mutex, NULL);

  pthread_t abrir, cerrar, controlador;

  pthread_create(&controlador, NULL, controlador_func, &datos);
  pthread_create(&abrir, NULL, abrir_func, &datos);
  pthread_create(&cerrar, NULL, cerrar_func, &datos);

  pthread_join(controlador, NULL);
  pthread_join(abrir, NULL);
  pthread_join(cerrar, NULL);

  printf("FIN\n"); fflush(stdout);

  return 0;
}