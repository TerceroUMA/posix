//gcc -o main ejercicio2conRegionesCondicion.c -lpthread && ./main && rm ./main

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 10

pthread_mutex_t mutex; // Mutex para proteger la región crítica
pthread_cond_t hayChicasDentro; // Un condición de espera
pthread_cond_t hayChicosDentro; // Un condición de espera

int chicosDentro = 0;
int chicasDentro = 0;

void *mujer(void* idParam) {

  int id = *(int*) idParam;


  pthread_mutex_lock(&mutex);

  while(chicosDentro >= 1) {
    pthread_cond_wait(&hayChicosDentro, &mutex);
  }
  chicasDentro += 1;

  printf("La hebra %d, que es mujer entra:\n", id) ;
  printf("Hay %d mujeres y %d chicos:\n", chicasDentro, chicosDentro) ;

  // La hebrá tendrá que dormir y luego salir del baño. ¿como?

  // Este o el otro? pthread_cond_signal(&variable_libre);
  pthread_mutex_unlock(&mutex);

  return NULL;
}



int main(int argc, char *argv) {

  pthread_t threads[NUM_THREADS];
  char caracteres[3] = {'1', '2', '3'};

  
  pthread_mutex_init(&mutex,NULL);
  pthread_cond_init(&hayChicasDentro, NULL);
  pthread_cond_init(&hayChicosDentro, NULL);

  for (int i = 0; i < NUM_THREADS; i++) {

    if (i % 2 == 0) {

      int rc = pthread_create(&threads[i], NULL, mujer, &i);

      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      }

    } else {

     /*  int rc = pthread_create(&threads[i], NULL, acceso, &i);

      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      } */
    }

    /* printf("Se crea la hebra %d\n", i + 1); */

  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("\n\nCon las regiones críticas junto con condiciones de espera no tenemos pérdidas en el número de incrementaciones.\n\n");
}