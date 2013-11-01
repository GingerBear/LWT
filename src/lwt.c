#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "lwt.h"
#include "queue.h"

enum lwt_state_t {
    lwt_READY, thrd_WAIT, sem_WAIT, lwt_SLEEP, lwt_EXIT
};

struct lwt_t {
    char        * name;
    int         tid;
    char        * stack;        // store new stack 
    int         prior;          // number to adjust quantum, more prior get more runing time, 0 default
    int         status;         // for wating thread, or last thread of system
    jmp_buf     env;            // env for setjmp/longjmp
    lwt_state   state;
    lwt_fn      fn;
    int         argc;
    void        * argv;
    lwt         * next;
    lwt         * child;        // child thread, its exit ends parent thread's wait.
    clock_t     sleep_time;     // sleep till sleep_time
};

struct sys_lwt_t {
    lwt         * curr;
    lwt         * main;
    int         sid;
    useconds_t  interval;       // thread's running interval, set by quantum, adjucted by prior
};

struct smphr_t {
    int     smphr_size;
    Queue   queue;              // queue store thread waiting for semophore
};

sys_lwt sys_t;                  // declare the syetem thread

void lwt_init (int lwt_quantum) 
{
    lwt *t;
    t           = (lwt *)malloc( sizeof(lwt) );
    t->name     = "main";               // set up inital thread, which is main
    t->tid      = 0;
    t->stack    = NULL;
    t->prior    = 0;
    t->state    = lwt_READY;
    t->fn       = NULL;
    t->argc     = 0;
    t->argv     = NULL;
    t->next     = t;                    // set next to otself
    t->status   = 0;

    sys_t.curr = t;                     // set system current thread to main
    sys_t.main = t;
    sys_t.sid = 0;
    sys_t.interval = lwt_quantum;

    signal(SIGALRM, alarm_handler);     // set up alarm signal handler
    ualarm(lwt_quantum, 0);             // set up the first alarm signal
    return;
}

lwt * lwt_create (char *name, int argc, void *argv, lwt_fn fn) 
{
    lwt *t;
    t           = (lwt *)malloc( sizeof(lwt) );
    t->name     = name;
    t->tid      = (++sys_t.sid);                // set incremental id of new created thread 
    t->stack    = (char *)malloc( lwt_STACK_SIZE );
    t->prior    = 0;
    t->state    = lwt_READY;
    t->fn       = fn;
    t->argc     = argc;
    t->argv     = argv;
    t->status   = 0;
    
    t->next     = sys_t.main->next;             // inset into linked list enqueue
    sys_t.main->next = t;

    if (setjmp(t->env) == 0) {                  // save thread running env by setjmp
        
        if (setjmp(sys_t.curr->env) == 0) {     // save main thread running env
            sys_t.curr = t;                     // make new created thread system current
            longjmp(t->env, 1);                 // go back to last setjmp, namely run the thread immediately 
        } else 
            return t;                           // continue run main thread
    } else {

        // Frankly, never type one line assmbely language before. So basically what the following 
        // code does is moving new malloc-ed stack outside from current stack to another stack. 
        // Reason to do this is to avoid stacks mess with each when perform a longjmp, if they 
        // are not moved to somewhere else. 
        #ifdef _X86
        __asm__("mov %0, %%esp;": :"r"(sys_t.curr->stack + lwt_STACK_SIZE) );
        #else
        __asm__("mov %0, %%rsp;": :"r"(sys_t.curr->stack + lwt_STACK_SIZE) );
        #endif

        sys_t.curr->fn(sys_t.curr->argv);       // run the thread
        sys_t.curr->state = lwt_EXIT;           // change the thread state to EXIT after run
        ualarm(0,0);                            // reset and raise to the handler
        raise(SIGALRM);
    }

    return t; 
}

void alarm_handler () 
{
    if (setjmp(sys_t.curr->env) == 0) {         // save current thread running env
        do {                                    
            sys_t.curr = sys_t.curr->next;      // switch to next READY thread
            switch (sys_t.curr->state) {
            case thrd_WAIT:
                _check_ready(sys_t.curr);       // if wait, get READY if its child thread EXIT
                break;
            case lwt_SLEEP:
                _check_awake(sys_t.curr);       // if sleep, get READY if sleep time passed
                break;
            default:
                break;
            }
        } while (sys_t.curr->state != lwt_READY);

        // Not sure if this can be called a priority mechanism,
        // but basically, it change the next alarm time.
        // So, if thread get higher prior, it will run longer.
        ualarm(sys_t.interval + sys_t.curr->prior, 0);
        longjmp(sys_t.curr->env, 1);
    }
}

void _check_ready (lwt *t) {
    if (t->child->state == lwt_EXIT)            // if thread's child thread exit
        t->state = lwt_READY;                   // change its state READY
}

void _check_awake (lwt *t) {    
    if (clock() > sys_t.curr->sleep_time)       // if runing time over sleep_time
        sys_t.curr->state = lwt_READY;          // change its state READY
}

void lwt_sleep (int s) {
    sys_t.curr->state = lwt_SLEEP;              
    sys_t.curr->sleep_time = 
        clock() + s * CLOCKS_PER_SEC;           // set up sleep_time (actually wake time)
    raise(SIGALRM);                             // raisn signal immediately
}

int thrd_wait (lwt *t) {
    sys_t.curr->state = thrd_WAIT;
    sys_t.curr->child = t;                      // make thread as current thread's child thread
    raise(SIGALRM);
    return t->status;
}

void lwt_exit (int status) {
    sys_t.curr->status = status;                // leave a status when exit for parent thread or system exit
    if(sys_t.curr == sys_t.curr->next)          // real exit if current thread is last
        exit(status);
    sys_t.curr->state = lwt_EXIT;
    raise(SIGALRM);
}

void lwt_del(lwt *t){
    lwt *pre = t;
    if (t == t->next)                           // if main thread, do nothing
        return;
    while (pre->next != t)                      // find previous thread in linked-list
        pre = pre->next;
    pre->next = t->next;                        // del node in linked-list
    free(t);                                    // free malloc
}

char * get_curr_name() {
    return sys_t.curr->name;
}

smphr * smphr_create(int smphr_size){
    smphr *s;
    s = malloc(sizeof(smphr));
    s->smphr_size = smphr_size;
    s->queue = CreateQueue();                   // use a queue to store waiting list
    return s;
}

int smphr_get_size(smphr *s){
    return s->smphr_size;
}

void P(smphr *s){
    int left = ualarm(0, 0);                    // disable the context switch, by clear the alarm
    if(s->smphr_size > 0){                      
        s->smphr_size -= 1;                     // semaphore offered
        ualarm(left, 0);                        // restore context switch
    } else {                                    // no more semaphore, blocked
        s->smphr_size -= 1;                     // TRICK! semaphore still need to be reduced one, in order to avoid fullCount getting too large. 
        sys_t.curr->state = sem_WAIT;
        Enqueue(sys_t.curr, s->queue);          // put into queue
        raise(SIGALRM);
    }
}

void V(smphr *s){
    int left = ualarm(0, 0);
    s->smphr_size += 1;                         // release semaphore
    if (!IsEmpty(s->queue)) {                   
        Front(s->queue)->state = lwt_READY;     // dequeue front and set to READY, if not empty
        Dequeue(s->queue);
    }
    ualarm(left, 0);
}
