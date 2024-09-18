#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#define MAXCOUNT 8
#define MINSLEEP 200000
#define MAXSLEEP 500000

int stage[6] = {-1, -1, -1, -1, -1, -1};

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_enter = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_wake = PTHREAD_COND_INITIALIZER;
int allowedThreads = 0;
int stageType;
int numSleepingForStage = 0;

void enter_stage(int, int);
void try_to_enter(int, int);
void leaveStage(int, int);
void rotate_stage();

void *runningShoes(void *arg)
{
    int id = *((int *)arg);
    free(arg);

    while (1)
    {
        printf("Running shoes #%d is running\n", id);
        pthread_mutex_lock(&lock);
        try_to_enter(id, 0);

        int rand_sleep = MINSLEEP + rand() % (MAXSLEEP - MINSLEEP + 1);
        printf("Running Shoes #%d Sleeping on stage for %d microseconds\n", id, rand_sleep);
        usleep(rand_sleep);

        leaveStage(id, 0);

        rand_sleep = MINSLEEP + rand() % (MAXSLEEP - MINSLEEP + 1);
        printf("Running Shoes #%d Sleeping leaving stage for %d microseconds\n", id, rand_sleep);
        usleep(rand_sleep);
    }
    return NULL;
}

void *dressShoes(void *arg)
{
    int id = *((int *)arg);
    free(arg);

    while (1)
    {
        printf("Dress shoes #%d are running\n", id);
        pthread_mutex_lock(&lock);
        try_to_enter(id, 1);

        int rand_sleep = MINSLEEP + rand() % (MAXSLEEP - MINSLEEP + 1);
        printf("Dress Shoes #%d Sleeping on stage for %d microseconds\n", id, rand_sleep);
        usleep(rand_sleep);

        leaveStage(id, 1);

        rand_sleep = MINSLEEP + rand() % (MAXSLEEP - MINSLEEP + 1);
        printf("Dress Shoes #%d Sleeping leaving stage for %d microseconds\n", id, rand_sleep);
        usleep(rand_sleep);
    }
    return NULL;
}

void *crossoverShoes(void *arg)
{
    int id = *((int *)arg);
    free(arg);

    while (1)
    {
        printf("Crossover shoes #%d are running\n", id);
        pthread_mutex_lock(&lock);
        try_to_enter(id, 2);

        int rand_sleep = MINSLEEP + rand() % (MAXSLEEP - MINSLEEP + 1);
        printf("Crossover Shoes #%d Sleeping on stage for %d microseconds\n", id, rand_sleep);
        usleep(rand_sleep);

        leaveStage(id, 2);

        rand_sleep = MINSLEEP + rand() % (MAXSLEEP - MINSLEEP + 1);
        printf("Crossover Shoes #%d Sleeping leaving stage for %d microseconds\n", id, rand_sleep);
        usleep(rand_sleep);
    }
    return NULL;
}

void try_to_enter(int id, int type)
{
    switch (type)
    {
    case 0:
        printf("Running Shoe %d is trying to enter the stage\n", id);
        break;
    case 1:
        printf("Dress Shoe %d is trying to enter the stage\n", id);
        break;
    case 2:
        printf("Crossover Shoe %d is trying to enter the stage\n", id);
        break;
    }
    if (type == stageType && allowedThreads < MAXCOUNT)
    {

        int onStage = 0;
        for (int i = 0; i < 6; i++)
        {
            if (stage[i] != -1)
            {
                onStage++;
            }
        }
        printf("There are %d shoes waiting\n", numSleepingForStage);
        printf("There are %d shoes on stage\n", onStage);

        if (numSleepingForStage > 1 && onStage < 5 && allowedThreads < MAXCOUNT) // add to stage if possible
        {
            numSleepingForStage++;
            switch (type)
            {
            case 0:
                printf("Running Shoe %d is entering stage and signaling another Running Shoe!\n", id);
                break;
            case 1:
                printf("Dress Shoe %d is entering stage and signaling another Dress Shoe!\n", id);
                break;
            case 2:
                printf("Crossover Shoe %d is entering stage and signaling another Cross Shoe!\n", id);
                break;
            }
            pthread_cond_signal(&cond_enter);
            enter_stage(id, type);
        }
        else // sleep until woken
        {
            numSleepingForStage++;
            switch (type)
            {
            case 0:
                printf("Running Shoe %d is sleeping until signaled to enter stage\n", id);
                break;
            case 1:
                printf("Dress Shoe %d is sleeping until signaled to enter stage\n", id);
                break;
            case 2:
                printf("Crossover Shoe %d is sleeping until signaled to enter stage\n", id);
                break;
            }
            // printf("Shoe %d, Type %d will sleep now, until ready to enter stage\n", id, type);
            pthread_mutex_unlock(&lock);
            pthread_cond_wait(&cond_enter, &lock);
            if (type == stageType)
            {
                switch (type)
                {
                case 0:
                    printf("Running Shoe %d is waking up to try and enter stage!\n", id);
                    break;
                case 1:
                    printf("Dress Shoe %d is waking up to try and enter stage!\n", id);
                    break;
                case 2:
                    printf("Crossover Shoe %d is waking up to try and enter stage!\n", id);
                    break;
                }
                // printf("Shoe %d, Type %d, Waking up to try and enter stage!\n", id, type);
                enter_stage(id, type);
            }
            pthread_mutex_unlock(&lock);
        }
    }
    else if (allowedThreads >= MAXCOUNT && type == stageType)
    {
        int onStage = 0;
        for (int i = 0; i < 6; i++)
        {
            if (stage[i] != -1)
            {
                onStage++;
            }
        }
        if (onStage == 0)
        {
            printf("Rotating for Fairness\n");
            rotate_stage();
            pthread_mutex_unlock(&lock);
        }
        else
        {

            pthread_mutex_unlock(&lock);
            pthread_cond_wait(&cond_wake, &lock);
            try_to_enter(id, type);
        }
    }
    else
    {
        // while (ready == 0)
        // {
        switch (type)
        {
        case 0:
            printf("Running Shoe %d is sleeping until stage rotates!\n", id);
            break;
        case 1:
            printf("Dress Shoe %d is sleeping until stage rotates!\n", id);
            break;
        case 2:
            printf("Crossover Shoe %d sleeping until stage rotates!\n", id);
            break;
        }
        // printf("sleeping %d, %d because of incorrect type on stage\n", id, type);
        pthread_cond_wait(&cond_wake, &lock);
        try_to_enter(id, type);
    }
}

void enter_stage(int id, int type)
{
    // pthread_mutex_lock(&lock);

    if (type == stageType && allowedThreads < MAXCOUNT)
    {
        for (int i = 0; i < 6; i++)
        {
            if (stage[i] == -1)
            {
                numSleepingForStage--;
                stage[i] = id;

                switch (type)
                {
                case 0:
                    printf("Running Shoe #%d is on shoebox #%d!\n", id, i);
                    break;
                case 1:
                    printf("Dress Shoe #%d is on shoebox #%d!\n", id, i);
                    break;
                case 2:
                    printf("Crossover Shoe #%d is on shoebox #%d!\n", id, i);
                    break;
                }
                allowedThreads++;
                printf("Number of allowed Shoes so far is %d\n", allowedThreads);
                break;
            }
        }
    }
    pthread_mutex_unlock(&lock);
}

// decides what type of shoe is on stage
void rotate_stage()
{
    printf("Resetting global variables\n");
    allowedThreads = 0;
    numSleepingForStage = 0;
    int newType = rand() % 3;
    while (stageType == newType)
    {
        newType = rand() % 3;
    }
    stageType = newType; // change to rand generated number
    switch (stageType)
    {
    case 0:
        printf("Running Shoes have the Stage!\n");
        break;
    case 1:
        printf("Dress Shoes have the Stage!\n");
        break;
    case 2:
        printf("Crossover Shoes have the Stage!\n");
        break;
    }
    printf("Broadcasting to all sleeping threads\n");
    pthread_cond_broadcast(&cond_enter);
    pthread_cond_broadcast(&cond_wake);
    pthread_mutex_unlock(&lock);
}

void leaveStage(int id, int type)
{
    pthread_mutex_lock(&lock);

    for (int i = 0; i < 6; i++)
    {
        if (stage[i] == id)
        {
            stage[i] = -1;
            switch (type)
            {
            case 0:
                printf("Running Shoe #%d has left the stage! Box #%d is free now.\n", id, i);
                break;
            case 1:
                printf("Dress Shoe #%d has left the stage! Box #%d is free now.\n", id, i);
                break;
            case 2:
                printf("Crossover Shoe #%d has left the stage! Box #%d is free now.\n", id, i);
                break;
            }
            break;
        }
    }

    pthread_mutex_unlock(&lock);
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

    stageType = rand() % 3;
    printf("stage type is %d\n", stageType);

    for (int i = 0; i < 18; i++)
    {
        // myarg_t args = {0, i};
        int *arg = malloc(sizeof(int));
        *arg = i;
        int result = pthread_create(&thread_id_r[i], NULL, runningShoes, arg);
    }

    pthread_t thread_id_d[16];
    // dress shoes = 1
    for (int i = 0; i < 16; i++)
    {
        // myarg_t args = {1, i};
        int *arg = malloc(sizeof(int));
        *arg = i;
        int result = pthread_create(&thread_id_d[i], NULL, dressShoes, arg);
    }

    pthread_t thread_id_c[15];
    // crossover shoes = 2
    for (int i = 0; i < 15; i++)
    {
        // myarg_t args = {2, i};
        int *arg = malloc(sizeof(int));
        *arg = i;
        int result = pthread_create(&thread_id_c[i], NULL, crossoverShoes, arg);
    }

    while (1)
    {
        // usleep(100000);
    }
}