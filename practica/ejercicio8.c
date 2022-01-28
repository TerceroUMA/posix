//gcc -o main ejercicio8.c -pthread -lpthread -lrt && sudo taskset -c 0 ./main && rm ./main

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sched.h>
#include <pthread.h>
#include <sys/mman.h>
#include <signal.h>

#define SIG_A (SIGRTMIN)
#define SIG_B (SIGRTMIN + 1)
#define SIG_C (SIGRTMAX)

struct Data {
  int signum;
};

void* tarea(void* arg) {

  struct Data* data = arg;

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, data->signum);

  siginfo_t siginfo;
  sigwaitinfo(&sigset, &siginfo);

  if (siginfo.si_signo == SIG_A) {
    printf("Soy la tarea A y me ha llegado el valor: %d\n", siginfo.si_value.sival_int);
  } else if (siginfo.si_signo == SIG_B) {
    printf("Soy la tarea B y me ha llegado el valor: %d\n", siginfo.si_value.sival_int);
  } else if (siginfo.si_signo == SIG_C) {
    printf("Soy la tarea C y me ha llegado el valor: %d\n", siginfo.si_value.sival_int);
  } else {
    printf("WTF\n");
  }
  

  return NULL;
}

int main() {

  mlockall(MCL_CURRENT | MCL_FUTURE);

  pthread_t hebraMain = pthread_self();
  int policy;
  struct sched_param param;
  pthread_getschedparam(hebraMain, &policy, &param);
  param.sched_priority = 100;
  pthread_setschedparam(hebraMain, policy, &param);

  sigset_t sigset;
  sigemptyset(&sigset);
  sigaddset(&sigset, SIG_A);
  sigaddset(&sigset, SIG_B);
  sigaddset(&sigset, SIG_C);
  pthread_sigmask(SIG_BLOCK, &sigset, NULL);

  struct Data data;
  pthread_t tarea_a, tarea_b, tarea_c;
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);

  data.signum = SIG_A;
  param.sched_priority = 1;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tarea_a, &attr, tarea, &data);

  data.signum = SIG_B;
  param.sched_priority = 2;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tarea_b, &attr, tarea, &data);

  data.signum = SIG_C;
  param.sched_priority = 10;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tarea_c, &attr, tarea, &data);


  sleep(2);
  union sigval sigv; 
  sigv.sival_int = 3;
  sigqueue(getpid(), SIG_A, sigv);
  sigv.sival_int = 4;
  sigqueue(getpid(), SIG_C, sigv);
  sigv.sival_int = 2;
  sigqueue(getpid(), SIG_B, sigv);

  pthread_join(tarea_a, NULL);
  pthread_join(tarea_b, NULL);
  pthread_join(tarea_c, NULL);

  return 0;
}