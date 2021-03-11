#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>
#include <semaphore.h>
#include <unistd.h>

#define MY_RAND_MAX 999

pthread_mutex_t test_mutex;
int buffer[5];
int count = 0;
bool active = true;
sem_t empty;
sem_t full;

void *producer(void *);

void *consumer(void *);

int getRandomNum(int);

int main(int argc, char * argv[])
{
    srand(time(NULL));
    int sleepNo, pNo, cNo;

    printf("Number of arguments = %d", argc-1);
    sleepNo = atoi(argv[1]);
    printf("\nMain process sleep time = %d", sleepNo);
    pNo = atoi(argv[2]);
    printf("\nNumber of producers = %d", pNo);
    cNo = atoi(argv[3]);
    printf("\nNumber of consumers = %d", cNo);

    sem_init(&empty, 0, 5);
    sem_init(&full, 0, 0);

    pthread_t pro[pNo], con[cNo];
    int proCount[pNo], conCount[cNo];

    for (int i = 0; i < pNo; ++i) {
        proCount[i] = i+1;
        pthread_create(&pro[i], NULL, &producer, &proCount[i]);
        printf("\nProducer-%d thread created successfully", i+1);
    }

    for (int i = 0; i < cNo; ++i) {
        conCount[i] = i+1;
        pthread_create(&con[i], NULL, &consumer, &conCount[i]);
        printf("\nConsumer-%d thread created successfully", i+1);
    }

    sleep(sleepNo);
    printf("\nSleep time elapsed. Producers and Consumers terminated.");
    active = false;

    for (int i = 0; i < pNo; ++i) {
        pthread_join(pro[i], NULL);
    }

    for (int i = 0; i < cNo; ++i) {
        pthread_join(con[i], NULL);
    }

    int leftover = 0;
    for (int i = 0; i < 5; ++i) {
        if (buffer[i] != 0)
            leftover++;
    }
    printf("\nNumber of items in buffer = %d\n", leftover);

    pthread_mutex_destroy(&test_mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    exit(0);
}

void *consumer(void *arg) {
    int thread_number = *((int *) arg);

    do {
        int nextNom;

        if (active)
            sem_wait(&full);
        else
            return NULL;

        pthread_mutex_lock(&test_mutex);

        int full_value;
        sem_getvalue(&full, &full_value);

        nextNom = buffer[full_value];
        buffer[full_value] = 0;

        if (active)
            printf("\nConsumer-%d removed item \t%d", thread_number, nextNom);

        pthread_mutex_unlock(&test_mutex);

        if (active)
            sem_post(&empty);

        sleep(getRandomNum(3));

    } while (active);
    sem_post(&full);
    return NULL;
}

void *producer(void *arg) {
    int thread_number = *((int *) arg);

    do {
        int nextItem = getRandomNum(MY_RAND_MAX);

        if (active)
            sem_wait(&empty);
        else
            return NULL;

        pthread_mutex_lock(&test_mutex);

        int full_value;
        sem_getvalue(&full, &full_value);

        buffer[full_value] = nextItem;

        if (active)
            printf("\nProducer-%d inserted item %d", thread_number, nextItem);

        pthread_mutex_unlock(&test_mutex);

        if (active)
            sem_post(&full);

        sleep(getRandomNum(3));

    } while (active);
    sem_post(&empty);
    return NULL;
}

int getRandomNum(int MAX) {
    return (rand() % MAX) + 1;
}
