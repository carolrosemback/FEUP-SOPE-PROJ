#include "U.h"

void writeRegister(int i, int pid, long tid, int dur, int pl, char *oper) {
    printf("%ld ; %d ; %d ; %ld ; %d ; %d ; %s\n", time(NULL), i, pid, tid, dur, pl, oper);
    fflush(stdout);
}

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

void *pedidos(void *arg)
{
    int fd = *(int*)arg;
    int pid = getpid();
    long int tid = pthread_self();
    char msg[SIZE];
    int time = rand()% RAND + 1;
    int messagelen;
    char private_fifo[SIZE]="/tmp/";
    int fd_priv;
    char temp[SIZE];
    int id, s_pid, pl, dur; //WC
    long s_tid;

    sprintf(temp,"%d",pid);
    strcat(private_fifo,temp);
    strcat(private_fifo,".");
    sprintf(temp,"%ld",tid);
    strcat(private_fifo,temp);


    if (fd==-1)
    {
        //printf("WC is closed\n");
        return NULL;
    }

    writeRegister(i,pid,tid,time,-1,IWANT); 
    
    if (mkfifo(private_fifo, 0660) != 0) //create private FIFO
    {
        fprintf(stderr,"Error creating private FIFO\n");
        return NULL;
    }

    sprintf(msg, "[ %d, %d, %ld, %d, %d ]", i, pid, tid, time, -1);
    messagelen=strlen(msg)+1;
    if (write(fd, &msg, messagelen) < 0)
        return NULL;

    if ((fd_priv = open(private_fifo, O_RDONLY)) < 0) //open private FIFO
    {
        fprintf(stderr, "Error opening private FIFO\n");
        unlink(private_fifo);
        remove(private_fifo);
        return NULL;
    }

    if (read(fd_priv, &msg, SIZE) <= 0)
        writeRegister(i, pid, tid, dur, pl, FAILD);
    else //read private FIFO
    {
        sscanf(msg, "[ %d, %d, %ld, %d, %d ]", &id, &s_pid, &s_tid, &dur, &pl);

        if (pl == -1 && dur == -1) //closing
        {
            writeRegister(id, pid, tid, dur, pl, CLOSD);
            wc_open = false;
        }
        else//open
            writeRegister(id, pid, tid, dur, pl, IAMIN);
    }

    close(fd_priv);
    if(unlink(private_fifo) < 0)    //Destroys private FIFO
        fprintf(stderr, "Error destroying Private FIFO\n");
    return NULL;
}

bool processArgs(int argc, char* argv[], double* nsecs, char* FIFO_path)
{
    if(argc != 4) return false;

    for(int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i],"-t") == 0)
        {
            if(numStr(argv[i+1]))
            {
                *nsecs = atoi(argv[i+1]);
            }
            else return false;
            i++;
        }
        else
        {
            strcpy(FIFO_path,argv[i]);
        }
    }
    return true;
}

int main(int argc, char *argv[])
{
    double nsecs;
    char fifo[1024];
    //char fifopath[SIZE]="tmp/";
    pthread_t threads[MAX_THR];
    int thr = 0;
    int fd = -1;
    time_t start, end;
    double elapsed;

    if(!processArgs(argc, argv, &nsecs, fifo))
    {
        fprintf(stderr,"Argument error\n");
        return -1;
    }

    //strcat(fifopath,fifo);

    start = time(NULL);
    do
    {
        end = time(NULL);

        if(fd == -1)
        {
            if((fd=open(fifo,O_WRONLY)) <0)
            {
                fprintf(stderr, "Error opening FIFO\n");
            }        
        }
        if(pthread_create(&threads[thr],NULL,pedidos,&fd) != 0)
        {
            fprintf(stderr, "System max threads reached\n");
            continue;
        }
        usleep(10 * 1000);
        thr++;
        i++;
        elapsed = difftime(end, start);
    }
    while (elapsed < nsecs && wc_open);

    for(int i = 0; i < thr; i++)
    {
        pthread_join(threads[i],NULL);
    }

    if(fd != -1) close(fd);

    return 0;
}




