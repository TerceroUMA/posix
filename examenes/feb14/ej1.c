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

#define SIG_ENTRADA (SIGRTMIN)
#define SIG_SALIDA  (SIGRTMIN + 1)
#define SIG_CONTROL (SIGRTMIN + 2)

struct Data {
  pthread_mutex_t mutex;
  int recinto;
};

sigset_t sigset;

void* entrada_func(void* arg) {

  struct Data* datos = arg;

  timer_t timer;
  sigset_t sigset;
  struct timespec periodo = { 1, 0 };
  struct itimerspec its;
  struct sigevent sigev;

  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIG_ENTRADA;
  sigev.sigev_value.sival_ptr = &timer;
  
  timer_create(CLOCK_MONOTONIC, &sigev, &timer);

  its.it_interval = periodo;
  its.it_value.tv_sec = 1;
  its.it_value.tv_nsec = 0;

  timer_settime(timer, 0, &its, NULL);

  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_ENTRADA);
  int info;

  while(1) {

    sigwait(&sigset, &info);

    if (info == SIG_ENTRADA) {

      pthread_mutex_lock(&datos->mutex);
      datos->recinto++;
      pthread_mutex_unlock(&datos->mutex);

      int number = rand() % 100;
      union sigval s;
      if (number < 60) {
        sigqueue(getpid(), SIG_CONTROL, s);
      }
    }

  }

  timer_delete(timer);

  return NULL;
}

void* salida_func(void* arg) {

  struct Data* datos = arg;

  timer_t timer;
  sigset_t sigset;
  struct timespec periodo = { 2, 0 };
  struct itimerspec its;
  struct sigevent sigev;

  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIG_SALIDA;
  sigev.sigev_value.sival_ptr = &timer;
  
  timer_create(CLOCK_MONOTONIC, &sigev, &timer);

  its.it_interval = periodo;
  its.it_value.tv_sec = 1;
  its.it_value.tv_nsec = 0;

  timer_settime(timer, 0, &its, NULL);

  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_SALIDA);
  int info;

  while(1) {

    sigwait(&sigset, &info);

    if (info == SIG_SALIDA) {

      pthread_mutex_lock(&datos->mutex);
      datos->recinto--;
      pthread_mutex_unlock(&datos->mutex);

      int number = rand() % 100;
      union sigval s;
      if (number < 40) {
        sigqueue(getpid(), SIG_CONTROL, s);
      }
    }

  }

  timer_delete(timer);

  return NULL;
}

void* controlador_func(void* arg) {

  struct Data* datos = arg;

  sigset_t sigset;

  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_CONTROL);
  int info;

  while(1) {

    sigwait(&sigset, &info);

    if (info == SIG_CONTROL) {

      pthread_mutex_lock(&datos->mutex);
      printf("En el recinto hay %d personas\n", datos->recinto);
      pthread_mutex_unlock(&datos->mutex);

    }
  }

  return NULL;
}


int main() {

  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_ENTRADA);
  sigaddset(&sigset, SIG_SALIDA);
  sigaddset(&sigset, SIG_CONTROL);

  pthread_sigmask(SIG_BLOCK, &sigset, NULL);

  srand(time(NULL));

  struct Data datos;
  datos.recinto = 0;

  pthread_mutex_init(&datos.mutex, NULL);

  pthread_t hebraMain = pthread_self();
  int policy;
  struct sched_param param;
  pthread_getschedparam(hebraMain, &policy, &param);
  param.sched_priority = 100;
  pthread_setschedparam(hebraMain, policy, &param);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr,  PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

  param.sched_priority = 5;
  pthread_attr_setschedparam(&attr, &param);

  pthread_t entrada, salida, controlador;

  pthread_create(&entrada, &attr, entrada_func, &datos);
  pthread_create(&salida, &attr, salida_func, &datos);
  pthread_create(&controlador, &attr, controlador_func, &datos);

  pthread_join(entrada, NULL);
  pthread_join(salida, NULL);
  pthread_join(controlador, NULL);

  pthread_attr_destroy(&attr);

  return 0;
}