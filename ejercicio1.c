//gcc -o main ejercicio1.c -lpthread && ./main && rm ./main

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 3

void *printCharacter(void* characterParameter) {

  char symbol = *(char*) characterParameter;

  for(int i = 0; i < 3; i++) {
    printf("Una hebra escribe el caracter: %c\n", symbol) ;
  }

  return NULL;
}

int main(int argc, char *argv) {

  pthread_t threads[NUM_THREADS];
  char caracteres[3] = {'1', '2', '3'};


  for (int i = 0; i < NUM_THREADS; i++) {

    printf("Se crea la hebra %d\n", i + 1);
    int rc = pthread_create(&threads[i], NULL, printCharacter, &caracteres[i]);

    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }

  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
}