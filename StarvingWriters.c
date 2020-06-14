#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
/// The constant number of seconds spent in resource
#define WAITCONST 10 

//RESOURCE SEMAPHORE
sem_t ResourceSem;

static volatile  ushort exitFlag = 1;

sem_t MutexSem;

pthread_cond_t WriteNow = PTHREAD_COND_INITIALIZER;

pthread_mutex_t MainLock = PTHREAD_MUTEX_INITIALIZER;

int ReadersCount = 0;

int ReadersQueue = 0;

int WritersCount = 0;

int WritersQueue = 0;

void *Logger()
{
    //lock this section od code
        pthread_mutex_lock(&MainLock);
    while(exitFlag == 1)
    {
        pthread_cond_wait(&WriteNow, &MainLock); // wait for signal
        printf("\nReadersQ:%d WritersQ:%d [in:R:%d W:%d]", ReadersQueue,WritersQueue,ReadersCount,WritersCount); //When signalled print relevant output
    }
        pthread_mutex_unlock(&MainLock);// unlock resource
//pthread_exit(NULL);

}

///
/// Function for writer thread
///

void *Writer()
{

    /// Writers try to enter and exit indefinetly
    while(exitFlag == 1)
    {
        /// Entering procedure
    sem_wait(&ResourceSem);
    WritersQueue--;            //// Writers exit queue and enters into resource
    WritersCount++;
    pthread_cond_signal(&WriteNow); //signals writing thread
    //printf("\nWriter aquired the lock");

    int sleepS = rand()%WAITCONST; 
    /// Does stuff ( in this case just waits)
    sleep(sleepS);
    WritersCount--;  //Writer exits room nad enters queue
    WritersQueue++;
    pthread_cond_signal(&WriteNow); //signals logging thread
    sem_post(&ResourceSem);
   // printf("\nWriter released the lock");
  // sleep(sleepS);
    }
pthread_exit(NULL);
}


///
/// The function for Reader thread
///

void *Reader()
{
    int sleepS = 0;
    while(exitFlag == 1)
    {
    //Readers Entry Section (Only one reader fits the door :VVVV)
    sem_wait(&MutexSem);
    ReadersQueue--; //Readers exit queue and enter room
    ReadersCount++;
    //printf("\nReader Entered room Count is %d",ReadersCount);
    pthread_cond_signal(&WriteNow); // signals logging thread
    if(ReadersCount == 1 )
    {
        sem_wait(&ResourceSem); // locks reasource from writers
    }
    sem_post(&MutexSem);

    //Simulating reading

         sleepS = rand()%WAITCONST;

        sleep(sleepS);

    //Readers exit section 

    sem_wait(&MutexSem); // lock access for other readers (Again one reader fits the door)
    ReadersCount--; //Readers exit queue and enter room
    ReadersQueue++;
    pthread_cond_signal(&WriteNow); // sginals logging thread
    if(ReadersCount == 0 )
    {
        sem_post(&ResourceSem);
    }
    sem_post(&MutexSem);
    //sleep(sleepS);
    }
    //printf("\nReader exited room Count is %d",ReadersCount);
  //  pthread_exit(NULL);
}

void SetExitFlag()
{
exitFlag = 0;
}

///
///param  argv[1] - number of readers
///param  argv[2] - number of writers
///

int main(int argc, char **args)
{

     if(argc != 3 )
 {
     printf("User failed to provide valid arguments, please provide arguments in following order: ReadersCount Writerscount\nRunning with default R:10 W:5");
 } 

    ///Variable initialization
const int READERSCOUNT = argc >= 3 ?  atoi(args[1]) : 10;
const int WRITERSCOUNT = argc >= 3 ?  atoi(args[2]) : 5;
ReadersQueue = READERSCOUNT;
WritersQueue = WRITERSCOUNT;
sem_init(&ResourceSem,0,1);
sem_init(&MutexSem,0,1);

srand(5);
    
  
/// allocating dynamic data structures 
pthread_t *WritersArray = (pthread_t*)malloc(WRITERSCOUNT*(sizeof(pthread_t)));
pthread_t *ReadersArray = (pthread_t*)malloc(READERSCOUNT*(sizeof(pthread_t)));
pthread_t LogThread;
int i = 0;

signal(SIGINT,SetExitFlag);

/// logger thread creation 
pthread_create(&LogThread,NULL,Logger,NULL);

    ///Readers' threads
    for(i = 0 ; i < READERSCOUNT; i++)
    {
        pthread_create(&ReadersArray[i],NULL,Reader,NULL);
    }
    ///Writers' threads
    for(i = 0 ; i < WRITERSCOUNT; i++)
    {
         pthread_create(&WritersArray[i],NULL,Writer,NULL);
    }
    ///Wait for threads to finish
    for(i = 0 ; i < WRITERSCOUNT; i++)
    {
        pthread_join(WritersArray[i], NULL);
    }
    i = 0 ; // double check if variable gets zeroed

      for(i = 0 ; i < READERSCOUNT; i++)
    {
        pthread_join(ReadersArray[i], NULL);
    }

pthread_join(LogThread,NULL);
// Dealloc operations
free(ReadersArray);
free(WritersArray);

//pthread_exit(0)
}