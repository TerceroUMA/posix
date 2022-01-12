//gcc -o main ejercicio.c -lpthread -lrt && ./main && rm ./main 
//gcc -o main ejercicio.c -lpthread -lrt
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <time.h>
#include <stdio.h>


//Variables globales
#define MAX 10

int contador = 0;

// semaforo para controlar que el bufer no esta lleno
sem_t hay_espacio, productorSem, consumidorSem;

// semaforo para controlar que el buffer no esta vacio
sem_t hay_datos;

// semaforo para controlar el acceso al buffer en exclusion mutua
sem_t mutex;

timer_t timer2;
int signum2; /* señal recibida */
sigset_t set; /* señales a las que se espera */

void *productorFunc(void *argg ) {
  printf("hola productor \n");


  while (1) {

    // Esperamos el temporizador nos avise para entrar. Se bloquea siempre hasta que el temporizador avise
    sem_wait(&productorSem);

    sem_wait(&mutex);

    if (contador < MAX) {
      contador++;
      printf("Se ha producido un dato\n");
    } else {
      printf("Buffer lleno\n");
    }

    sem_post(&mutex);
  }
  
  return NULL;

}

void *consumidorFunc(void *argg ) {

  printf("hola consumidor\n");

  while (1) {

    // Esperamos el temporizador nos avise para entrar. Se bloquea siempre hasta que el temporizador avise
    sem_wait(&consumidorSem);

    sem_wait(&mutex);

    if (contador > 0) {
      contador--;
      printf("Se ha producido un dato\n");
    } else {
      printf("Buffer vacío\n");
    }

    sem_post(&mutex);
  }

  return NULL;

}

void *controladorFunc(void *argg ) {

  printf("hola controlador\n");

  // Creamos las varibales del timer necesarias
  struct itimerspec required, old;
	struct timespec first, period; // Creo que contiene la info de la hora actual
	struct sigevent sig; /* información de señal */
  int signum; /* señal recibida */
	timer_t timer;

	sig.sigev_notify = SIGEV_SIGNAL; //?
	sig.sigev_signo = SIGALRM; //?
	sig.sigev_value.sival_ptr = &timer; //?

  if (clock_gettime (CLOCK_MONOTONIC, &first) != 0) perror("\nclock_gettime\n");
	if (timer_create(CLOCK_MONOTONIC,&sig,&timer) != 0) perror("\ntimer_create\n");
  if (timer_settime(timer,0, &required, &old) != 0) perror ("\ntimer_settime\n");

  int i = -1;
  while(1) {

    printf("s");

    if (sigwait(&set, &signum) != 0) perror("Error sigwait");

    i++;
    if (i == 2) {
      sem_post(&consumidorSem);
    }
    sem_post(&productorSem);
    i = i % 2;

  }

  return NULL;
}

int main () {

  //Creamos variables
  pthread_t consumidor, productor, controlador;
  pthread_attr_t attr;

  // Inicializamos semáforos
  sem_init(&hay_espacio, 0, MAX);
  sem_init(&productorSem, 0, 0);
  sem_init(&consumidorSem, 0, 0);
  sem_init(&hay_datos, 0, 0);
  sem_init(&mutex, 0, 1);
  pthread_attr_init(&attr);

  if (sigemptyset(&set) != 0) perror("\nasdn\n");
	if (sigaddset(&set, SIGALRM) != 0) perror("\nasdn\n");
  pthread_sigmask(SIG_BLOCK,&set,NULL);

  // Creamos las hebras
  pthread_create(&consumidor, &attr, consumidorFunc, NULL);
  pthread_create(&productor, &attr, productorFunc, NULL);
  pthread_create(&controlador, &attr, controladorFunc, NULL);

  pthread_join(consumidor, NULL);
  pthread_join(productor, NULL);
  pthread_join(controlador, NULL);
  printf("f"); fflush(stdout);

  return 0;

}

