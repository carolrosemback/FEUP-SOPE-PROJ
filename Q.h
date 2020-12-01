#pragma once

#include <stdbool.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define MAX_FILE_NAME_LENGHT 1024
#define MAX_THREADS 100000
#define OK 0

bool wc_open = true;
int main_fifo_fd = -1;
int* places;
int line[MAX_THREADS] = {0};

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

sem_t sem; 
sem_t semaphore; 

typedef struct process_client_args
{
	int i;
	int pid;
	long tid;
	int dur;
	int pl;
	int nPlaces;
}process_client_args;

typedef struct look_for_clients_args
{
	int nplaces;
	int nthreads;
	char FIFO_path[MAX_FILE_NAME_LENGHT];
}look_for_clients_args;

bool numStr(char* str);
bool processArgs(int argc, char* argv[], double* nsecs, int* nplaces, int*nthreads, char* FIFO_path);
int readline(int fd, char *str);
process_client_args* new_ProcessClientArgs();
bool parse_client_args(process_client_args* args, char* str);
void* process_client(void* arg);
void* look_for_clients(void* FIFO_path);

