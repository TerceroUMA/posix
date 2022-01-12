//gcc -o main ejercicio4.c -lpthread && ./main && rm ./main

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Para el sleep

#define NUM_THREADS 10

pthread_mutex_t mutex; // Mutex para proteger la región crítica
/* pthread_cond_t hayChicasDentro; // Un condición de espera
pthread_cond_t hayChicosDentro; // Un condición de espera */

pthread_cond_t noPuedePasar;

int chicosDentro = 0;
int chicasDentro = 0;

void entraChica(int id) {

   pthread_mutex_lock(&mutex);

    while(chicosDentro >= 1 || chicasDentro >= 1) {
      pthread_cond_wait(&noPuedePasar, &mutex);
    }
    
    chicasDentro += 1;

    printf("La hebra %d, que es mujer, entra\n", id) ;
    printf("Hay %d mujeres y %d chicos\n\n", chicasDentro, chicosDentro) ;

    pthread_cond_broadcast(&noPuedePasar);
    pthread_mutex_unlock(&mutex);
}

void entraChico(int id) {

   pthread_mutex_lock(&mutex);

    while(chicosDentro >= 5 || chicasDentro >= 1) {
      pthread_cond_wait(&noPuedePasar, &mutex);
    }
    
    chicosDentro += 1;

    printf("La hebra %d, que es hombre, entra\n", id) ;
    printf("Hay %d mujeres y %d chicos\n\n", chicasDentro, chicosDentro) ;

    pthread_cond_broadcast(&noPuedePasar);
    pthread_mutex_unlock(&mutex);
}

/**
 * @brief Alguien va a salir del baño, puede se un chico o una chica
 * 
 * @param id Identificador de la hebra
 * @param genero 0 si es chico o 1 si es chica
 */
void saleAlguien(int id, int genero) {

  pthread_mutex_lock(&mutex);

  if (genero == 0) {
    chicosDentro -= 1;
    printf("La hebra %d, que es hombre, sale\n", id) ;
  } else {
    chicasDentro -= 1;
    printf("La hebra %d, que es mujer, sale\n", id) ;
  }

  printf("Hay %d mujeres y %d chicos\n\n", chicasDentro, chicosDentro) ;


  pthread_cond_broadcast(&noPuedePasar);
  pthread_mutex_unlock(&mutex);
}

void *mujer(void* idParam) {

  int id = *(int*) idParam;

  for (int i = 0; i < 10; i++) {

    entraChica(id);
    sleep(2);
    saleAlguien(id, 1);
    // La hebrá tendrá que dormir y luego salir del baño. ¿como?
  }

  return NULL;
}

void *hombre(void* idParam) {

  int id = *(int*) idParam;

  for (int i = 0; i < 10; i++) {

    entraChico(id);
    sleep(2);
    saleAlguien(id, 0);
    // La hebrá tendrá que dormir y luego salir del baño. ¿como?
  }

  return NULL;
}


int main(int argc, char *argv) {

  pthread_t threads[NUM_THREADS];
  char caracteres[3] = {'1', '2', '3'};

  
  pthread_mutex_init(&mutex,NULL);
  pthread_cond_init(&noPuedePasar, NULL);

  for (int i = 0; i < NUM_THREADS; i++) {

    if (i % 2 == 0) {

      int rc = pthread_create(&threads[i], NULL, mujer, &i);

      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      }

    } else {

      int rc = pthread_create(&threads[i], NULL, hombre, &i);

      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      }
    }

  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  printf("\n\nCon las regiones críticas junto con condiciones de espera no tenemos pérdidas en el número de incrementaciones.\n\n");
}

/* 
  void *hombre(void* idParam) {

  int id = *(int*) idParam;

  for (int i = 0; i < 10; i++) {

    pthread_mutex_lock(&mutex);

    while(chicosDentro >= 5 || chicasDentro >= 1) {
      pthread_cond_wait(&noPuedePasar, &mutex);
    }
    
    chicosDentro += 1;

    printf("La hebra %d, que es hombre, entra:\n", id) ;
    printf("Hay %d mujeres y %d chicos:\n\n", chicasDentro, chicosDentro) ;

    pthread_cond_broadcast(&noPuedePasar);
    pthread_mutex_unlock(&mutex);

    sleep(2);
    
    pthread_mutex_lock(&mutex);

    chicosDentro -= 1;

    printf("La hebra %d, que es hombre, sale:\n", id) ;
    printf("Hay %d mujeres y %d chicos:\n\n", chicasDentro, chicosDentro) ;

    // La hebrá tendrá que dormir y luego salir del baño. ¿como?

    pthread_cond_broadcast(&noPuedePasar);
    pthread_mutex_unlock(&mutex);
  }

  return NULL;
}
 */