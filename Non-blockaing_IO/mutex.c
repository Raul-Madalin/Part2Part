#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t locked;
int a;
int b;

void* procfunc1(void* var) {

	printf("X\n");

	pthread_mutex_lock(&locked);
	
	a++;
	printf("%d\n", a);
	//int* id = (int*) var;
	//printf("Thread ID: %d\n", id);

	pthread_mutex_unlock(&locked);
}

void* procfunc2(void* var) {

	printf("Y\n");

	pthread_mutex_lock(&locked);
	
	a*=10;
	printf("%d\n", a);
	//int* id = (int*) var;
	//printf("Thread ID: %d\n", id);

	pthread_mutex_unlock(&locked);
}

int main() {
	int error;
	int *th1, *th2;
	th1 = 1;
	th2 = 2;
	
	if(pthread_mutex_init(&locked, NULL) != 0) {
		printf("Mutex creation was unsuccessful !");
		exit(1);
	}
	
	a = 0;
	b = 10;

	pthread_create(&th1, NULL, procfunc1, th1);
	pthread_create(&th2, NULL, procfunc2, th2);

	pthread_join(th1, NULL);
	//pthread_join(th2, NULL);

	return 0;
}