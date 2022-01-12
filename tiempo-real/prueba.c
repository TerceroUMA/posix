#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <time.h>

#include <stdio.h>


#define MAXT 10

long Buffer[MAXT];
long cont=0;

timer_t timer2;
int signum2; /* se�al recibida */
sigset_t set; /* se�ales a las que se espera */

sem_t Hay_espacio, prod, cons;
sem_t Hay_Datos;
sem_t mutex;



void *Productor(void *argg ){
  int i=0;

  int dato=0;

  while (1) {
  sem_wait(&prod);
    sem_wait(&Hay_espacio);

    sem_wait(&mutex);
    Buffer[i]=dato;
    dato=(dato+1) % 10000;
    i=(i+1)%MAXT;
    sem_post(&mutex);

    sem_post(&Hay_Datos);		

  }
}


void *Consumidor(void *argg ){
	int i=0;

	int dato=0;

	while (1) {
		sem_wait(&cons);
		sem_wait(&Hay_Datos);

		sem_wait(&mutex);
		dato=Buffer[i];
		i=(i+1)%MAXT;
		sem_post(&mutex);

		sem_post(&Hay_espacio);		

		printf("Dato Consumido =%d\n",dato);

	}
}


void *periodic (void *arg) {

	struct itimerspec required, old;
	struct timespec first, period;
	struct sigevent sig; /* informaci�n de se�al */

	int signum; /* se�al recibida */
	timer_t timer;
	sig.sigev_notify = SIGEV_SIGNAL;
	sig.sigev_signo = SIGALRM;
	sig.sigev_value.sival_ptr = &timer;


	first.tv_sec = first.tv_sec + 3;
	period.tv_sec = 1;
	period.tv_nsec = 0; /* 10 ms */
	required.it_value = first;
	required.it_interval = period;

	if (clock_gettime (CLOCK_MONOTONIC, &first) != 0) error();
	if (timer_create(CLOCK_MONOTONIC,&sig,&timer) != 0) error();


	if (timer_settime(timer,0, &required, &old) != 0) 
		error ();

	while (1) {
	 	printf("AAAAAAAAAAAAAAA\n");
		if (sigwait(&set, &signum) != 0) error();
		cont++;
		sem_post(&prod);
		if ((cont %3)==0) sem_post(&cons);

	}
}


int main () {

  pthread_t th1,th2;
  pthread_attr_t attr;


	sem_init(&Hay_espacio,0,MAXT);
	sem_init(&Hay_Datos,0,0);
	sem_init(&mutex,0,1);
	sem_init(&prod,0,1);
	sem_init(&cons,0,1);
	pthread_attr_init(&attr);

printf("4a\n");
	if (sigemptyset(&set) != 0) error ();

	if (sigaddset(&set, SIGALRM) != 0) error();
	

  	pthread_sigmask(SIG_BLOCK,&set,NULL);

printf("4w\n");


       pthread_create(&th1,&attr,periodic,NULL);
        pthread_create(&th1,&attr,Productor,NULL);
        pthread_create(&th2,&attr,Consumidor,NULL);


       pthread_join (th1,NULL);

	printf("Fin\n");

        return 0;
}