//gcc -o main ejemplo.c -lpthread && ./main && rm ./main

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#define NUM_THREADS 5

void *PrintHello(void *threadid) {

  long tid;
  tid = *(long*)threadid;
  printf("Hello World! It's me, thread #%ld!\n", tid);

  return NULL;
}

int main(int argc, char *argv[]) {

  pthread_t threads[NUM_THREADS];
  int rc;
  long t;
  long id[NUM_THREADS];

  for(t = 0; t < NUM_THREADS; t++){
    id[t]=t;
    printf("In main: creating thread %ld\n", t);
    rc = pthread_create(&threads[t], NULL, PrintHello, &id[t]);
    if (rc){
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  for (int i = 0; i < NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }


}