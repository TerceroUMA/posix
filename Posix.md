- [Posix](#posix)
  - [Mutex](#mutex)
    - [Semáforos](#semáforos)
    - [Regiones críticas](#regiones-críticas)
    - [Spinlocks](#spinlocks)
  - [Panificación procesos](#panificación-procesos)
  - [Memoria virtual](#memoria-virtual)
  - [Señales de tiempo real](#señales-de-tiempo-real)
    - [Ejemplo](#ejemplo)

::: warning
*here be dragons*
:::

:bowtie:
`:bowtie:`
$\alef$

# Posix
## Mutex 

### Semáforos

Iniciar un semáforo. Primer parámetro es el semáforo de tipo `sem_t`, el segundo indica si es local al proceso actual (pshared es cero) o si es compartido entre procesos (diferente de 0), nosotros lo pondremos a 0 y el último es con cuanto queremos inicializarlo:

```c
int sem_init (sem_t *sem, int pshared, unsigned int value)
```

Para agarrar un hueco del semáforo:
```c
int sem_wait (sem_t * sem)
int sem_trywait (sem_t * sem)
```
Funciones que decrementan el valor del semáforo en 1. Si el valor del semáforo es 0 la hebra se bloquea y espera a que el semáforo sea incrementado por otra hebra. En el caso de sem_trywait la hebra no se bloqueará.


Para liberar un hueco del semáforo:
```c
int sem_post (sem_t * sem)
```
Si otras hebras están esperando en el semáforo uno de ellos se despertará.

Para devolver el valor actual del semáforo:
```c
int sem_getvalue (sem_t * sem, int * sval)
```
Cuidado!! El valor puede haber cambiado cuando termine getvalue.

### Regiones críticas

Tenemos varias funciones para manejar regiones críticas.
```c
//------------ Variables Globales -----------------
// El mutex se inicializa a desbloqueado
pthread_mutex_t mutex;

// Crea la región crítica
int pthread_mutex_init(
  pthread_mutex_t *restrict mutex,
  const pthread_mutexattr_t *restrict attr // Si es null, atributos por defecto
);

// Destruye la región crítica
int pthread_mutex_destroy(pthread_mutex_t *mutex); 

// Creación y destrucción de los atributos de mutex
// Hay 3 atributos posibles: 
// – Protocolo: Para evitar la inversión de prioridades (trasparencia 95)
// – Prioceiling: Especifica el techo de prioridad de un semáforo (trasparencia 95)
// – Process-shared: Especifica si los procesos comparten el mutex. */
int pthread_mutexattr_init(pthread_mutexattr_t *attr);
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr);

// Las dos funciones de abajo sirven para coger los mutex
// Trylock Intenta tomar un mutex. Si ya está bloqueado se devuelve un código de error EBUSY
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_trylock(pthread_mutex_t *mutex);

// Libera la región crítica
// – Si el mutex ya está desbloqueado
// – Si está tomado por otra hebra
int pthread_mutex_unlock(pthread_mutex_t *mutex);
```

```c
// EJEMPLO
#include <pthread.h>
#include <stdio.h>

// Variable global
pthread_mutex_t mutex;

int main () {

  // Definimos los threads
  pthread_t th1,th2,th3,th4;

  // Creamos el mutex
  pthread_mutex_init(&mutex,NULL);

  // Se crean hebras
  // Se hace join a las hebras
}

// Región crítica en una función
void IncVsitantes() {
  pthread_mutex_lock(&mutex);
  Visitantes=Visitantes+1;
  pthread_mutex_unlock(&mutex);
}

// Función hebras
void *HebraVisitas(void *argg ){
  for (i=0;i<*NumVisitas;i++) {
    IncVsitantes();
  }
}
```

### Spinlocks

Cuando una hebra intenta toma un spinlock y éste ya está tomado por otra hebra tendrá que esperar en un bucle comprobando cuando se queda libre. Se pueden implementar usando variables condición y mutexes. Las condiciones serán mas como etiquetas o colas a las que se añaden hebras.

```c
// Creamos el spinlock
int pthread_spin_init(pthread_spinlock_t *lock, int pshared);

// Cogemos el spinlock
int pthread_spin_lock(pthread_spinlock_t *lock);

// Intentamos coget el spinlock
int pthread_spin_trylock(pthread_spinlock_t *lock);

// Liberamos el spinlock
int pthread_spin_unlock(pthread_spinlock_t *lock);

// Inicializa una condición
int pthread_cond_init(
  pthread_cond_t *restrict cond,
  const pthread_condattr_t *restrict attr
);

// Poner valor en la condición
pthread_cond_t cond = PTHREAD_COND_INITIALIZER; 

// Destruye la condición
pthread_cond_destroy(pthread_cond_t *cond);

// Espera a que una condición sea cumplida (etiqueta)
int pthread_cond_wait(
  pthread_cond_t *restrict cond,
  pthread_mutex_t *restrict mutex
);

// Se libera la condición y se avisan a las hebras que están esperando
// ¿Diferencia entre una y otra? En diapos se usa signal
int pthread_cond_broadcast(pthread_cond_t *cond);
int pthread_cond_signal(pthread_cond_t *cond); 
```


```c
// EJEMPLO Buffer acotado

#define BUFF_SIZE 10

typedef struct {
  pthread_mutex_t mutex; // Mutex para proteger el buffer
  pthread_cond_t buffer_no_lleno; // Condición para esperar a que haya hueco
  pthread_cond_t buffer_no_vacio; // Condición para esperar a que haya dato
  int count, first, last; // Número de elementos en el buffer, índice del primer elemento y índice del último elemento
  int buf[BUFF_SIZE]; // Buffer (array)
} buffer;

// Una función sacar
int Sacar(int *item, buffer *B ) {

  // Tomar el mutex
  pthread_mutex_lock(&B->mutex);

  // Mientras no haya elementos en el buffer esperamos
  while(B->count == 0) {
    // Esperamos a que no haya nada en el buffer (señal de que hay hueco)
    pthread_cond_wait(&B->buffer_no_vacio, &B->mutex);
  }

  /* Tomar dato del buffer and actualizar count y first */
  // ...
  
  // Despertamos a las hebras que están esperando a que haya hueco
  pthread_cond_signal(&B->buffer_no_lleno);

  // Liberar el mutex
  pthread_mutex_unlock(&B->mutex);

  return 0;
}
```

Si con condiciones de espera, una hebra coge el mutex, pero se queda bloqueada dentro del while hasta que se cumpla la condición, entoncés como ¿podrá otra hebra coger el mutex?

## Panificación procesos

¿Que pasa cuando tenemos tareas de igual prioridad?

Tres modos:

* **SCHED_FIFO:** La primera que haya llegado entra, las demás se esperan
* **SCHED_RP:** Se van turnando el entrar al procesador si tienen la misma priorida.
* **SCHED_OTHER:** Definir tu tu propio esquema. El profe no la ha probado así que no creo que lo usemos.

```c
#include <sched.h>

//Establece la política de planificación y sus parámetros
int sched_setscheduler(pid_t pid, int policy, const struct sched_param
*param);

//Cambia los parámetros
int sched_getscheduler(pid_t pid);
int sched_setparam(pid_t pid, const struct sched_param *param);
int sched_getparam(pid_t pid, struct sched_param *param);

//Devuelven los límites de los parámetros de planificación
int sched_get_priority_max(int policy);
int sched_get_priority_min(int policy);

//Abandona el procesador
int sched_yield(void)
```

```c

// La signación de tareas se ponen con los atributos al crear la hebra. Esto se consigue con la función de abajo
int pthread_attr_setschedparam (pthread_attr_t *attr, const struct sched_param *param);


//Cambio dinámico de prioridad, pero normalmente trabajaremos con prioridades fijas
int pthread_setschedparam (pthread_t thread, int policy, const struct sched_param *param);


//Para que en linux funcione la planificación de las tareas en los atributos hay que poner también cierto atributo
// El segundo parámetro debe ser: PTHREAD_EXPLICIT_SCHED
int pthread_attr_setinheritsched (pthread_attr_t *attr, int inheritsched);

//Es posible que las hebras que implementemos tengan más prioridad que nuestro main que es el proceso principal. Para arreglarlo ponemos que nuestro main sea el proceso más prioritario, + ¿Cómo? - Sí 

```

La diapo 87 y 88 es un ejemplo a nivel de proceso y no de hebra, en las prácticas nos lo explicarán mejor.

## Memoria virtual

La memoria virtual introduce una fuente de impredecibilidad en las aplicaciones. Existen funciones en POSIX.4 para bloquear en memoria física los procesos críticos.

El recolector de basura es algo muy prioritario y caudno estamos con sistemas en tiempo real tener que esperar a que el recolector pase antes de acceder a una zona de memoria puede ser desastroso, por eso lo mejor sería bloquear zonas de memoria que nos interese.
```c
#include <sys/mman.h>
int mlock(const void *addr, size_t len); //Bloqueamos zonas de memoria
int munlock(const void *addr, size_t len); //Desbloqueamos zonas de memroias
int mlockall(int flags); //Creo que bloquea todas las posiciones de memoria. Flags lo usaremos en las prácticas. 
int munlockall(void);
```

Dioapositiva 90 es un ejemplo que bloquea pero por páginas de memoria.

Saltamos de la diapositiva 90 hastta la 98, lo que nos saltamos lo dará la semana que viene.

## Señales de tiempo real
Hay dos tipos de señales, una antigua de POSIX 1 y señales de tiempo real (la que usaremos).

Lo malo de las señales antiguas es que las señales no se encolaban si el proceso al que le llegaba no estaba preparado y entonces la señal podía perderse. En las de tiempo real se encolan y están ordenadas por prioridad.

Rango de señales van de SIGRTMIN a SIGRTMAX. Se referencia con SIGRTMIN + 1, SIGRTMIN + 2, etc. El número de señales entre el min y max depende de la máquina. 

Toda aquella señal que llega a mi proceso y que no tengo bloqueada termina el programa. Por ello necesito un conjunto de funciones que digan que cuando las señales me llegan no aborten el programa y que se encolen para atenderlas cuando quieras con algún manejador que hagamos. Esto se hace con el código de abajo.

```c
int sigemptyset(sigset_t *set); //Limpia el conjunto
int sigaddset(sigset_t *set, int signo); //Añade una señal al conjunto
```

Pero con las dos operaciones de arriba no estamos bloqueando las señales. El bloqueo se genera por:

```c
// How: SIG_BLOQ o SIG_UNBLOQ
// El segundo es el conjunto de señales.
// El tercero no se para que pero dice el profe que no le ve la utilidad, así que lo dejaremos a null
int pthread_sigmask(int how, const sigset_t *restrict set,sigset_t *restrict oset);

// Esta funcion es a nivel de procesos
int sigprocmask(int how, const sigset_t *restrict set,sigset_t *restrict oset);
```

Ahora hay que crear un manejador para las señales. Esto se hace con la función **sigaction.** Podemos:
* Ignorar la señal
* Asignar un manejador por defecto
* Asignar un manejador propio

```c
// 1: La señal a la que se le va a asignar la respuesta
// 2: Estructura donde definon el manejador asociado a la señal 
// 3: en ese parámetro se guarda la respuesta anterior, no le ve la utilidad
int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);

// POSIX 1, NO SE USA
struct sigaction {
  void (*sa_handler)(int);
  sigset_t sa_mask;
  int sa_flags;
};

// POSIX 4 (tiempo real), si se usa
struct sigaction {
  void (*sa_sigaction)(int, siginfo_t   *, void *); //Nombre el que sea, pero tiene que tener tres parámetros, ¿Que hace cada uno? Sí. Mira el puto man
  sigset_t sa_mask; // Permite añadir nuevas señales a las que me quiero bloqueat con la respuesta a la señal que estoy manejando ahora, no lo vamos a usa.
  int sa_flags; // Tiene que existir siempre para indicar que son manejadores de tiempo real (?
};
```

### Ejemplo

```c
main()
{
  struct sigaction act; // manejador señal
  sigset_t sigset; // conjunto de señales

  // Configura la señal

  // manejador de la señal
  act.sa_sigaction = ManejadorSig;

  // máscara vacía
  sigemptyset(&act.sa_mask);

  // Modo extendido
  // Los flags caracterizan las señales, las dos mas tipicas son las de abajo. La primera es la que identifica una señal de tiempo real, y el segundo es para proteger ciertas llamadas del sistema.
  act.sa_flags = SA_SIGINFO | SA_RESTART;

  // configuración

  if (sigaction(SIGRTMAX, &act, NULL)<0) {
    perror("Sigaction fallado");
  }

  sigemptyset(&sigset);
  sigaddset(&sigset, SIGRTMAX);

  while(1)   {
    // Espera indefinidamente la aparición de la señal

    // Desde la consola se puede enviar una señal al proceso
    // kill -s <SIGNUM> <PID>
    // La s indica que se espera un nombre (SIGRTMAX p.ej), si no se pone hay que enviar un número.
  }
}
```

Manejador:
```c
void ManejadorSig( int signo, siginfo_t *info, void *context) {
  printf("Soy el manejador de la señal # %d Valor: %d ",
  info->si_signo, info->si_value.sival_int);

  printf("Code: (%d) ",info->si_code);

  if( info->si_code == SI_USER)
  printf("SI_USER \n" );

  else if( info->si_code == SI_TIMER )
  printf("SI_TIMER \n" );

  else if( info->si_code == SI_QUEUE )
  printf("SI_QUEUE \n" );

  else if( info->si_code == SI_ASYNCIO )
  printf("SI_ASYNCIO \n" );

  else if( info->si_code == SI_MESGQ )
  printf("SI_MESGQ \n" );
}
```

Hasta ahora atendemos la señal según llegan, pero hay una forma de hacer que se espera hasta que me interese atenderla. Esto se hace con la función sigwait y sigwaitinfo.

```c
int sigwait(const sigset_t *set, int *sig);
int sigwaitinfo(const sigset_t *set, siginfo_t *info);
```

Ejemplo diapo 109 se queremos que una hebra maneje las señales.

```c
int main(int argc, char *argv[])
{
  sigset_t sigset;
   // conjunto de señales
  struct sigaction act;
   // manejador señal
  pthread_t th_id;
   // identificador para el thread
  pthread_attr_t attr;
   // atributos de los threads
  int senal;

  // bloquea la señal:
  // los threads creados posteriormente heredan la máscara
  sigemptyset(&sigset);

  sigaddset(&sigset, SIGRTMIN);

  // Con esta máscara se bloque las señales indicadas y es heredada por todas las hebras
  pthread_sigmask(SIG_BLOCK, &sigset, NULL);

  printf("Thread main: Señal # %d bloqueada por el proceso: %d\n",SIGRTMIN,
  getpid());

  senal=SIGRTMIN;

  printf("Thread main: Crea el thread de señal y espero\n\n");

  pthread_attr_init (&attr);

  pthread_create (&th_id, &attr, Thread, &senal);

  ………
}

```

Hebra manejadora de eventos:

```c
...

void *Thread(void *arg) {
  int sig;
  sigset_t sigset; // conjunto de señales
  int info; // información de la señal RT
  sig = (int)arg;

  printf("Thread : espero la señal # %d\n", sig);

  // conjunto de señales que vamos a esperar
  sigemptyset(&sigset);
  sigaddset(&sigset, sig);

  // Espera la cancelación o la señal
  while(1)
  {
    //Cuando se invoca el sigwait, si no ha llegado ninguna señal se bloquea, pero se desbloquea cuando llega una señal del grupo de señales indicado en el parámetro sigset. 
    // desbloquea y espera la señal de forma atómica
    sig=sigwait(&sigset, &info);

    // sigwaitinfo es un punto de cancelación
    if (sig!=-1)
    {
      printf("----- Señal %d depositada en el thread \n",sig);

      MostraDatosSig(&info);
    }
}
```