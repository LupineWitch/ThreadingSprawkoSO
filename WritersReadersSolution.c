#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>

#define SLEEPTIME 10

static volatile int keepRunning = 1; //Flag for exiting from application

sem_t in; //Queue to be served
sem_t mx; //Access to ReadersIn
sem_t wrt; //Access to writing

int ReadersWaiting;
int WritersWaiting;
int ReadersIn; //Counter for our algorithm
int WritersIn;

pthread_mutex_t MainLock; //Lock for synchronizing logging to out stream
pthread_cond_t WriteNow; //Condition on what to wait for logging

void *Logger() //Logger thread to synchronize writing
{
        pthread_mutex_lock(&MainLock);
    while(keepRunning)
    {
        pthread_cond_wait(&WriteNow, &MainLock);
        printf("\nReadersQ: %d WritersQ: %d [in: R:%d W:%d]", ReadersWaiting,WritersWaiting,ReadersIn,WritersIn);
    }
        pthread_mutex_unlock(&MainLock);
}

void* Reader(void* params)
{
    int id = (int)(intptr_t)params; //Get ID of the Reader
    while(keepRunning)
    {
        ReadersWaiting++;
        pthread_cond_signal(&WriteNow);
        sem_wait(&in); //Waiting to be served
        ReadersWaiting--;
        pthread_cond_signal(&WriteNow);
        
        sem_wait(&mx); //Waiting for the ReadersIn access
        

        if(ReadersIn == 0)
        {
            sem_wait(&wrt);
        }
        ReadersIn++;
        pthread_cond_signal(&WriteNow);

        sem_post(&in);
        sem_post(&mx);

        /*
        Reading resource [CRITICAL SECTION FOR READERS]      
        */
        sleep(rand()%SLEEPTIME);

        sem_wait(&mx);
        ReadersIn--;
        pthread_cond_signal(&WriteNow);
        if(ReadersIn == 0)
        {
            sem_post(&wrt);
        }
        sem_post(&mx);
    }
}

void* Writer(void* params)
{
    int id = (int)(intptr_t)params; //Get ID of the Writer
    while(keepRunning)
    {
        WritersWaiting++;
        pthread_cond_signal(&WriteNow);
        sem_wait(&in);
        WritersWaiting--;
        pthread_cond_signal(&WriteNow);
        

        sem_wait(&wrt);
        WritersIn++;
        pthread_cond_signal(&WriteNow);
        sem_post(&in);

        /*
        Writing resource [CRITICAL SECTION FOR WRITERS]
        */
        sleep(rand()%SLEEPTIME);
        
        sem_post(&wrt);
        WritersIn--;
        pthread_cond_signal(&WriteNow);
    }
}

void exit_handler()
{
    keepRunning=0;
}

int main(int argc, char* argv[])
{
    //Program won't work if number of arguments is different than 2 (+1 becasue name of program is also an argument)
    if(argc != 3)
    {
        printf("Sposob wywolania: 3 <liczba czytelnikow> <liczba pisarzy>\n");
        return -1;
    }

    //Catching SIGINT to safely kill application
    signal(SIGINT, exit_handler);

    //Declare table for Readers threads
    int NumR = atoi(argv[1]);
    pthread_t Readers[NumR];

    //Declare table for Writers threads
    int NumW = atoi(argv[2]);
    pthread_t Writers[NumW];

    //Declare Logging thread
    pthread_t Logger_th;

    //Initialize semaphores
    sem_init(&in, 0, 1);
    sem_init(&mx, 0, 1);
    sem_init(&wrt, 0, 1);

    //Initialize counts of Readers and Writers
    ReadersWaiting = 0;
    WritersWaiting = 0;
    ReadersIn = 0;
    WritersIn = 0;

    //Initialize mutex and CV for Logger
    pthread_mutex_init(&MainLock, NULL);
    pthread_cond_init(&WriteNow, NULL);

    //Initialize Logging thread
    pthread_create(&Logger_th, NULL, &Logger, NULL);

    //Initialize Readers threads
    for(int i=0; i<NumR; i++)
    {
        pthread_create(&Readers[i], NULL, &Reader, (void*)(intptr_t)i);
    }

    //Initialize Writers threads
    for(int i=0; i<NumW; i++)
    {
        pthread_create(&Writers[i], NULL, &Writer, (void*)(intptr_t)i);
    }

    //Wait for threads to finish and freeing the memory
    pthread_join(Logger_th, NULL);
    for(int i=0; i<NumR; i++)
    {
        pthread_join(Readers[i], NULL);
    }
    for(int i=0; i<NumR; i++)
    {
        pthread_join(Writers[i], NULL);
    }
    
    //Freeing the semaphores, mutexes and CVs
    sem_destroy(&in);
    sem_destroy(&mx);
    sem_destroy(&wrt);
    pthread_mutex_destroy(&MainLock);
    pthread_cond_destroy(&WriteNow);

    return 0;
}