#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <time.h>

#define PILE 60

sem_t mutex;

// 4 worker groups to make sure only 1 worker works at once
sem_t sem_weighing;
sem_t sem_barcoding;
sem_t sem_measuring;
sem_t sem_jostling;

// 4 stations so only 1 can go in at once
sem_t sem_red;
sem_t sem_green;
sem_t sem_blue;
sem_t sem_yellow;

sem_t sem_bigOrder;

struct package
{
    int packageID;
    int steps[4];
    int stepNum;
};

int amtpackages = 0;

int ID_r = 0;
int ID_g = 0;
int ID_b = 0;
int ID_y = 0;

struct package PPP[PILE];

struct package getPackage();
void canIGo(char c, int ID);
void reserveTheFuture(struct package p, char c, int ID);
void grab(struct package p, char c, int ID);
void retire(char c, int ID, struct package p);

void initSemaphores()
{
    sem_init(&mutex, 0, 1);

    // stations
    sem_init(&sem_weighing, 0, 1);
    sem_init(&sem_barcoding, 0, 1);
    sem_init(&sem_measuring, 0, 1);
    sem_init(&sem_jostling, 0, 1);

    // workers groups
    sem_init(&sem_red, 0, 1);
    sem_init(&sem_green, 0, 1);
    sem_init(&sem_blue, 0, 1);
    sem_init(&sem_yellow, 0, 1);

    sem_init(&sem_bigOrder, 0, 1);
}

void *worker(void *arg)
{

    int typeAndID = *((int *)arg);
    free(arg);

    char c;
    int ID = typeAndID % 4;
    // red, green, blue, yellow
    // mod to make types rotates for fairness

    // lock
    sem_wait(&mutex);
    switch (ID)
    {
    case 0:
        c = 'r';
        ID = ID_r;
        ID_r++;
        break;
    case 1:
        c = 'g';
        ID = ID_g;
        ID_g++;
        break;
    case 2:
        c = 'b';
        ID = ID_b;
        ID_b++;
        break;
    case 3:
        c = 'y';
        ID = ID_y;
        ID_y++;
        break;
    default:
        break;
    }
    // unlcok
    sem_post(&mutex);

    printf("Worker of type %c, number: %d starting\n", c, ID);

    while (amtpackages > 0)
    {
        printf("Number of packages left to sort: %d\n", amtpackages);
        // wait on worker sem
        canIGo(c, ID);
        if (amtpackages < 1)
        {
            printf("ThreadID #%d is Breaking down\n", ID);
            amtpackages == -1;
            sem_post(&sem_red);
            sem_post(&sem_green);
            sem_post(&sem_yellow);
            sem_post(&sem_blue);
            break;
            // pthread_exit(NULL);
            // return NULL;
        }

        // get package
        struct package p = getPackage();

        // wait on spot

        reserveTheFuture(p, c, ID);

        grab(p, c, ID);

        // bring up values
        retire(c, ID, p);
    }

    int *value = 0;
    pthread_exit(NULL);

    return NULL;
}

void canIGo(char c, int ID)
{
    // wait on team semaphore
    if (amtpackages < 1)
    {
        printf("ThreadID #%d is Breaking down\n", ID);
        amtpackages == -1;
        pthread_exit(NULL);
        return;
    }
    switch (c)
    {
    case 'r':
        sem_wait(&sem_red);
        break;

    case 'g':
        sem_wait(&sem_green);
        break;

    case 'b':
        sem_wait(&sem_blue);
        break;

    case 'y':
        sem_wait(&sem_yellow);
        break;

    default:
        break;
    }

    printf("Worker %c:%d is going.\n", c, ID);
}

struct package getPackage()
{

    // lock
    sem_wait(&mutex);

    // get package from pile if not empty

    struct package current = PPP[amtpackages];
    amtpackages--;

    // unlock
    sem_post(&mutex);

    return current;
}

void reserveTheFuture(struct package p, char c, int ID)
{

    int value = 0;

    int lastBigOrder[4] = {0, 0, 0, 0};

    // if single if goes
    if (p.stepNum == 4)
    {
        // if 4 needs big lock
        sem_wait(&sem_bigOrder);
    }
    else if (p.stepNum > 1)
    {
        // if 2 or 3 check how many shared
        for (int i = 0; i < 4; i++)
        {
            if ((p.steps[i] + lastBigOrder[i]) == 2)
            {
                value++;
            }
        }

        // if share more than 1
        if (value >= 2)
        {
            sem_wait(&sem_bigOrder);
            p.stepNum == 4;
        }

        // set to last order
        for (int i = 0; i < 4; i++)
        {
            lastBigOrder[i] = p.steps[i];
        }
    }
}

void grab(struct package p, char c, int ID)
{
    // need to bring up value

    int lastStep = 0;

    // for the steps
    for (int i = 0; i < 5; i++)
    {

        if (i < 4)
        {
            printf("The value of package %d step %d: %d\n", p.packageID, i, p.steps[i]);
            if (p.steps[i] == 1)
            {
                // if step needs to be done

                // wait on step
                switch (i)
                {
                case 0:
                    sem_wait(&sem_weighing);
                    printf("%c:%d is weighing for %d\n", c, ID, i);
                    usleep(rand() % 1000);
                    break;

                case 1:
                    sem_wait(&sem_barcoding);
                    printf("%c:%d is barcoding for %d\n", c, ID, i);
                    usleep(rand() % 1000);
                    break;

                case 2:
                    sem_wait(&sem_measuring);
                    printf("%c:%d is measuring for %d\n", c, ID, i);
                    usleep(rand() % 1000);
                    break;

                case 3:
                    sem_wait(&sem_jostling);
                    printf("%c:%d is jostling for %d\n", c, ID, i);
                    usleep(rand() % 1000);
                    break;

                default:
                    break;
                }
            }
        }

        if (i > 0)
        {
            if (p.steps[lastStep] == 1)
            {
                // post step
                switch (lastStep)
                {
                case 0:
                    sem_post(&sem_weighing);
                    printf("%c:%d is done weighing for %d\n", c, ID, lastStep);
                    break;

                case 1:
                    sem_post(&sem_barcoding);
                    printf("%c:%d is done barcoding for %d\n", c, ID, lastStep);
                    break;

                case 2:
                    sem_post(&sem_measuring);
                    printf("%c:%d is done measuring for %d\n", c, ID, lastStep);
                    break;

                case 3:
                    sem_post(&sem_jostling);
                    printf("%c:%d is done jostling for %d\n", c, ID, lastStep);
                    break;

                default:
                    break;
                }
            }

            // complete 1 step
            p.steps[lastStep] = 0;
        }

        lastStep = i;
        printf("value of i is: %d\n", i);

        if (i == 5)
        {
            // if last step

            // free last order lock
            if (p.stepNum == 4)
            {
                printf("hello?");
                sem_post(&sem_bigOrder);
            }
        }
    }
}

void retire(char c, int ID, struct package p)
{

    // releasing team semaphore
    switch (c)
    {
    case 'r':
        sem_post(&sem_red);
        break;

    case 'g':
        sem_post(&sem_green);
        break;

    case 'b':
        sem_post(&sem_blue);
        break;

    case 'y':
        sem_post(&sem_yellow);
        break;

    default:
        break;
    }

    printf("A worker %c:%d, package %d retires.\n", c, ID, p.packageID);
}

void fillPile()
{

    int stepNum = 0;

    // lock
    sem_wait(&mutex);

    for (int i = 0; i < PILE; i++)
    {

        // create a package
        struct package new;
        printf("New package being made: %d.\n", new.packageID);
        new.packageID = (amtpackages + 1);
        for (int i = 0; i < 4; i++)
        {
            new.steps[i] = rand() % 2;
            printf("Value of array %d on creation: %d.\n", new.packageID, new.steps[i]);
            if (new.steps[i] == 1)
            {
                stepNum++;
            }
        }

        // add to pile
        PPP[i] = new;
        amtpackages++;
    }

    // unlock
    sem_post(&mutex);
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

    printf("Initializing.\n");
    initSemaphores();

    fillPile();

    printf("Creating threads.\n");
    pthread_t thread_id[60];

    // red, green, blue, yellow
    for (int i = 0; i < 60; i++)
    {
        int *arg = malloc(sizeof(int));
        if (arg == NULL)
        {
            perror("Failed to allocate memory");
            exit(EXIT_FAILURE);
        }
        *arg = i;
        int result = pthread_create(&thread_id[i], NULL, worker, arg);
        if (result != 0)
        {
            perror("Failed to create thread");
            exit(EXIT_FAILURE);
        }
    }

    printf("hello?\n");
    // red, green, blue, yellow
    for (int i = 0; i < 60; i++)
    {
        printf("joining on %d\n", i);
        pthread_join(thread_id[i], NULL);
    }

    sem_destroy(&sem_barcoding);
    sem_destroy(&sem_weighing);
    sem_destroy(&sem_measuring);
    sem_destroy(&sem_jostling);

    sem_destroy(&mutex);
    sem_destroy(&sem_bigOrder);

    sem_destroy(&sem_red);
    sem_destroy(&sem_green);
    sem_destroy(&sem_blue);
    sem_destroy(&sem_yellow);
}
