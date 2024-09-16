#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include 
#define MAXCOUNT 10



pthread_t stage[6] = {-1, -1, -1, -1, -1, -1};
pthread_t stageQueue[6] = {-1, -1, -1, -1, -1, -1};
pthread_t runningQueue[18];
pthread_t dressQueue[16];
pthread_t crossQueue[15];
pthread_t stageType;
int allowedThreads= 0;
pthread_mutex_t lock;
pthread_cond_t cond;
// pthread_signal_t cond;

// 18 running shoes
// 16 dress shoes
// 15 crossover shoes

typedef struct __lock_t {
    int flag;
    int guard;
    // queue_t *q;

} lock_t;

void init(lock_t mutex){
    mutex -> flag = 0
}

// typedef struct {
//     int shoeType;
//     int id;
// } myarg_t;

void *runningShoes(void *arg)
{
    // int thread_id = *((int *)arg);
    
 
    while (1)
    {
        // myarg_t *args = (myarg_t *) arg;
        printf("Running shoes #%d is running\n", arg);

        usleep(500000);
        // nanosleep(&request, &remaining);
        printf("Running: %d slept\n", arg);

        // tryToEnterStage('r', arg);

        printf("Running shoes #%d have left\n", arg);
    }
    return NULL;
}

void *dressShoes(void *arg)
{
    // int thread_id = *((int *)arg);
   
    while (1)
    {
        // myarg_t *args = (myarg_t *) arg;
        printf("Dress shoes #%d are running\n", arg);

        usleep(500000);
        // nanosleep(&request, &remaining);

        // tryToEnterStage('d', args->id);

        // wait rand time

        //trytoleave

        printf("Dress shoes #%d have left\n", arg);
    }
    return NULL;
}

void *crossoverShoes(void *arg)
{
    // int thread_id = *((int *)arg);
    
    while (1)
    {
        // myarg_t *args = (myarg_t *) arg;
        printf("Crossover shoes #%d are running\n", arg);

        usleep(500000);
        // nanosleep(&request, &remaining);

        // tryToEnterStage('c', args->id);

        printf("Crossover shoes #%d have left.\n", arg);
    }
    return NULL;
}


int try_to_enter(int id, int type){
    //check type on stage vs entered type
    if(type == stageType && allowedThreads != MAXCOUNT){
        //add to queue
            // if >= 2 objects in queue
                //send as many Shoes to stage as possible, preferably in FIFO order
                // wait X time
            //leave stage
    }
    else if(allowedThreads == MAXCOUNT){
        rotate_stage(type);
    }
    else{
        //sleep thread
    }




}

//decides what type of shoe is on stage
void rotate_stage(int type){
    //randomizer, randomize until it is not the input type
    int newType = rand() % 3;
    while (type == newType){
        int newType = rand() % 3;
    }
    stageType = newType; //change to rand generated number
}

int leave(){

}






int main(int argc, char *argv[])
{

    // get seed
    FILE *filename = fopen("seed.txt", "r");

    char ch;

    if (filename == NULL)
    {
        printf("Error: could not open file");
        return 1;
    }

    int seed;

    if (fscanf(filename, "%d", &seed) != 1)
    {
        fprintf(stderr, "Error reading file\n");
        fclose(filename);
        return 1;
    }

    fclose(filename);

    printf("seed is: %d\n", seed);

    srand(seed);

    pthread_t thread_id_r[18];
    // running shoes = 0
    for (int i = 0; i < 18; i++)
    {   
        // myarg_t args = {0, i};
        int result = pthread_create(&thread_id_r[i], NULL, runningShoes, i);
    }

    pthread_t thread_id_d[16];
    // dress shoes = 1
    for (int i = 0; i < 16; i++)
    {
        // myarg_t args = {1, i};
        int result = pthread_create(&thread_id_d[i], NULL, dressShoes, i);
    }

    pthread_t thread_id_c[15];
    // crossover shoes = 2
    for (int i = 0; i < 15; i++)
    {
        // myarg_t args = {2, i};
        int result = pthread_create(&thread_id_c[i], NULL, crossoverShoes, i);
    }

    while (1)
    {
        // usleep(100000);
    }
}