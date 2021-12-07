//gcc -o main ejercicio2conRegionesCondicion.c -lpthread && ./main && rm ./main

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4
#define NUM_ITER 100000

pthread_mutex_t mutex; // Mutex para proteger la región crítica
pthread_cond_t variable_libre; // Un condición de espera

int variableCompartida = 0;
int usando = 0;

void *acceso(void* idParam) {

  int id = *(int*) idParam;

  for(int i = 0; i < NUM_ITER; i++) {

    pthread_mutex_lock(&mutex);

    while(usando == 1) {
      pthread_cond_wait(&variable_libre, &mutex);
    }
    usando = 1;

    variableCompartida++;
    printf("La hebra %d aumenta el valor de la variable a: %d\n", id, variableCompartida) ;

    usando = 0;
    pthread_cond_signal(&variable_libre);
    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}

int main(int argc, char *argv) {

  pthread_t threads[NUM_THREADS];
  char caracteres[3] = {'1', '2', '3'};

  
  pthread_mutex_init(&mutex,NULL);
  pthread_cond_init(&variable_libre, NULL);

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

  printf("\n\nCon las regiones críticas junto con condiciones de espera no tenemos pérdidas en el número de incrementaciones.\n\n");
}