#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define BUFFER_SIZE 10

typedef struct {
    int buffer[BUFFER_SIZE];
    int head;
    int tail;
    sem_t mutex;
    sem_t items;
    sem_t spaces;
} Monitor;

void init(Monitor *monitor) {
    monitor->head = 0;
    monitor->tail = 0;
    sem_init(&monitor->mutex, 0, 1);
    sem_init(&monitor->items, 0, 0);
    sem_init(&monitor->spaces, 0, BUFFER_SIZE);
}

void destroy(Monitor *monitor) {
    sem_destroy(&monitor->mutex);
    sem_destroy(&monitor->items);
    sem_destroy(&monitor->spaces);
}

void add(Monitor *monitor, int value) {
    sem_wait(&monitor->spaces);
    sem_wait(&monitor->mutex);
    monitor->buffer[monitor->tail] = value;
    monitor->tail = (monitor->tail + 1) % BUFFER_SIZE;
    sem_post(&monitor->mutex);
    sem_post(&monitor->items);
}

int read(Monitor *monitor) {
    sem_wait(&monitor->items);
    sem_wait(&monitor->mutex);
    int value = monitor->buffer[monitor->head];
    monitor->head = (monitor->head + 1) % BUFFER_SIZE;
    sem_post(&monitor->mutex);
    sem_post(&monitor->spaces);
    return value;
}

Monitor shared_monitor;

void *producer(void *arg) {
    srand(time(NULL));
    while (1) {
        
        int value = rand();
        add(&shared_monitor, value);
        printf("Producer %ld added value %d\n", (long)arg, value);
        sleep(1);
    }
    pthread_exit(NULL);
}

void *consumer(void *arg) {
    while (1) {
        int value = read(&shared_monitor);
        printf("Consumer %ld read value %d\n", (long)arg, value);
        sleep(1);
    }
    pthread_exit(NULL);
}

int main() {
    init(&shared_monitor);

    pthread_t producer1, producer2, consumer1, consumer2;
    pthread_create(&producer1, NULL, producer, (void *)1);
    pthread_create(&producer2, NULL, producer, (void *)2);
    pthread_create(&consumer1, NULL, consumer, (void *)1);
    pthread_create(&consumer2, NULL, consumer, (void *)2);

    pthread_join(producer1, NULL);
    pthread_join(producer2, NULL);
    pthread_join(consumer1, NULL);
    pthread_join(consumer2, NULL);

    destroy(&shared_monitor);

    return 0;
}