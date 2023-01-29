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

const semaphore sem_init = {{0}, {0}};



void init_sem(semaphore* sem, unsigned int value) {
    *sem = sem_init;
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

//lightswitch
typedef struct lightswitch {
    atomic_int counter;
    mutex mutex;
} lightswitch;

const lightswitch ls_init = {0, 0};


void init_lightswitch(lightswitch* ls) {
    *ls = ls_init;
    atomic_store(&ls->counter, 0);
    init_m(&ls->mutex);
}

void lock_lightswitch(lightswitch* ls, semaphore* writer_sem) {
    acquire(&ls->mutex);
    atomic_fetch_add(&ls->counter, 1);
        if (atomic_load(&ls->counter) == 1) {
            wait(&writer_sem);
        }
    release(&ls->mutex);
}

void unlock_lightswitch(lightswitch* ls, semaphore* writer_sem) {
    acquire(&ls->mutex);
    atomic_fetch_sub(&ls->counter, 1);
        if (atomic_load(&ls->counter) == 0) {
            post(&writer_sem);
        }
    release(&ls->mutex);
}

int j = 0;


typedef struct rwlock {
    lightswitch read_switch;
    semaphore room_empty;
    semaphore turnstile;
} rwlock;
const rwlock rw_init = {{0,0}, {0,0}, {0,0}};

void init_rwlock(rwlock* lock) {
    *lock = rw_init;
    lock->read_switch = ls_init;
    lock->room_empty = sem_init;
    lock->turnstile = sem_init;

    init_lightswitch(&lock->read_switch);
    init_sem(&lock->room_empty, 1);
    init_sem(&lock->turnstile, 1);
}

void write(rwlock* lock) {
    wait(&lock->turnstile);
        wait(&lock->room_empty);
        // critical section writers
        ++j;
        printf("write j = %d\n", j);

    post(&lock->turnstile);
    post(&lock->room_empty);
}

void read(rwlock* lock) {
    wait(&lock->turnstile);
    post(&lock->turnstile);

    lock_lightswitch(&lock->read_switch, &lock->room_empty);
        // critical section readers
        printf("read j = %d\n", j);

    unlock_lightswitch(&lock->read_switch, &lock->room_empty);
}

rwlock rw = rw_init;

//init_rwlock(&rw);



void* reader_2(void* p) {
    printf("reader %d read j\n", (*((int *)p)));
    read(&rw);
}


void* writer_2(void* p) {
    printf("writer %d write j \n", (*((int *)p)));

    write(&rw);
}


int main()
{
//    pthread_t read[2*readers_count],write[writers_count];
//    pthread_attr_t attr;
//    pthread_attr_init(&attr);
//    int a[2*readers_count];
//    int i;
//    init_m(&m);
//    init_sem(&wrt, 1);
//
//    for(i = 0; i < readers_count; i++) {
//        a[i] = i;
//        pthread_create(&read[i], &attr, reader, (void *)&a[i]);
//    }
//    for(i = 0; i < writers_count; i++) {
//        a[i] = i;
//        pthread_create(&write[i], &attr, writer, (void *)&a[i]);
//    }
//
//    for(i = readers_count; i < 2 * readers_count; i++) {
//        a[i] = i;
//        pthread_create(&read[i], &attr, reader, (void *)&a[i]);
//    }
//
//    for(i = 0; i < readers_count; i++) {
//        pthread_join(read[i], NULL);
//    }
//    for(i = 0; i < writers_count; i++) {
//        pthread_join(write[i], NULL);
//    }
//    printf("n = %d\n", n);

//cu lightswitch -> no starvation

    pthread_t read[2*readers_count],write[writers_count];
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    int a[readers_count];
    int w[writers_count];
    int aa[readers_count];
    init_rwlock(&rw);
    int i, s , t, nn , mm;

    for(i = 0; i < readers_count; i++) {
        a[i] = i;
        pthread_create(&read[i], &attr, reader_2, (void *)&a[i]);
        Sleep(10);
    }
    for(s = 0; s < writers_count; s++) {
        w[s] = s;
        pthread_create(&write[s], &attr, writer_2, (void *)&w[s]);
    }
    for(t = readers_count; t < 2*readers_count; t++) {
        aa[t] = t;
        pthread_create(&read[t], &attr, reader_2, (void *)&aa[t]);
    }

    for(nn = 0; nn < 2*readers_count; nn++) {
        pthread_join(read[nn], NULL);
    }
    for(mm = 0; mm < writers_count; mm++) {
        pthread_join(write[mm], NULL);
    }
    printf("j = %d\n", j);
    return 0;
}
