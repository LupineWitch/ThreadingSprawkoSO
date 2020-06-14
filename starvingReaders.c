#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>

static volatile ushort exitFlag  = 1 ;

int readersCount;           /// Count of writers
int writersCount;           /// Count of writers
int readersCountIn = 0;     /// Number of readers needing shared resources
int writersCountIn = 0;     /// Number of writers needing shared resources
int writersImp = 0;         /// Critical writers

/// Initializing mutexes
pthread_mutex_t resourceMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t checkResourceMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t readerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t writerMutex = PTHREAD_MUTEX_INITIALIZER;
 
void *Writer(void *arg) 
{
    int *number = (int*)arg;
    while (exitFlag == 1) 
    {
        pthread_mutex_lock(&writerMutex);
        writersCountIn++;

        /// Locking out readers
        if (writersCountIn == 1) 
        {
            pthread_mutex_lock(&checkResourceMutex);
        }

        pthread_mutex_unlock(&writerMutex);
        pthread_mutex_lock(&resourceMutex);
        writersImp++;
        printf("WRITER ID %d WALKED IN \n",*number);
        printf("ReaderQ: %d , WriterQ: %d [in: R:%d W:%d]\n",readersCount-readersCountIn,writersCount-writersImp,readersCountIn,writersImp);
        pthread_mutex_unlock(&resourceMutex);
        writersImp--;
        sleep(2);
        printf("WRITER ID %d WALKED OUT \n",*number);
        printf("ReaderQ: %d , WriterQ: %d [in: R:%d W:%d]\n",readersCount-readersCountIn,writersCount-writersImp,readersCountIn,writersImp);
        pthread_mutex_lock(&writerMutex);
        writersCountIn--;

        /// Unlocking readers
        if (writersCountIn == 0) 
        {
            pthread_mutex_unlock(&checkResourceMutex);
        }
        pthread_mutex_unlock(&writerMutex);
    }
    return 0;
}

void* Reader(void* arg)
{
    int* number = (int*)arg;
    while (exitFlag == 1) 
    {
        /// Reader trying to enter
        pthread_mutex_lock(&checkResourceMutex);

        pthread_mutex_lock(&readerMutex);
        readersCountIn++; 

        /// Locking out writers
        if (readersCountIn == 1)
        {
            pthread_mutex_lock(&resourceMutex);
        }

        printf("READER %d ENTERED \n", *number);
        printf("ReaderQ: %d , WriterQ: %d [in: R:%d W:%d]\n", readersCount - readersCountIn, writersCount - writersImp, readersCountIn, writersImp);
        pthread_mutex_unlock(&readerMutex);
        pthread_mutex_unlock(&checkResourceMutex);
        sleep(2);
        pthread_mutex_lock(&readerMutex);
        readersCountIn--;

        // Unlocking writers
        if (readersCountIn == 0)
        {
            pthread_mutex_unlock(&resourceMutex);
        }

        printf("READER %d ENTERED \n", *number);
        printf("ReaderQ: %d , WriterQ: %d [in: R:%d W:%d]\n", readersCount - readersCountIn, writersCount - writersImp, readersCountIn, writersImp);
        pthread_mutex_unlock(&readerMutex);
    }
    return 0;
}
 
void exitRoutine()
{
    exitFlag = 0 ;
}

int main(int argc, char *argv[]) 
{
    /// error - not enough arguments
    if(argc < 3)
    {
        printf("Podano %d argumentow, jest to ilosc niewystarczajaca", argc);
        return -1;
    }
    /// array to integer
    readersCount = atoi(argv[1]);
    writersCount = atoi(argv[2]);
    
    signal(SIGINT,exitRoutine);


    // Initialize arrays of threads IDs
    pthread_t *readersThreads = malloc(readersCount * sizeof(pthread_t));
    pthread_t *writersThreads = malloc(readersCount * sizeof(pthread_t));
 
    // Initialize shared memory (array) with random numbers
 
    // Creating theads of readers
    for (int i = 0; i < readersCount; ++i) 
    {
        int id = i;
        //*id = i;
        pthread_create(&readersThreads[i], NULL, Reader,(void*)&id);
    }

    /// Creating theads of writers
    for (int i = 0; i < writersCount; ++i) 
    {
        int id =  i;
        //*id = i;
        pthread_create(&writersThreads[i], NULL, Writer, (void*)&id);
        
    }
 
    /// Waiting until readers finish
    for (int i = 0; i < readersCount; ++i) 
    {
        pthread_join(readersThreads[i], NULL);
    }

    /// Waiting until writers finish
    for (int i = 0; i < writersCount; ++i) 
    {
        pthread_join(writersThreads[i], NULL);
    }
 
    /// Freeing up allocated space
    free(readersThreads);
    free(writersThreads);
    
    pthread_mutex_destroy(&resourceMutex);
    pthread_mutex_destroy(&checkResourceMutex);
    pthread_mutex_destroy(&readerMutex);
    pthread_mutex_destroy(&writerMutex);

    return 0;
}