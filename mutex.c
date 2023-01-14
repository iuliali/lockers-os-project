#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

typedef struct spinlock_mutex{
    volatile atomic_flag taken;
} mutex;

const mutex mutex_init = {ATOMIC_FLAG_INIT};

void init(mutex* mutex) {
    *mutex = mutex_init;
}

void acquire(mutex* mutex) {
        while (atomic_flag_test_and_set(&mutex->taken));
    }
void release(mutex* mutex) {
	atomic_flag_clear(mutex);
}

int loop = 100000;
int n =0; int k = 0;
mutex m;


void* foo(void *p)
{
    int i = 0;
    for(i = 0; i < loop; i++) {

        acquire(&m);
        ++n;
        release(&m);

        //neprotejat de mutex
        ++k;
    }

    return NULL;
 }

int count = 100;

int main(int argc, char **argv)
{
    init(&m);
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

    printf("Cu mutexul nostru %d\n", n);
    printf("Fara mutex %d\n", k);
    return 0;
}

