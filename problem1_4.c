#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

void enterStage(char c, int id);

sem_t mutex;
sem_t sem_stage;
sem_t sem_running;
sem_t sem_crossover;
sem_t sem_dress;

pthread_t stage[6];
pthread_t stageQueue[6];

// 18 running shoes
// 16 dress shoes
// 15 crossover shoes

void initSemaphores()
{
    sem_init(&mutex, 0, 1);
    sem_init(&sem_stage, 0, 6);
    sem_init(&sem_running, 0, 1);
    sem_init(&sem_crossover, 0, 1);
    sem_init(&sem_dress, 0, 1);
}

void *runningShoes(int *arg)
{
    // int thread_id = *((int *)arg);

    while (1)
    {
        printf("Running shoes #%d is running\n", arg);

        usleep(100000);

        printf("Running: slept\n", arg);

        tryToEnterStage('r', arg);

        printf("Running shoes #%d have left\n", arg);


    }
    return NULL;
}

void *dressShoes(int *arg)
{
    // int thread_id = *((int *)arg);
    while (1)
    {
        printf("Dress shoes #%d are running\n", arg);

        usleep(100000);

        tryToEnterStage('d', arg);

        printf("Dress shoes #%d have left\n", arg);

    }
    return NULL;
}

void *crossoverShoes(int *arg)
{
    // int thread_id = *((int *)arg);
    while (1)
    {
        printf("Crossover shoes #%d are running\n", arg);

        usleep(100000);

        tryToEnterStage('c', arg);

        printf("Crossover shoes #%d have left.\n", arg);
    }
    return NULL;
}

void tryToEnterStage(char c, int id)
{
    // sem indicates how many spots available
    // sem_wait(&sem_stage);

    printf("Waiting on lock to try for stage.\n");
    sem_wait(&mutex);
    printf("Gained lock\n");

    int stage = 0;
    sem_getvalue(&sem_stage, &stage);
    if (stage == 0) {
       sem_post(&mutex);
       printf("I, %c%d, was forced to give up lock.\n", c, id); 
    }

    if (c == 'r')
    {
        printf("Running: Waiting on lock.\n");
        // goes on stage
        sem_wait(&sem_running);

        printf("Running: Waiting on stage.\n");
        // sem indicates how many spots available
        sem_wait(&sem_stage);

        int spaceLeft = 0;
        sem_getvalue(&sem_running, &spaceLeft);
        if (spaceLeft == 5)
        {
            // lock other two
            printf("Running: Locking stage.\n");
            sem_wait(&sem_crossover);
            sem_wait(&sem_dress);
        }

        sem_post(&sem_running);
        sem_post(&mutex);

        printf("Running: Entering stage.\n");
        enterStage(c, id);
    }
    else if (c == 'd')
    {
        printf("Dress shoes: Waiting on lock.\n");
        // goes on stage
        sem_wait(&sem_dress);

        printf("Dress shoes: Waiting on stage.\n");
        // sem indicates how many spots available
        sem_wait(&sem_stage);

        int spaceLeft = 0;
        sem_getvalue(&sem_dress, &spaceLeft);
        if (spaceLeft == 5)
        {
            // lock other two
            printf("Dress shoes: Locking stage.\n");
            sem_wait(&sem_crossover);
            sem_wait(&sem_running);
        }

        sem_post(&sem_dress);
        sem_post(&mutex);

        printf("Dress shoes: Entering stage.\n");
        enterStage(c, id);
    }
    else if (c == 'c')
    {
        printf("Running: Waiting on lock.\n");
        // goes on stage
        sem_wait(&sem_crossover);

        printf("Running: Waiting on stage.\n");
        // sem indicates how many spots available
        sem_wait(&sem_stage);

        int spaceLeft = 0;
        sem_getvalue(&sem_crossover, &spaceLeft);
        if (spaceLeft == 5)
        {
            // lock other two
            printf("Running: Locking stage.\n");
            sem_wait(&sem_running);
            sem_wait(&sem_dress);
        }

        sem_post(&sem_crossover);
        sem_post(&mutex);

        printf("Running: Entering stage.\n");
        enterStage(c, id);
        sem_post(&mutex);
    }
}

void enterStage(char c, int id)
{

    printf("Stage %c: Waiting on lock.\n", c);
    sem_wait(&mutex);
    // gets on stage
    int index;
    for (int i = 0; i < 6; i++)
    {
        if (stage[i] == NULL)
        {
            stage[i] = id;
            index = i;
            // index = pthread_self - should be i
            break;
        }
    }
    //check if queuestage has more than 2 shoes
    // if has more than two shoes, send up as many as you can
    //leave

    printf("I, %c %d am on shoebox #%d!\n", c, id, index);

    sem_post(&mutex);

    // sleep
    //usleep(100000);

    leaveStage(c, index);
}

void leaveStage(char c, int i)
{

    sem_wait(&mutex);
    // remove from stage
    stage[i] = NULL;

    switch (c)
    {

    case 'r':
        printf("Running: Unlocking stage.\n");
        sem_post(&sem_dress);
        sem_post(&sem_crossover);
        break;

    case 'd':
        ("Dress: Unlocking stage.\n");
        sem_post(&sem_running);
        sem_post(&sem_crossover);
        break;

    case 'c':
        ("Crossover: Unlocking stage.\n");
        sem_post(&sem_dress);
        sem_post(&sem_running);
        break;

    default:
        break;
    }

    printf("A %c is leaving!\n", c);

    sem_post(&sem_stage);

    sem_post(&mutex);

    // sleep
    usleep(100000);

    // try again
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

    initSemaphores();

    pthread_t thread_id_r[18];
    // running shoes
    for (int i = 0; i < 18; i++)
    {

        int result = pthread_create(&thread_id_r[i], NULL, runningShoes, i);
    }

    pthread_t thread_id_d[16];
    // dress shoes
    for (int i = 0; i < 16; i++)
    {

        int result = pthread_create(&thread_id_d[i] + 18, NULL, dressShoes, i);
    }

    pthread_t thread_id_c[15];
    // crossover shoes
    for (int i = 0; i < 15; i++)
    {

        int result = pthread_create(&thread_id_c[i] + 34, NULL, crossoverShoes, i);
    }
}