//gcc -o main ejercicio2.c -pthread -lpthread -lrt && sudo taskset -c 0 ./main && rm ./main

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define NUM_ITER 40

#define PERIODO_A_SEC 90
#define PERIODO_A_NSEC 200000000
#define PRIORIDAD_A 24
#define INC_A 100

#define PERIODO_B_SEC 100
#define PERIODO_B_NSEC 300000000
#define PRIORIDAD_B 26
#define INC_B 1

struct argumentosStruct {
  pthread_mutex_t mutex;
  int contador;
};

void espera_activa ( time_t seg ) {

  volatile time_t t = time (0) + seg ;
  while ( time (0) < t);

}

const char * get_time ( char * buf) {
  time_t t = time (0);
  char * f = ctime_r (&t, buf );
  f[strlen(f) - 1] = '\0';
  return f;
}

void addtime ( struct timespec * tm , const struct timespec * val ) {
  tm-> tv_sec += val -> tv_sec ;
  tm-> tv_nsec += val -> tv_nsec ;
  if (tm-> tv_nsec >= 1000000000L) {
    tm-> tv_sec += (tm-> tv_nsec / 1000000000L);
    tm-> tv_nsec = (tm-> tv_nsec % 1000000000L);
  }
}

void *tareaA(void* arg) {

  struct argumentosStruct* argumentos = (struct argumentosStruct*) arg;
  struct timespec next; //? Varibale con la siguiente ejecución
  char buf [30];

  // Periodo con el que se ejecutará la tarea
  const struct timespec periodo = { PERIODO_A_SEC , PERIODO_A_NSEC };

  clock_gettime ( CLOCK_MONOTONIC , & next ); // Actualiza next con el valor de tiempo actual

  printf (" Tarea A [%s]\n", get_time (buf));
  
  while (1) {

    addtime (&next , & periodo ); // Actualizamos next, la próxima vez q se llegue a clock_nanosleep se suspenderá

    // La hebra se queda suspendida hasta que el tiempo actual sea mayor que el nex, solo se suspende si actual es <= next
    clock_nanosleep ( CLOCK_MONOTONIC , TIMER_ABSTIME , &next , NULL );
    
    printf (" Tarea A [%s]\n", get_time (buf));
    
    for(int i = 0; i < NUM_ITER; i++) {
    
      pthread_mutex_lock(&argumentos -> mutex );

      argumentos -> contador += INC_A ;
      printf (" Tarea A: %d\n", argumentos -> contador);

      pthread_mutex_unlock(&argumentos -> mutex);
      espera_activa (1);  
    }
  }
  

  return NULL;
}

void *tareaB(void* arg) {

  struct argumentosStruct* argumentos = (struct argumentosStruct*) arg;
  struct timespec next; //? Varibale con la siguiente ejecución
  char buf [30];

  // Periodo con el que se ejecutará la tarea
  const struct timespec periodo = { PERIODO_B_SEC , PERIODO_B_NSEC };

  clock_gettime ( CLOCK_MONOTONIC , &next ); // Actualiza next con el valor de tiempo actual

  printf ("Tarea B [%s]\n", get_time (buf));
  
  
  while (1) {

    addtime (&next , & periodo ); // Actualizamos next, la próxima vez q se llegue a clock_nanosleep se suspenderá

    // La hebra se queda suspendida hasta que el tiempo actual sea mayor que el nex, solo se suspende si actual es <= next
    clock_nanosleep ( CLOCK_MONOTONIC , TIMER_ABSTIME , &next , NULL );
    
    printf (" Tarea B [%s]\n", get_time (buf));
    
    for(int i = 0; i < NUM_ITER; i++) {
    
      pthread_mutex_lock(&argumentos -> mutex );

      argumentos-> contador += INC_B ;
      printf (" Tarea B: %d\n", argumentos -> contador);

      pthread_mutex_unlock(&argumentos-> mutex);
      espera_activa (1);  
    }
  }
  

  return NULL;
}

int main(int argc, char *argv) {

  mlockall(MCL_CURRENT | MCL_FUTURE); // Bloque la memoria para que no se le vaya los datos con la paginación

  // Creo mi struct para pasar datos
  struct argumentosStruct argumentos;
  argumentos.contador = 0;

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
  paramA.sched_priority = PRIORIDAD_A;
  paramB.sched_priority = PRIORIDAD_B;

  // Inicializo los atributos
  pthread_attr_t attr;
  pthread_attr_init(&attr); // Inicializamos los atributos
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED); // El segundo parámetro dice que le vamos escribir la prioridad de forma explicita

  pthread_t tA, tB;

  // Decimos el tipo de política q vamos a usar para la hebra tarea A
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  pthread_attr_setschedparam(&attr, &paramA);

  int rc = pthread_create(&tA, &attr, tareaA, &argumentos);
  if (rc) {
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }

  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  pthread_attr_setschedparam(&attr, &paramB);

  rc = pthread_create(&tB, &attr, tareaB, &argumentos);
  if (rc) {
    printf("ERROR; return code from pthread_create() is %d\n", rc);
    exit(-1);
  }

  pthread_join(tA, NULL);
  pthread_join(tB, NULL);

  printf("\n\nFin del programa.\n\n"); fflush(stdout);

  pthread_attr_destroy(&attr);

  return 0;
}