#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>

#define MAX_ITERATION 1000000

int i = 0;
sem_t s;

void* incThreadFunc() {

	sem_wait(&s);
	printf("increasing\n");
	for(int d = 0; d < MAX_ITERATION; d++){
		i++;
	}
	sem_post(&s);
	return NULL;
}

void* decThreadFunc() {

	sem_wait(&s);
	printf("decreasing\n");
	for(int d = 0; d < MAX_ITERATION-1; d++){
		i--;
	}
	sem_post(&s);
	return NULL;
}

int main() {

	pthread_t t1, t2;

	sem_init(&s, 0, 1);
	
	pthread_create(&t1, NULL, *incThreadFunc, NULL);
	pthread_create(&t2, NULL, *decThreadFunc, NULL);
	
	pthread_join(t1, NULL);
	pthread_join(t2, NULL);
	
	printf("Value: %d\n", i);
	return 0;
}
