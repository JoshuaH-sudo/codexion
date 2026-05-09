// Test file to learn condition variables in C
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

// Shared state
pthread_mutex_t	mutex;
pthread_cond_t	cond;
bool		resource_ready = false;

// Producer thread: Prepares resource and signals
void* producer(void* arg) {
	printf("Producer: Starting work...\n");
	sleep(2);  // Simulate work
	printf("Producer: Resource is ready!\n");

	// Lock mutex before modifying shared state
	pthread_mutex_lock(&mutex);
	resource_ready = true;
	printf("Producer: Signaling ALL waiting threads\n");
	// Broadcast wakes ALL waiting threads (not just one)
	pthread_cond_broadcast(&cond);
	pthread_mutex_unlock(&mutex);

	return NULL;
}

// Consumer thread: Waits for resource
void* consumer(void* arg) {
	int id = *(int *)arg;

	printf("Consumer %d: Waiting for resource...\n", id);

	// Lock mutex before waiting
	pthread_mutex_lock(&mutex);

	// Wait: releases lock, sleeps, re-acquires lock when signaled
	while (!resource_ready) {
		printf("Consumer %d: Sleeping on condition variable\n", id);
		pthread_cond_wait(&cond, &mutex);
	}

	printf("Consumer %d: Got resource! Done.\n", id);
	pthread_mutex_unlock(&mutex);

	return NULL;
}

int main(void) {
	pthread_t prod, cons1, cons2;
	int id1 = 1, id2 = 2;

	// Initialize mutex and condition variable
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	printf("Main: Creating producer and consumers\n");
	pthread_create(&prod, NULL, producer, NULL);
	pthread_create(&cons1, NULL, consumer, &id1);
	pthread_create(&cons2, NULL, consumer, &id2);

	// Wait for all threads
	pthread_join(prod, NULL);
	pthread_join(cons1, NULL);
	pthread_join(cons2, NULL);

	// Cleanup
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&cond);

	printf("Main: All done!\n");
	return 0;
}
