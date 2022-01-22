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

#define SIG_FIN ( SIGRTMIN )
#define SIG_BAT ( SIGRTMIN + 1)
#define SIG_EVENTO ( SIGRTMIN + 2)
#define SIG_LLAMADA ( SIGRTMIN + 3)
#define SIG_SMS ( SIGRTMIN + 4)
#define SIG_MTR ( SIGRTMIN + 5)

sigset_t sigset;
int bateria = 100;
pthread_mutex_t mutex;

void* monitorizacion_func() {

  int info = 0;
  int fin = 0;
  const struct timespec periodo = { 5, 0 };

  timer_t timerId;

  struct sigevent sigev;
  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIG_MTR;
  sigev.sigev_value.sival_ptr = &timerId; 

  timer_create(CLOCK_MONOTONIC, &sigev, &timerId);

  struct itimerspec its;
  its.it_interval = periodo;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1; // Periodico

  timer_settime(timerId, 0, &its, NULL);

  sigset_t sset;
  sigemptyset(&sset);
  sigaddset(&sset, SIG_MTR);
  sigaddset(&sset, SIG_FIN);

  while (fin == 0) {

    sigwait(&sset, &info);

    if (info == SIG_FIN) {
      fin = 1;
    } else if (info == SIG_MTR) {
      pthread_mutex_lock(&mutex);
      printf("Hay una batería del %d porciento \n", bateria); fflush(stdout);
      pthread_mutex_unlock(&mutex);
    } else {
      printf("Error inesperado en el monitor\n"); fflush(stdout);
    }
  }
  
  timer_delete(timerId);

  return NULL;
}

void* bateria_func() {

  int info = 0;
  int fin = 0;
  const struct timespec periodo = { 1, 0 };

  timer_t timerId;
  sigevent_t sigev;
  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIG_BAT;
  sigev.sigev_value.sival_ptr = &timerId;

  timer_create(CLOCK_MONOTONIC, &sigev, &timerId);

  struct itimerspec its;
  its.it_interval = periodo;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1;

  timer_settime(timerId, 0, &its, NULL);


  sigset_t sset;
  sigemptyset(&sset);
  sigaddset(&sset, SIG_BAT);
  
  while (fin == 0) {

    sigwait(&sset, &info);

    if (info == SIG_BAT) {
      
      pthread_mutex_lock(&mutex);
      bateria--;

      if (bateria <= 0) {
        fin = 1;
      }

      pthread_mutex_unlock(&mutex);
    } else {
      printf("Error inesperado en la batería\n"); fflush(stdout);
    }
  }

  union sigval sigval;
  sigqueue(getpid(), SIG_FIN, sigval);
  sigqueue(getpid(), SIG_FIN, sigval);
  sigqueue(getpid(), SIG_FIN, sigval);

  timer_delete(timerId);

  return NULL;
}

void* event_func() {

  int info = 0;
  int fin = 0;
  const struct timespec periodo = { 1, 0 };

  timer_t timerId;
  sigevent_t sigev;
  sigev.sigev_notify = SIGEV_SIGNAL;
  sigev.sigev_signo = SIG_EVENTO;
  sigev.sigev_value.sival_ptr = &timerId;

  timer_create(CLOCK_MONOTONIC, &sigev, &timerId);

  struct itimerspec its;
  its.it_interval = periodo;
  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = 1;

  timer_settime(timerId, 0, &its, NULL);

  sigset_t sset;
  sigemptyset(&sset);
  sigaddset(&sset, SIG_EVENTO);
  sigaddset(&sset, SIG_FIN);

  union sigval sigValue;
  
  while (fin == 0) {

    sigwait(&sset, &info);

    if (info == SIG_EVENTO) {
      
      int random = rand() % 100 + 1;

      if (0 <= random && random < 10) {
        
        int random2 = rand() % 100 + 1;
        sigValue.sival_int = random2;
        sigqueue(getpid(), SIG_LLAMADA, sigValue);

      } else if (10 <= random && random < 20) {
        
        int random2 = rand() % 100 + 1;
        sigValue.sival_int = random2;
        sigqueue(getpid(), SIG_SMS, sigValue);

      }


    } else if (SIG_FIN) {
      fin = 1;
    } else {
      printf("Error inesperado en la batería\n"); fflush(stdout);
    }
  }

  timer_delete(timerId);

  return NULL;
}

void* telefono_func() {

  int info = 0;
  int fin = 0;

  sigset_t sset;
  sigemptyset(&sset);
  sigaddset(&sset, SIG_LLAMADA);
  sigaddset(&sset, SIG_SMS);
  sigaddset(&sset, SIG_FIN);
  
  while (fin == 0) {

    sigwait(&sset, &info);

    if (info == SIG_LLAMADA) {
      printf("Ha llegado una llamada al teléfono\n"); fflush(stdout);
    } else if (info == SIG_SMS) {
      printf("Ha llegado un SMS al teléfono\n"); fflush(stdout);
    } else if (info == SIG_FIN) {
      fin = 1;
    }else {
      printf("Error inesperado en la batería\n"); fflush(stdout);
    }
  }

  return NULL;
}

int main() {

  mlockall(MCL_CURRENT || MCL_FUTURE);

  srand(time(0));
  

  pthread_mutex_init(&mutex, NULL);

  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_FIN);
  sigaddset(&sigset, SIG_BAT);
  sigaddset(&sigset, SIG_EVENTO);
  sigaddset(&sigset, SIG_LLAMADA);
  sigaddset(&sigset, SIG_SMS);
  sigaddset(&sigset, SIG_MTR);
  pthread_sigmask(SIG_BLOCK, &sigset, NULL);

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

  printf("Fin"); fflush(stdout);
}