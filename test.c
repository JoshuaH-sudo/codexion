// Test file to learn threads in C
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void* foo(void* arg) {
    printf("Thread1 is running.\n");

    // Explicitly terminate thread
    pthread_exit(NULL);

    printf("This will not be executed.\n");
    return NULL;
}

void* myThreadFunc(void* arg) {
    while(1) {
        printf("Thread2 is running...\n");
        sleep(1);
    }
    return NULL;
}

int main() {
    pthread_t thread1, thread2;
	// pthread_Create(pointer to var, attributes, function to run, argument to function)
    pthread_create(&thread1, NULL, foo, NULL);
	pthread_create(&thread2, NULL, myThreadFunc, NULL);
    sleep(5);
    //Requesting to cancel the thread after 5 seconds.
    pthread_cancel(thread2);
    // Wait for created thread to finish
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

	printf("Main thread finished.\n");
    return 0;
}
