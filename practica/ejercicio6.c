//gcc -o main ejercicio6.c -pthread -lpthread -lrt && sudo taskset -c 0 ./main && rm ./main

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

pthread_mutex_t mutex;

int contador = 0;

void espera_activa(time_t seg) {

  time_t tiempo = time(0) + seg;
  while (time(0) < tiempo) {}
  
}

void* tarea_a_func() {

  while (1) {

    sleep(3);
    pthread_mutex_lock(&mutex);
    ++contador;
    printf("A -> Contador: %d\n",contador); fflush(stdout);
    pthread_mutex_unlock(&mutex);
  }  

  return NULL;
}

void* tarea_b_func() {

  sleep(5);
  espera_activa(15);

  return NULL;
}

void* tarea_c_func() {

  while (1) {

    sleep(1);
    pthread_mutex_lock(&mutex);
    espera_activa(7);
    ++contador;
    printf("C -> Contador: %d\n",contador); fflush(stdout);
    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}

int main() {

  mlockall(MCL_CURRENT | MCL_FUTURE);

  // Herencia de prioridad: PTHREAD_PRIO_INHERIT
  // Techo de prioridad: PTHREAD_PRIO_PROTECT

  pthread_mutexattr_t mutexAttr;
  pthread_mutexattr_setprotocol(&mutexAttr, PTHREAD_PRIO_INHERIT);
  //pthread_mutexattr_setprioceiling(&mutexAttr, 75); //? Solo en el techo de prioridad. El valor es la prioridad más grande de las hebras que pueden acceder al recurso protegido.
  pthread_mutex_init(&mutex, NULL);
  pthread_mutexattr_destroy(&mutexAttr);

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

  pthread_t tarea_a, tarea_b, tarea_c;
  param.sched_priority = 75;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tarea_a, &attr, tarea_a_func, NULL);
  
  param.sched_priority = 50;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tarea_b, &attr, tarea_b_func, NULL);
  
  param.sched_priority = 25;
  pthread_attr_setschedparam(&attr, &param);
  pthread_create(&tarea_c, &attr, tarea_c_func, NULL);

  pthread_join(tarea_a, NULL);
  pthread_join(tarea_b, NULL);
  pthread_join(tarea_c, NULL);
  
  pthread_attr_destroy(&attr);
  pthread_mutex_destroy(&mutex);

  printf("Fin"); fflush(stdout);  
  
  return 0;

}