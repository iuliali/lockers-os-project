#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <Windows.h>


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
//    atomic_int zero;
//    atomic_store(&zero,0);
    acquire(&sem->mutex);
    //printf("thread id = %d : wait, sem : %d\n", pthread_self(), atomic_load(&sem->S));
    //printf("wait : %d\n", sem->S);
    while(atomic_load(&sem->S) < 1) ;
    //printf("wait\n");
    printf("thread %d entered sem: %d\n", pthread_self(), atomic_load(&sem->S));


    atomic_fetch_sub(&sem->S, 1);
    release(&sem->mutex);
}

void post(semaphore* sem) {
    atomic_fetch_add(&sem->S, 1);
    printf("thread %d out sem: %d\n", pthread_self(), atomic_load(&sem->S));

    //printf("post\n");
}



int loop = 100000;
int n =0; int k = 0;
semaphore s;


void* foo(void *p)
{


//    for(i = 0; i < loop; i++) {
        //printf("thread %d\n", *ptr);
        wait(&s);
        //++n;
        Sleep(3000);
        //printf("%d\n", n);

        post(&s);
        //printf("thread %d out\n", pthread_self());


//        //neprotejat de semafor
//        ++k;
//    }

    return NULL;
 }

int count = 10;

int main(int argc, char **argv)
{

    init_sem(&s, 3);
    int i;
    pthread_t pids[count];
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    for(i = 0; i < count; i++) {
        pthread_create(&pids[i], &attr, foo, i);
    }

    for(i = 0; i < count; i++) {
        pthread_join(pids[i], NULL);
    }

    printf("Cu sem nostru %d\n", n);
    printf("Fara sem %d\n", k);

    return 0;
}

