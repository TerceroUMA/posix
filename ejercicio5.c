//gcc -o main ejercicio5.c -lpthread && ./main && rm ./main

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 4

int botellaBool = 1;
int pegamentoBool = 1;
int papelBool = 1;
int botellaBaseLista = 0;

pthread_mutex_t mutex; // Mutex para proteger la región crítica
pthread_cond_t esperarBotellaAcabada; // Un condición de espera
pthread_cond_t esperarBotellaEmpezada; // Un condición de espera
pthread_cond_t sePuedeMeterBotella; // Un condición de espera
pthread_cond_t sePuedeMeterPegamento; // Un condición de espera
pthread_cond_t sePuedeMeterPapel; // Un condición de espera

void *agente(void* idParam) {

  int id = *(int*) idParam;
  int turno = -1;
  while(1) {

    turno = (turno + 1) % 3;

    pthread_mutex_lock(&mutex);

    if (turno == 0) { // falta por poner la botella
      botellaBool = 0;
      pegamentoBool = 1;
      papelBool = 1;
      //pthread_cond_signal(&esperarBotellaEmpezada);
      pthread_cond_signal(&sePuedeMeterBotella);
    } else if (turno == 1) { // falta por poner el pegamento
    printf("hasd");
      botellaBool = 1;
      pegamentoBool = 0;
      papelBool = 1;
      //pthread_cond_signal(&esperarBotellaEmpezada);
      pthread_cond_signal(&sePuedeMeterPegamento);
    } else { // falta por poner el papel
      botellaBool = 1;
      pegamentoBool = 1;
      papelBool = 0;
      //pthread_cond_signal(&esperarBotellaEmpezada);
      pthread_cond_signal(&sePuedeMeterPapel);
    }
    botellaBaseLista = 1;

    printf("\nAgente crea botella base: \n\t- Botella: %d\n\t- Pegamento: %d\n\t- papel: %d\n\n", botellaBool, pegamentoBool, papelBool);

    while (botellaBool ==  0 || pegamentoBool == 0 || papelBool == 0) {
      pthread_cond_wait(&esperarBotellaAcabada, &mutex);
      printf("Botella acabada\n");
    }
    
    pthread_mutex_unlock(&mutex);

  }

  return NULL;
}

void *botella(void* idParam) {

  int id = *(int*) idParam;

  /* pthread_mutex_lock(&mutex);
  while(botellaBaseLista == 0) {
    pthread_cond_wait(&esperarBotellaEmpezada, &mutex);
  }
  pthread_mutex_unlock(&mutex); */

  while (1) {
    pthread_mutex_lock(&mutex);

    while(botellaBool == 1) {
      pthread_cond_wait(&sePuedeMeterBotella, &mutex);
    }

    printf("Se añade la botella\n");
    botellaBool = 1;

    pthread_cond_signal(&esperarBotellaAcabada);

    pthread_mutex_unlock(&mutex);
  }
  

  return NULL;
}

void *papel(void* idParam) {

  int id = *(int*) idParam;

  while (1) {
    pthread_mutex_lock(&mutex);

    while(papelBool == 1) {
      pthread_cond_wait(&sePuedeMeterPapel, &mutex);
    }

    printf("Se añade el papel\n");
    papelBool = 1;

    pthread_cond_signal(&esperarBotellaAcabada);

    pthread_mutex_unlock(&mutex);
  }
  

  return NULL;
}

void *pegamento(void* idParam) {

  int id = *(int*) idParam;

  while (1) {
    pthread_mutex_lock(&mutex);

    while(pegamentoBool == 1) {
      pthread_cond_wait(&sePuedeMeterPegamento, &mutex);
    }

    printf("Se añade el pegamento\n");
    pegamentoBool = 1;

    pthread_cond_signal(&esperarBotellaAcabada);

    pthread_mutex_unlock(&mutex);
  }
  

  return NULL;
}

int main(int argc, char *argv) {

  pthread_mutex_init(&mutex,NULL);
  pthread_cond_init(&esperarBotellaAcabada, NULL);
  pthread_cond_init(&esperarBotellaEmpezada, NULL);
  pthread_cond_init(&sePuedeMeterBotella, NULL);
  pthread_cond_init(&sePuedeMeterPegamento, NULL);
  pthread_cond_init(&sePuedeMeterPapel, NULL);

  pthread_t threads[NUM_THREADS];
  long id[NUM_THREADS];

  id[0] = 0;
  id[1] = 1;
  id[2] = 2;
  id[3] = 3;

  printf("Se crea la hebra agente %d\n", 0);
  pthread_create(&threads[0], NULL, agente, &id[0]);

  printf("Se crea la hebra %d\n", 1);
  pthread_create(&threads[1], NULL, botella, &id[1]);

  printf("Se crea la hebra %d\n", 2);
  pthread_create(&threads[2], NULL, papel, &id[2]);

  printf("Se crea la hebra %d\n", 3);
  pthread_create(&threads[3], NULL, pegamento, &id[3]);

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
}