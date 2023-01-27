#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct spinlock_mutex{
    volatile atomic_flag taken;
} mutex;

const mutex mutex_init = {ATOMIC_FLAG_INIT};

void init_m(mutex* mutex) {
    *mutex = mutex_init;
}

void acquire(mutex* mutex) {
        while (atomic_flag_test_and_set(&mutex->taken));
    }
void release(mutex* mutex) {
	atomic_flag_clear(mutex);
}

typedef struct semaphore{
    volatile atomic_int S;
    mutex mutex;
} semaphore;

//const semaphore sem_init = {0};

void init_sem(semaphore* sem, unsigned int value) {
    atomic_int val;
    atomic_store(&val, value);
    atomic_init(&sem->S, val);
    init_m(&sem->mutex);
}

void wait(semaphore* sem){
    printf("wait\n");
    while(atomic_load(&sem->S) <= 0) ;
    atomic_fetch_sub(&sem->S, 1);
}

void post(semaphore* sem) {
    atomic_fetch_add(&sem->S, 1);
    printf("post\n");
}



int loop = 10;
int n =0; int k = 0;
semaphore s;


void* foo(void *p)
{
    int i = 0;
    for(i = 0; i < loop; i++) {

        wait(&s);
        ++n;
        printf("%d\n", n);
        post(&s);

        //neprotejat de semafor
        ++k;
    }

    return NULL;
 }

int count = 100;

int main(int argc, char **argv)
{

    init_sem(&s, 1);
    int i;
    pthread_t pids[count];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    for(i = 0; i < count; i++) {
        pthread_create(&pids[i], &attr, foo, NULL);
    }

    for(i = 0; i < count; i++) {
        pthread_join(pids[i], NULL);
    }

    printf("Cu sem nostru %d\n", n);
    printf("Fara sem %d\n", k);

    return 0;
}

