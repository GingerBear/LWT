#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "lwt.h"
#include "queue.h"

#define N 50
#define QUANTUM 100

int current = 0;
int buffer[N];

smphr *emptyCount;
smphr *fullCount;
smphr *useQueue;

void producer(){
    srand(time(NULL));  // Intializes random number generator
    int item = 0;
    while(1){        
        item = rand();
        P(emptyCount);
        P(useQueue);
        if (current < N) {
            buffer[current++] = item;
        }
        V(useQueue);
        V(fullCount);
        printf("Producer: %s\tproduce: %d\te: %d\tf: %d\tSize:%d\n"
            , get_curr_name()
            , item
            , smphr_get_size(emptyCount)
            , smphr_get_size(fullCount)
            , current);
    }
}

void consumer(){
    int item = 0;
    int i = 0;
    while(i < 30){ 
        P(fullCount);
        P(useQueue);
        if (current > 0) {
            item = buffer[--current];
        }
        V(useQueue);
        V(emptyCount);
        printf("Consumer: %s\tconsume: %d\te: %d\tf: %d\tSize:%d\n"
            , get_curr_name()
            , item
            , smphr_get_size(emptyCount)
            , smphr_get_size(fullCount)
            , current);
        i++;
    }
}

int main(int argc, char *argv[]){
    int i,j;
    lwt *t;
    emptyCount  = smphr_create(N);
    fullCount   = smphr_create(0);
    useQueue    = smphr_create(1);

    lwt_init(QUANTUM);

    lwt_create("p1", 0, NULL, producer);  
    lwt_create("p2", 0, NULL, producer);  
    lwt_create("p3", 0, NULL, producer);  
    lwt_create("p4", 0, NULL, producer);  
    lwt_create("c1", 0, NULL, consumer);     
    t = lwt_create("c2", 0, NULL, consumer);     

    thrd_wait(t);
    printf("Two Consumers consume 30 items!\n");
    printf("Done!\n");
}



