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

void init_sem(semaphore* sem, unsigned int value) {
    atomic_int val;
    atomic_store(&val, value);
    atomic_init(&sem->S, val);
    init_m(&sem->mutex);
}

void wait(semaphore* sem){
    //printf("wait\n");
    acquire(&sem->mutex);
    while(atomic_load(&sem->S) <= 0) ;
    atomic_fetch_sub(&sem->S, 1);
    release(&sem->mutex);
}

void post(semaphore* sem) {
    atomic_fetch_add(&sem->S, 1);
    //printf("post\n");
}

int n = 0; int k = 0;
int readers_count = 100, writers_count = 10, num_readers = 0;

semaphore wrt;
mutex m;

void* writer(void* p){
    printf("writer %d is trying to enter\n", (*((int *)p)));
    wait(&wrt);

    printf("writer %d has modified n\n", (*((int *)p)));
    ++n;
    post(&wrt);
    printf("writer %d has left\n", (*((int *)p)));
}

void* reader(void* p){
    acquire(&m);
    ++num_readers;
    if(num_readers == 1){
        wait(&wrt);
    }
    release(&m);

    printf("reader %d has read n \n", (*((int *)p)));

    acquire(&m);
    --num_readers;
    if(num_readers == 0){
        post(&wrt);
    }
    release(&m);
    printf("reader %d has left\n", (*((int *)p)));
}


int main()
{
    pthread_t read[readers_count],write[writers_count];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int a[readers_count];
    int i;
    init_m(&m);
    init_sem(&wrt, 1);

    for(i = 0; i < readers_count; i++) {
        a[i] = i;
        pthread_create(&read[i], &attr, reader, (void *)&a[i]);
    }
    for(i = 0; i < writers_count; i++) {
        a[i] = i;
        pthread_create(&write[i], &attr, writer, (void *)&a[i]);
    }

    for(i = 0; i < readers_count; i++) {
        pthread_join(read[i], NULL);
    }
    for(i = 0; i < writers_count; i++) {
        pthread_join(write[i], NULL);
    }
    printf("n = %d\n", n);
    return 0;
}




