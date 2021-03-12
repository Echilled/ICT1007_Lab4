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
    /* Seed the random number generator */
    srand(time(NULL));
    int sleepNo, pNo, cNo;

    switch (argc) {
        case 4:
            printf("Number of arguments = %d", argc-1);
            sleepNo = atoi(argv[1]);
            printf("\nMain process sleep time = %d", sleepNo);
            pNo = atoi(argv[2]);
            printf("\nNumber of producers = %d", pNo);
            cNo = atoi(argv[3]);
            printf("\nNumber of consumers = %d", cNo);
            break;
        default:
            printf("Incorrect number of arguments!");
            exit(0);
    }

    /* Initialize the semaphores
     * empty = 5
     * full = 0 */
    sem_init(&empty, 0, 5);
    sem_init(&full, 0, 0);

    /* Create pthreads according to number specified in the argument
     * Store the identifier in an array in order to pass it as a parameter to the necessary functions */
    pthread_t pro[pNo], con[cNo];
    int proCount[pNo], conCount[cNo];

    for (int i = 0; i < pNo; ++i) {
        /* Create an identifier for the thread and store it into the array */
        proCount[i] = i+1;
        printf("\nProducer-%d thread created successfully", i+1);
    }

    for (int i = 0; i < cNo; ++i) {
        /* Create an identifier for the thread and store it into the array */
        conCount[i] = i+1;
        printf("\nConsumer-%d thread created successfully", i+1);
    }

    for (int i = 0; i < pNo; ++i) {
        /* Create the thread and pass the identifier as a parameter to the producer function */
        pthread_create(&pro[i], NULL, &producer, &proCount[i]);
    }

    for (int i = 0; i < cNo; ++i) {
        /* Create the thread and pass the identifier as a parameter to the consumer function */
        pthread_create(&con[i], NULL, &consumer, &conCount[i]);
    }

    /* Main thread will sleep based on number specified in the argument */
    sleep(sleepNo);
    printf("\nSleep time elapsed. Producers and Consumers terminated.");
    /* All the producers and consumers should not be running now */
    active = false;

    for (int i = 0; i < pNo; ++i) {
        /* Wait for the threads to terminate */
        pthread_join(pro[i], NULL);
    }

    for (int i = 0; i < cNo; ++i) {
        /* Wait for the threads to terminate */
        pthread_join(con[i], NULL);
    }

    /* Count the amount of leftover items in the array */
    int leftover = 0;
    for (int i = 0; i < 5; ++i) {
        if (buffer[i] != 0)
            leftover++;
    }
    printf("\nNumber of items in buffer = %d\n", leftover);

    /* Destroy the mutex & semaphores */
    pthread_mutex_destroy(&test_mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    exit(0);
}

/* Function for consumer threads */
void *consumer(void *arg) {
    /* Convert the parameter passed in into an integer (identifier) */
    int thread_number = *((int *) arg);

    do {
        /* Variable to hold the number to be consumed from buffer */
        int nextNom;

        /* sem_wait will decrement full
         * if full = 0, it will block until it gets incremented */
        if (active)
            sem_wait(&full);
        else
            return NULL;

        /* Mutex lock */
        pthread_mutex_lock(&test_mutex);

        /* Get the value of full and put into full_value
         * This is to tell the thread which element in the buffer is the most recent "full" value */
        int full_value;
        sem_getvalue(&full, &full_value);
//        printf("\nCurrent value in semaphore %d", full_value);

        nextNom = buffer[full_value];
        buffer[full_value] = 0;

        if (active)
            printf("\nConsumer-%d removed item \t%d", thread_number, nextNom);

        /* Mutex unlock */
        pthread_mutex_unlock(&test_mutex);

        /* Increment empty to signal that there are more free slots in the buffer */
        if (active)
            sem_post(&empty);

        /* Sleep based on random number between 1 to 3 */
        sleep(getRandomNum(3));

    } while (active);
    /* ??? */
    sem_post(&full);
    return NULL;
}

void *producer(void *arg) {
    /* Convert the parameter passed in into an integer (identifier) */
    int thread_number = *((int *) arg);

    do {
        /* Generate a random number */
        int nextItem = getRandomNum(MY_RAND_MAX);

        /* Get value of empty
         * If empty = 0, it will block until it gets incremented */
        if (active)
            sem_wait(&empty);
        else
            return NULL;

        /* Mutex lock */
        pthread_mutex_lock(&test_mutex);

        /* Get the value of full and put into full_value
         * This is to tell the thread which element in the buffer is the most recent "full" value */
        int full_value;
        sem_getvalue(&full, &full_value);
//        printf("\nCurrent value in semaphore %d", full_value);

        buffer[full_value] = nextItem;

        if (active)
            printf("\nProducer-%d inserted item %d", thread_number, nextItem);

        /* Mutex unlock */
        pthread_mutex_unlock(&test_mutex);

        /* Increment full to signal that there are less free slots in the buffer */
        if (active)
            sem_post(&full);

        /* Sleep based on random number between 1 to 3 */
        sleep(getRandomNum(3));

    } while (active);
    /* ??? */
    sem_post(&empty);
    return NULL;
}

int getRandomNum(int MAX) {
    return (rand() % MAX) + 1;
}
