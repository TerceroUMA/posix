//gcc -o main ejercicio2.c -lpthread && ./main && rm ./main

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 2
#define NUM_ITER 100000

int variableCompartida = 0;

void *acceso(void* idParam) {

  int id = *(int*) idParam;

  for(int i = 0; i < NUM_ITER; i++) {
    variableCompartida++;
    printf("La hebra %d aumenta el valor de la variable a: %d\n", id, variableCompartida) ;
  }

  return NULL;
}

int main(int argc, char *argv) {

  pthread_t threads[NUM_THREADS];
  char caracteres[3] = {'1', '2', '3'};


  for (int i = 0; i < NUM_THREADS; i++) {

    printf("Se crea la hebra %d\n", i + 1);
    int rc = pthread_create(&threads[i], NULL, acceso, &i);

    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }

  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("\n\nCon 100 iteraciones no notamos perdidas de incrementaciones en la variable pero con 100000 vemos que ya hay pérdidas, habría que implementar un semáforo\n\n");
}