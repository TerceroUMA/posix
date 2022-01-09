//gcc -o main ejercicio2.c -lpthread && ./main && rm ./main
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h> // Para el sleep
#include<semaphore.h>

#define NUM_APARCAMIENTOS 3
#define NUM_POR_TIPO 3
#define NUM_PROCESOS 7


sem_t espera, cola;
sigset_t set;

pthread_t threads[NUM_PROCESOS];

/**
 * @brief Comportamiento de los coches que quieren entrar en el aparcamiento
 * 
 * @param idParam ide de la hebra
 * @return void* 
 */
void *coches(void* idParam) {

  int id = *(int*) idParam;

  while(1) {

    printf("Llegando al parking coche: %d\n",id); fflush(stdout);
    sem_wait(&cola); // El coche llega a la cola y espera a su turno

    printf("Coche %d quiere sacar un ticket\n",id); fflush(stdout);
    // Alternativa -> kill(getpid(), señal)
    pthread_kill(threads[0], SIGRTMAX);
    sem_wait(&espera); // Espera respuesta del controlador (si está el parking lleno, hasta que salga algún coche). Como inicialemnte este semáforo es 0, se bloqueará la hebra hasta que le den permiso para entrar
    printf("El coche %d entra y aparca\n", id);
    sem_post(&cola); // El coche ha terminado de entrar al parling y cualquier otro proceso que estuviese esperando puede pasar ahora
    
    sleep(5); // Al rato saldrá
    printf("El coche %d sale del parking\n", id);
    pthread_kill(threads[0], SIGRTMIN);
    sleep(3); // Espera y luego vuelve al parking
  }
}

/**
 * @brief Comportamiento de los camiones que quieren entrar en el aparcamiento
 * 
 * @param idParam ide de la hebra
 * @return void* 
 */
void *camionesFunc(void* idParam) {

  int id = *(int*) idParam;

  while(1) {

    printf("Llegando al parking camion: %d\n",id); fflush(stdout);
    sem_wait(&cola); // El camion llega a la cola y espera a su turno

    printf("Camión %d quiere sacar un ticket\n",id); fflush(stdout);
    // Alternativa -> kill(getpid(), señal)
    pthread_kill(threads[0], SIGRTMAX - 1);
    sem_wait(&espera); // Espera respuesta del controlador (si está el parking lleno, hasta que salga algún camion). Como inicialemnte este semáforo es 0, se bloqueará la hebra hasta que le den permiso para entrar
    printf("El camion %d entra y aparca\n", id);
    sem_post(&cola); // El camion ha terminado de entrar al parling y cualquier otro proceso que estuviese esperando puede pasar ahora
    
    sleep(5); // Al rato saldrá
    printf("El camion %d sale del parking\n", id);
    pthread_kill(threads[0], SIGRTMIN + 1);
    sleep(3); // Espera y luego vuelve al parking
  }
}

void *manejadorHebra(void* idParam) {
  
  int info;
  int aparcamientosOcupados = 0;
  int esperando = 0; // Indica si hay gente esperando porque el parking está lleno

	while(1) {

    // Espera a que le envien la señal, cuando le llegue avanzará
    sigwait(&set, &info);

    // Quieren entrar y hay plazas disponibles
    if (info == SIGRTMAX && aparcamientosOcupados < NUM_APARCAMIENTOS) {
      
      aparcamientosOcupados++;
      sem_post(&espera); // El coche que esperaba a su ticket lo recibe y podrá entrar

    // Quieren entrar y no hay plazas disponibles
    } else if(info == SIGRTMAX) {

      esperando = 1; // Indica que hay coches esperando porque el parking está lleno
      printf("Parking lleno\n"); fflush(stdout);
      
    } else if (info == SIGRTMAX - 1 && aparcamientosOcupados < NUM_APARCAMIENTOS) {
      aparcamientosOcupados += 2;
      sem_post(&espera); // El camion que esperaba a su ticket lo recibe y podrá entrar
    } else if (info == SIGRTMAX - 1) {
      esperando = 1; // Indica que hay camiones esperando porque el parking está lleno
      printf("Parking lleno\n"); fflush(stdout);
    }

    // Quieren salir
    if (info == SIGRTMIN) {

      aparcamientosOcupados--;

      // Si había un coche esperando entonces ya no habrá ninguno, se le dará un ticket al coche y habrá una plaza menos
      if (esperando) {
        esperando = 0;
        aparcamientosOcupados++;
        sem_post(&espera);
      }
    }

    if (info == SIGRTMIN + 1) {

      aparcamientosOcupados--;

      // Si había un camion esperando entonces ya no habrá ninguno, se le dará un ticket al camion y habrá una plaza menos
      if (esperando) {
        esperando = 0;
        aparcamientosOcupados += 2;
        sem_post(&espera);
      }
    }
  }
	
  return NULL;

}

int main() {

  sigemptyset(&set); //Limpia el conjunto
  sigaddset(&set, SIGRTMIN); //Añade una señal al conjunto
  sigaddset(&set, SIGRTMIN + 1); //Añade una señal al conjunto
  sigaddset(&set, SIGRTMAX); //Añade una señal al conjunto
  sigaddset(&set, SIGRTMAX - 1); //Añade una señal al conjunto
  pthread_sigmask(SIG_BLOCK, &set, NULL);

  int coche[NUM_POR_TIPO];
  int camiones[NUM_POR_TIPO];


  sem_init(&cola, 0, 1); // Solo podrá haber un coche en la cola
  sem_init(&espera, 0, 0); // Cuando se pide un ticket se debe de bloquear hasata q le den el ticket (desbloqueo)

  // Crea las hebras
  for (int i = 0; i < NUM_PROCESOS; i++) {
    
    if (i == 0) {

      int rc = pthread_create(&threads[i], NULL, manejadorHebra, &threads[i]);

      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      } else {
        printf("Se crea la hebra controladora: %d\n", i);
      }

    } else {

      if (i % 2 == 0) {

        coche[i]=i;
        pthread_create(&threads[i], NULL, coches, &coche[i]);
        
      } else {

        camiones[i]=i;
        pthread_create(&threads[i], NULL, camionesFunc, &camiones[i]);
      }

    }
        fflush(stdout);
  }

  //Esperamos a que acaben los join
  for (int i = 0; i < NUM_PROCESOS; i++) {
    pthread_join(threads[i], NULL);
  }
  
}