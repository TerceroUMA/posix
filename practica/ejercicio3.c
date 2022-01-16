//gcc -o main ejercicio3.c -pthread -lpthread -lrt && sudo taskset -c 0 ./main && rm ./main
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

# define INACTIVO 0
# define ACTIVO 1

# define PERIODO_TMP_SEC 1
# define PERIODO_TMP_NSEC 500000000
# define PRIORIDAD_TMP 22
# define TMP_UMBRAL_SUP 100
# define TMP_UMBRAL_INF 90

# define PERIODO_STMP_SEC 1
# define PERIODO_STMP_NSEC 400000000
# define PRIORIDAD_STMP 24
# define STMP_VALOR_INI 80
# define STMP_INC 1
# define STMP_DEC 2

# define PERIODO_PRS_SEC 1
# define PERIODO_PRS_NSEC 350000000
# define PRIORIDAD_PRS 26
# define PRS_UMBRAL_SUP 1000
# define PRS_UMBRAL_INF 900

# define PERIODO_SPRS_SEC 1
# define PERIODO_SPRS_NSEC 350000000
# define PRIORIDAD_SPRS 28
# define SPRS_VALOR_INI 800
# define SPRS_INC 10
# define SPRS_DEC 20

# define PERIODO_MTR_SEC 1
# define PERIODO_MTR_NSEC 0
# define PRIORIDAD_MTR 20

struct Data_Tmp {
  pthread_mutex_t mutex ;
  int estado ;
  int val ;
};

struct Data_Prs {
  pthread_mutex_t mutex ;
  int estado ;
  int val ;
};
struct Data_Mtr {
  struct Data_Tmp tmp ;
  struct Data_Prs prs ;
};

pthread_mutex_t mutexTemp, mutexPres;


void espera_activa(time_t seg) {

  volatile time_t t = time(0) + seg;
  while (time(0) < t);
  
}

void addTime(struct timespec* tm, const struct timespec* periodo) {

  tm -> tv_sec   += periodo -> tv_sec;
  tm -> tv_nsec  += periodo -> tv_nsec;

  if (tm -> tv_nsec >= 1000000000L) {
    tm -> tv_sec += (tm -> tv_sec / 1000000000L);
    tm -> tv_nsec = (tm -> tv_nsec % 1000000000L);
  }
}

void* pres_func(void* arg) {

  struct Data_Prs* data = arg;
  const struct timespec periodo = { PERIODO_PRS_SEC, PERIODO_PRS_NSEC };
  struct timespec next;

  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1) {

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    addTime(&next, &periodo);
    pthread_mutex_lock(&data -> mutex);

    switch (data -> estado) {
        
      case INACTIVO:

        if (data -> val > PRS_UMBRAL_SUP) {

          data -> estado = ACTIVO;
          printf ("Abrir válvula de presión \n");

        }

        break;

      case ACTIVO:
        
        if (data -> val < PRS_UMBRAL_INF) {

          data -> estado = INACTIVO;
          printf ("Cerrar válvula de presión \n");
        }

        break;
    
    }

    pthread_mutex_unlock(&data -> mutex);

  }

  return NULL;

}

void* sensor_pres_func(void* arg) {

  struct Data_Prs* data = arg;
  const struct timespec periodo = { PERIODO_SPRS_SEC, PERIODO_SPRS_NSEC };
  struct timespec next;

  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1) {

    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    addTime(&next, &periodo);
    pthread_mutex_lock(&data -> mutex);

    switch (data -> estado) {
        
      case INACTIVO:

          data -> val += SPRS_INC;
          printf ("Se aumenta la presión a %d\n", data -> val);
        break;

      case ACTIVO:
        
          data -> val -= SPRS_DEC;
          printf ("Se reduce la presión a %d\n", data -> val);
        break;
    
    }

    pthread_mutex_unlock(&data -> mutex);

  }

  return NULL;

}

void* temp_func(void* arg) {

  struct Data_Tmp* data = arg;
  const struct timespec periodo = { PERIODO_TMP_SEC, PERIODO_TMP_NSEC };
  struct timespec next;

  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1) {
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    addTime(&next, &periodo);
    pthread_mutex_lock(&data -> mutex);

    switch (data -> estado) {
        
      case INACTIVO:

        if (data -> val > TMP_UMBRAL_SUP) {

          data -> estado = ACTIVO;
          printf ("Activar inyección de aire frío \n");

        }

        break;

      case ACTIVO:
        
        if (data -> val < TMP_UMBRAL_INF) {

          data -> estado = INACTIVO;
          printf ("Desactivar inyección de aire frío \n");
        }

        break;
    
    }

    pthread_mutex_unlock(&data -> mutex);

  }

  return NULL;

}

void* sensor_temp_func(void* arg) {

  struct Data_Tmp* data = arg;
  const struct timespec periodo = { PERIODO_STMP_SEC, PERIODO_STMP_NSEC };
  struct timespec next;

  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1) {
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    addTime(&next, &periodo);
    pthread_mutex_lock(&data -> mutex);

    switch (data -> estado) {
        
      case INACTIVO:

          data -> val += STMP_INC;
          printf ("Se aumenta la temperatura a %d\n", data -> val);
        break;

      case ACTIVO:
        
          data -> val -= STMP_DEC;
          printf ("Se reduce la temperatura a %d\n", data -> val);
        break;
    }

    pthread_mutex_unlock(&data -> mutex);

  }

  return NULL;

}

void* monitor_funcion (void* arg ) {

  struct Data_Mtr* data = arg;

  const struct timespec periodo = { PERIODO_MTR_SEC, PERIODO_MTR_NSEC };
  struct timespec next;

  clock_gettime(CLOCK_MONOTONIC, &next);

  while(1) {
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next, NULL);
    addTime(&next, &periodo);

    pthread_mutex_lock(&data -> tmp.mutex);
    pthread_mutex_lock(&data -> prs.mutex);

    printf ("Temperatura : %d %s Presión : %d %s\n",
      data -> tmp.val , ( data ->tmp. estado == INACTIVO ? "++" : " --") ,
      data -> prs.val , ( data ->prs. estado == INACTIVO ? "++" : " --" ));

    pthread_mutex_unlock(&data -> tmp.mutex);
    pthread_mutex_unlock(&data -> prs.mutex);
  }

  return NULL;
}

int main() {

  malloc(MCL_CURRENT | MCL_FUTURE);

  pthread_t monitor, controladorTemperaturaHebra, controladorPresionHebra, sensorTemperaturaHebra, sensorPresionHebra;

  /* ------------- CREAR STRUCTS ------------- */
  struct Data_Prs data_prs;
  data_prs.estado = INACTIVO;
  data_prs.val = SPRS_VALOR_INI;
  data_prs.mutex = mutexPres;

  struct Data_Tmp data_tmp;
  data_tmp.estado = INACTIVO;
  data_tmp.val = STMP_VALOR_INI;
  data_tmp.mutex = mutexTemp;

  struct Data_Mtr data;
  data.prs = data_prs;
  data.tmp = data_tmp;

  pthread_mutex_init(&data.prs.mutex, NULL);
  pthread_mutex_init(&data.tmp.mutex, NULL);
  /* ----------------------------------------- */

  /* ------------- PRIORIDAD MAIN ------------- */
  pthread_t myMain = pthread_self();
  int policy = 0;
  struct sched_param param;
  pthread_getschedparam(myMain, &policy, &param);
  param.sched_priority = 100;
  pthread_setschedparam(myMain, policy, &param);
  /* ----------------------------------------- */

  /* ------------- PRIORIDAD HEBRAS ------------- */
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy (&attr, SCHED_FIFO);

  param.sched_priority = PRIORIDAD_MTR;
  pthread_attr_setschedparam(&attr, &param);
  param.sched_priority = PRIORIDAD_PRS;
  pthread_attr_setschedparam(&attr, &param);
  param.sched_priority = PRIORIDAD_SPRS;
  pthread_attr_setschedparam(&attr, &param);
  param.sched_priority = PRIORIDAD_TMP;
  pthread_attr_setschedparam(&attr, &param);
  param.sched_priority = PRIORIDAD_STMP;
  pthread_attr_setschedparam(&attr, &param);
  /* ----------------------------------------- */

  /* ------------- CREAR HEBRAS ------------- */
  pthread_create(&monitor, &attr, monitor_funcion, &data);
  pthread_create(&controladorPresionHebra, &attr, pres_func, &data.prs);
  pthread_create(&controladorTemperaturaHebra, &attr, temp_func, &data.tmp);
  pthread_create(&sensorPresionHebra, &attr, sensor_pres_func, &data.prs);
  pthread_create(&sensorTemperaturaHebra, &attr, sensor_temp_func, &data.tmp);

  pthread_join(monitor, NULL);
  pthread_join(controladorPresionHebra, NULL);
  pthread_join(controladorTemperaturaHebra, NULL);
  pthread_join(sensorPresionHebra, NULL);
  pthread_join(sensorTemperaturaHebra, NULL);
  /* ----------------------------------------- */
}