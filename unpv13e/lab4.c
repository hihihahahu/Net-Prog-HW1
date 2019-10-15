#include <stdio.h>
#include <pthread.h> 
#include <stdlib.h>

struct adds
{
    long a1, a2;
};

void* add(void* v)
{
    struct adds* adder = (struct adds*)v;
    if (adder -> a2 == 0) {return (void*)adder -> a1;}
    else
    {
        adder -> a2 = adder -> a2 - 1;
        return (void*)(add((void*)adder) + 1);
    }
}

int main(int argc, char* argv[])
{
    int NUM_CHILD = atoi(argv[1]);
    pthread_t children[NUM_CHILD][NUM_CHILD];
    struct adds* adders[NUM_CHILD][NUM_CHILD];
    
    for (long i = 1; i < NUM_CHILD; i++)
    {
        for (long j = 0; j < NUM_CHILD; j++)
        {
            pthread_t tid;
            adders[i][j] = (struct adds*)malloc(sizeof(struct adds));
            adders[i][j] -> a1 = i;
            adders[i][j] -> a2 = j + 1;
             printf("Main starting thread add() for [%ld + %ld]\n", i, j + 1);
            int val = pthread_create(&tid, NULL, add, (void*)adders[i][j]);
            if (val < 0) {return -1;}
            else
            {
                children[i][j] = tid;
                printf("Thread %li running add() with [%ld + %ld]\n", (unsigned long int)tid, i, j + 1);
            }
        }
    }
    
    for (int i = 1; i < NUM_CHILD; i++)
    {
        for (int j = 0; j < NUM_CHILD; j++)
        {
            int *ret_val;
            pthread_join(children[i][j], (void**)&ret_val);
            printf("In main, collecting thread %li computed [%d + %d] = %d\n",
                    (unsigned long int)children[i][j], i, j + 1,(int)ret_val);
            free(adders[i][j]);
        }
    }
    

    return 0;
}
