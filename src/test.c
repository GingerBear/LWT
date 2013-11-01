#include "lwt.h"
#include <stdio.h>

#define QUANTUM 100

void foo() {
    printf("Foo will sleep 2 senconds... \n");
    lwt_sleep(2);
    printf("Foo wake up \n");
    lwt_exit(3);
    printf("Problem my occur here\n");
}
void bar() {
    printf("Bar will sleep 3 seconds... \n");
    lwt_sleep(5);
    //printf("current is ... %s\n", sys_t.curr->name);
    //printf("next is ... %s\n", sys_t.curr->next->name);
    //printf("next is ... %s\n", sys_t.lcurr->next->next->name);
    printf("Bar wake up \n");
    lwt_exit(4);
}

int main () {
    lwt_init(QUANTUM);
    lwt *t1 = lwt_create("foo", 0, NULL, foo);
    //lwt_run(t1);
    lwt *t2 = lwt_create("bar", 0, NULL, bar);
    //lwt_run(t2);
    int s1 = thrd_wait(t1);
    int s2 = thrd_wait(t2);
    lwt_del(t1);
    lwt_del(t2);
    printf("Achieve with status: %d\n", s1);
    printf("Achieve with status: %d\n", s2);
    lwt_exit(5);
}