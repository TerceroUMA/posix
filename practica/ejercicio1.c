//gcc -o main ejercicio1.c -pthread -lpthread -lrt && sudo taskset -c 0 ./main && rm ./main
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define NUM_THREADS 2
#define NUM_ITER 40
#define AUMENTO 100

int variableCompartida = 0;

/**
 * @brief 
 * 
 * @param segundosDeseadoInt 
 * @param nanoSegundosDeseado No puede ser >= 1E9.
 */
void espera_activa ( time_t seg ) {

  volatile time_t t = time (0) + seg ;
  while ( time (0) < t);

}

struct argumentosStruct {
  pthread_mutex_t mutex;
};

void *tareaA(void* arg) {

  struct argumentosStruct argumentos = *(struct argumentosStruct*) arg;

  for(int i = 0; i < NUM_ITER; i++) {

    pthread_mutex_lock(&argumentos.mutex);

    variableCompartida += 100;
    printf("La hebra %d aumenta el valor de la variable a: %d\n", 1, variableCompartida); fflush(stdout);

    pthread_mutex_unlock(&argumentos.mutex);

    espera_activa(1);
  }

  return NULL;
}

void *tareaB(void* arg) {

  struct argumentosStruct argumentos = *(struct argumentosStruct*) arg;

  for(int i = 0; i < NUM_ITER; i++) {

    pthread_mutex_lock(&argumentos.mutex);

    variableCompartida += 100;
    printf("La hebra %d aumenta el valor de la variable a: %d\n", 2, variableCompartida);

    pthread_mutex_unlock(&argumentos.mutex);

    espera_activa(1);

    int i = 0;

    while (i < 100000000) i++;

  }

  return NULL; 
}

int main(int argc, char *argv) {

  mlockall(MCL_CURRENT | MCL_FUTURE); // Bloque la memoria para que no se le vaya los datos con la paginación

  // Creo mi struct
  struct argumentosStruct argumentos;

  // Creo mi mutex
  pthread_mutex_init(&argumentos.mutex, NULL);

  // Creo una varibale para los param del main
  struct sched_param paramMain;

  // Cambiamos la prioridad de nuestro main
  pthread_t hebraMain = pthread_self();
  int policyMain = 0;
  pthread_getschedparam(hebraMain, &policyMain, &paramMain);
  paramMain.sched_priority = 100;
  pthread_setschedparam(hebraMain, policyMain, &paramMain);

  // Creo varibales para los param de mis hebras
  struct sched_param paramA;
  struct sched_param paramB;
  paramA.sched_priority = 26;
  paramB.sched_priority = 24;

  // Creo los atributos
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED); // El segundo parámetro dice que le vamos escribir la prioridad de forma explicita  

  pthread_t threads[NUM_THREADS];

  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  pthread_attr_setschedparam(&attr, &paramA);

  //printf("Se crea la hebra %d\n", 1);
  int rc = pthread_create(&threads[0], &attr, tareaA, &argumentos);
  if (rc) {
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }

  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  pthread_attr_setschedparam(&attr, &paramB);

  //printf("Se crea la hebra %d\n", 2);
  rc = pthread_create(&threads[1], &attr, tareaB, &argumentos);
  if (rc) {
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }


  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("\n\nFin del programa.\n\n"); fflush(stdout);

  pthread_attr_destroy(&attr);
}