//gcc -o main ejercicio1.c -lpthread && ./main && rm ./main
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define NUM_APARCAMIENTOS 3
#define NUM_PROCESOS 6

bool[] permisoPedido;
bool[] permisoDado;

int aparcamientosOcupados = 0;

pthread_mutex_t mutex;
pthread_cond_t noPuedePasar;

/**
 * @brief Comportamiento de los coches que quieren entrar en el aparcamiento
 *
 * @param idParam ide de la hebra
 * @return void*
 */
void *coches(void* idParam) {

  int id = *(int*) idParam;

  while(1) {

    pthread_mutex_lock(&mutex);

    while(!permisoDado[id] || aparcamientosOcupados < NUM_APARCAMIENTOS) {
      pthread_cond_wait(&noPuedePasar, &mutex);
    }

    pthread_cond_broadcast(&noPuedePasar);
    pthread_mutex_unlock(&mutex);
  }
}

int main() {

  pthread_t threads[NUM_PROCESOS];
  pthread_cond_init(&noPuedePasar, NULL);

  sysset_t set;
  sysemptyset(&set); //Limpia el conjunto
  sysaddset(&set, SIGRTMIN); //Añade una señal al conjunto
  pthread_sigmask(SIG_BLOCK, &set, NULL);

  // Crea las hebras
  for (int i = 0; i < NUM_PROCESOS; i++) {

    pthread_mutex_init(&mutex,NULL);


     if (i == 0) {

      int rc = pthread_create(&threads[i], NULL, NULL, &i);

      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      } else {
        printf("Se crea la hebra controladora: %d", i);
      }

    } else {

      int rc = pthread_create(&threads[i], NULL, NULL, &i);

      if (rc) {
        printf("ERROR; return code from pthread_create() is %d\n", rc);
        exit(-1);
      } else {
        printf("Se crea la hebra de coches: %d", i);
      }
    }
  }

  //Esperamos a que acaben los join
  for (int i = 0; i < NUM_PROCESOS; i++) {
    pthread_join(threads[i], NULL);
  }

}
