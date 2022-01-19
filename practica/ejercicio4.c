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

#define PERIODO_A_SEC 90
#define PERIODO_A_NSEC 200000000
#define PRIORIDAD_A 24
#define ITER_A 40
#define INC_A 100
#define PERIODO_B_SEC 100
#define PERIODO_B_NSEC 300000000
#define PRIORIDAD_B 26
#define ITER_B 40
#define INC_B 1


pthread_mutex_t mutex;
int contador = 0;
sigset_t seniales;

void espera_activa(time_t tiempo) {
  
  volatile time_t t = time(0) + tiempo;
  while(time(0) < t) {}
}

void* tareaA(void* arg) {

  const struct timespec periodo = { PERIODO_A_SEC, PERIODO_A_NSEC };
  timer_t timerId;
  struct sigevent sgev;
  struct itimerspec its;
  sigset_t sigset;
  int signum;

  sgev.sigev_notify = SIGEV_SIGNAL; // modo?
  sgev.sigev_signo = SIGUSR1; // La señal que se enviará
  sgev.sigev_value.sival_ptr = &timerId;

  timer_create(CLOCK_MONOTONIC, &sgev, &timerId);

  its.it_interval = periodo;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1; // <- no puede ser 0

  timer_settime(timerId, 0, &its, NULL);

  sigemptyset(&sigset);
  sigaddset(&sigset, SIGUSR1);

  while(1) {

    sigwait(&sigset, &signum);

    for(int i = 0; i < ITER_A; ++i) {

      pthread_mutex_lock(&mutex);

      contador += INC_A;
      printf("Tarea A aumenta el contado a: %d\n", contador); fflush(stdout);
      pthread_mutex_unlock(&mutex);
      espera_activa(1);
    }
  }
  timer_delete(timerId);

  return NULL;
}

void* tareaB(void* arg) {

  timer_t timerId;
  const struct timespec periodo = { PERIODO_B_SEC, PERIODO_B_NSEC };
  sigset_t sigset;
  struct itimerspec its;
  struct sigevent sigev;
  int signum;

  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIGUSR2;
  sigev.sigev_value.sival_ptr = &timerId;

  timer_create(CLOCK_MONOTONIC, &sigev, &timerId);

  its.it_interval = periodo;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1; // Tiene que ser distinto de 0

  timer_settime(timerId, 0, &its, NULL);

  sigemptyset(&sigset);
  sigaddset(&sigset, SIGUSR2);

  while (1) {

    sigwait(&sigset, &signum);

    for(int i = 0; i < ITER_A; ++i) {
      pthread_mutex_lock(&mutex);
      contador += INC_B;
      printf("Tarea B aumenta el contado a: %d\n", contador); fflush(stdout);
      pthread_mutex_unlock(&mutex);
    }
  }

  timer_delete(timerId);

  return NULL;
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
  pthread_create(&tareaHebraB, &attr, tareaB, NULL);

  pthread_join(tareaHebraA, NULL);
  pthread_join(tareaHebraB, NULL);

  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex);
}