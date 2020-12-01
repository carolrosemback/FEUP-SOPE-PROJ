#include "Q.h"

bool numStr(char* str)
{
    bool started = false;
    bool finished = false;
    int first = 0;
    for(int i = 0; str[i] != '\0'; i++)
    {
        if(!started && str[i] != ' ')
        {
            if(!isdigit(str[i]) && str[i] != '-') return false;
            else{started = true; first = i;}
        }
        if(started)
        {
            if(i == first && (!isdigit(str[i]) && str[i] != '-')) return false;
            if(i != first && !isdigit(str[i]))
            {
                if(str[i] == ' ') finished = true;
                else return false;
            }
            if(finished && str[i] != ' ') return false;
        }
    }
    return started;
}

bool processArgs(int argc, char* argv[], double* nsecs, int* nplaces, int* nthreads, char* FIFO_path)
{
	if(argc != 8) 
        return false;

	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i],"-t") == 0)
        {
            if(numStr(argv[i+1]))
            {
                *nsecs = atoi(argv[i+1]);
            }
            else
                return false;
            i++;
        }

        else if(strcmp(argv[i], "-l") == 0)
        {
            if(numStr(argv[i+1]))
            {
                *nplaces = atoi(argv[i+1]);
            }
            else
                return false;
            i++;
        }
        else if(strcmp(argv[i], "-n") == 0)
        {
            if(numStr(argv[i+1]))
            {
                *nthreads = atoi(argv[i+1]);
            }
            else
                return false;
            i++;
        }
        else 
        {
           strcpy(FIFO_path,argv[i]);
           if(FIFO_path[0] == '\0')
            return false;
        }

	}
	return true;
}

int readline(int fd, char *str)
{
    int n;
    do
    {
        n = read(fd,str,1);
    }
    while (n>0 && *str++ != '\0');
    return (n>0);
}

process_client_args* new_ProcessClientArgs()
{
    process_client_args* args;
    args = malloc(sizeof(process_client_args));
    args->i = -1;
    args->pid = -1;
    args->tid = -1;
    args->dur = -1;
    args->pl = -1;
    args->nPlaces = -1;
    return args;
}

bool parse_client_args(process_client_args* args, char* str)
{
    sscanf(str, "[ %d, %d, %ld, %d, %d ]", &args->i, &args->pid, &args->tid, &args->dur, &args->pl);
    return true;
}

void* process_client(void* arg)
{
    int fd;
    char path[MAX_FILE_NAME_LENGHT];
    long int tid = pthread_self();

    process_client_args* args = (process_client_args*) arg;
    
    printf("%ld ; %d ; %d ; %ld ; %d ; %d ; RECVD\n", time(NULL), args->i, getpid(), tid, args->dur, args->pl);

    sprintf(path,"/tmp/%d.%ld",args->pid,args->tid);
    fd = open(path,O_WRONLY);

    if(fd == -1)
    {
        printf("%ld ; %d ; %d ; %ld ; %d ; %d ; GAVUP\n", time(NULL), args->i, getpid(), tid, args->dur, args->pl);        
    }
    else
    {
        char  message[MAX_FILE_NAME_LENGHT];
        int messagelen;
        
        if(!wc_open)
        {
            printf("%ld ; %d ; %d ; %ld ; -1 ; -1 ; 2LATE\n", time(NULL), args->i, getpid(), tid);
            sprintf(message,"[ %d, %d, %ld, -1, -1 ]\n", args->i, getpid(), tid);
            messagelen=strlen(message)+1;
            if(write(fd,message,messagelen) < 0)
                fprintf(stderr, "Couldnt write to private fifo\n");
        }
        else
        {
            bool has_room = false;
            sem_wait(&sem);
            while(!has_room)
            {
                for(int i = 0; i < args->nPlaces; i++)
                {
                    pthread_mutex_lock(&mutex);
                    if(places[i] == 0)
                    {
                        args->pl = i;
                        places[i] = 1;
                        has_room = true;
                        sem_post(&sem);
                        pthread_mutex_unlock(&mutex);
                        break;
                    }
                    else pthread_mutex_unlock(&mutex);
                }
            }
            sprintf(message,"[ %d, %d, %ld, %d, %d ]\n", args->i, getpid(), tid, args->dur, args->pl+1);
            messagelen=strlen(message)+1;
            if(write(fd,message,messagelen) < 0)
                fprintf(stderr, "Couldnt write to private fifo\n");
            else
            {
                printf("%ld ; %d ; %d ; %ld ; %d ; %d ; ENTER\n", time(NULL), args->i, getpid(), tid, args->dur, args->pl+1);
                usleep(args->dur * 1000);
                printf("%ld ; %d ; %d ; %ld ; %d ; %d ; TIMUP\n", time(NULL), args->i, getpid(), tid, args->dur, args->pl+1);
                places[args->pl] = 0;
            }
        }
    }
    sem_post(&semaphore);
    free(args);
    close(fd);
    return NULL;
}

void* look_for_clients(void* arg)
{
    look_for_clients_args* this_args = (look_for_clients_args*) arg;
    pthread_t tid[MAX_THREADS];
    int curr_thread = 0;
    char  str[100000];
    mkfifo(this_args->FIFO_path,0660);



    if((main_fifo_fd=open((char*)this_args->FIFO_path,O_RDONLY)) < 0)
    {
        fprintf(stderr, "Error public opening FIFO\n");
        return NULL;
    }

    places = malloc(sizeof(int)*(this_args->nplaces));
    sem_init(&sem, 0, this_args->nplaces);
    for(int i = 0; i < this_args->nplaces; i++)
    {
        places[i] = 0;
    }

    sem_init(&semaphore, 0, this_args->nthreads-1); 

    while(readline(main_fifo_fd,str))
    {
        process_client_args* args = new_ProcessClientArgs();
        args->nPlaces = this_args->nplaces;
        if(!parse_client_args(args,str))
        {
            fprintf(stderr,"Communication error: bad args\n");
        }    
        else{
            if(pthread_create(&tid[curr_thread], NULL, process_client, args) != OK)
            {   
                fprintf(stderr,"System max threads reached\n");
                break;
            }
            sem_wait(&semaphore);
            curr_thread++;
            if(curr_thread > MAX_THREADS)
            {
                fprintf(stderr,"Max threads reached\n");
                break;
            }
        }
    }
    for(int i = 0; i < curr_thread; i++)
    {
        pthread_join(tid[i],NULL);
    }
    close(main_fifo_fd);
    free(this_args);
    return NULL;
}

int main(int argc, char* argv[])
{
	double nsecs;
    int nplaces, nthreads;
	char FIFO_path[MAX_FILE_NAME_LENGHT];   
    pthread_t tid;
    time_t start, end;
    double elapsed;

	if(!processArgs(argc, argv, &nsecs, &nplaces, &nthreads, FIFO_path))
	{
		fprintf(stderr,"Argument error\n");
		return -1;
	}

    look_for_clients_args* args;
    args = malloc(sizeof(look_for_clients_args));
    args->nplaces = nplaces;
    args->nthreads = nthreads;
    strcpy(args->FIFO_path, FIFO_path);

    pthread_create(&tid, NULL, look_for_clients, args);	
    start = time(NULL);
    do
    {
        end = time(NULL);
        elapsed = difftime(end, start);
    }
    while (elapsed < nsecs);
    
    wc_open = false;
    if(main_fifo_fd != -1)
        pthread_join(tid,NULL);
    
    remove(FIFO_path);

	return 0;
}
