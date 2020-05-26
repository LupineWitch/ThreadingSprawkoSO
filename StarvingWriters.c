#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define WAITCONST 10 

sem_t ResourceSem;

sem_t MutexSem;

pthread_cond_t WriteNow = PTHREAD_COND_INITIALIZER;

pthread_mutex_t MainLock = PTHREAD_MUTEX_INITIALIZER;

int ReadersCount = 0;

int ReadersQueue = 0;

int WritersCount = 0;

int WritersQueue = 0;

void *Logger()
{
        pthread_mutex_lock(&MainLock);
    for(;;)
    {
        pthread_cond_wait(&WriteNow, &MainLock);
        printf("\nReadersQ:%d WritersQ:%d [in:R:%d W:%d]", ReadersQueue,WritersQueue,ReadersCount,WritersCount);
    }
        pthread_mutex_unlock(&MainLock);


}


void *Writer()
{
    for(;;)
    {
    sem_wait(&ResourceSem);
    WritersQueue--;
    WritersCount++;
    pthread_cond_signal(&WriteNow);
    //printf("\nWriter aquired the lock");

    int sleepS = rand()%WAITCONST;

    sleep(sleepS);
    WritersCount--;
    WritersQueue++;
    pthread_cond_signal(&WriteNow);
    sem_post(&ResourceSem);
   // printf("\nWriter released the lock");
  // sleep(sleepS);
    }

}


void *Reader()
{
    int sleepS = 0;
    for(;;)
    {
    //Readers Entry Section (Only one reader fits the door :VVVV)
    sem_wait(&MutexSem);
    ReadersQueue--;
    ReadersCount++;
    //printf("\nReader Entered room Count is %d",ReadersCount);
    pthread_cond_signal(&WriteNow);
    if(ReadersCount == 1 )
    {
        sem_wait(&ResourceSem);
    }
    sem_post(&MutexSem);

    //Simulating reading

         sleepS = rand()%WAITCONST;

        sleep(sleepS);

    //Readers exit section 

    sem_wait(&MutexSem);
    ReadersCount--;
    ReadersQueue++;
    pthread_cond_signal(&WriteNow);
    if(ReadersCount == 0 )
    {
        sem_post(&ResourceSem);
    }
    sem_post(&MutexSem);
    //sleep(sleepS);
    }
    //printf("\nReader exited room Count is %d",ReadersCount);
}



///
///param argv[0] - liczba czytaczy
///param  argv[1] - liczba pisaczy
///

int main(int argc, char **args)
{
const int READERSCOUNT = argc >= 2 ?  atoi(args[1]) : 10;
const int WRITERSCOUNT = argc >= 3 ?  atoi(args[2]) : 5;
ReadersQueue = READERSCOUNT;
WritersQueue = WRITERSCOUNT;
sem_init(&ResourceSem,0,1);
sem_init(&MutexSem,0,1);

srand(5);
    ///TODO: Print if no args are present

pthread_t *WritersArray = (pthread_t*)malloc(WRITERSCOUNT*(sizeof(pthread_t)));
pthread_t *ReadersArray = (pthread_t*)malloc(READERSCOUNT*(sizeof(pthread_t)));
pthread_t LogThread = (pthread_t)malloc(1*(sizeof(pthread_t)));
int i = 0;

pthread_create(&LogThread,NULL,Logger,NULL);

    for(i = 0 ; i < READERSCOUNT; i++)
    {
        pthread_create(&ReadersArray[i],NULL,Reader,NULL);
    }
    for(i = 0 ; i < WRITERSCOUNT; i++)
    {
         pthread_create(&WritersArray[i],NULL,Writer,NULL);
    }

    for(i = 0 ; i < WRITERSCOUNT; i++)
    {
        pthread_join(WritersArray[i], NULL);
    }
      for(i = 0 ; i < ReadersCount; i++)
    {
        pthread_join(ReadersArray[i], NULL);
    }

    pthread_join(LogThread,NULL);

free(ReadersArray);
free(WritersArray);

    

}