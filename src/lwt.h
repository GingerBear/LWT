#ifndef _LWT_H  
#define _LWT_H  

#define lwt_STACK_SIZE (16*1024*8)

typedef void (*lwt_fn)(void *);	// Declare a type to store function
enum lwt_state_t;
typedef enum lwt_state_t lwt_state;
struct lwt_t;
typedef struct lwt_t lwt;
struct sys_lwt_t;
typedef struct sys_lwt_t sys_lwt;
struct smphr_t;
typedef struct smphr_t smphr;

void lwt_init (int lwt_quantum);
lwt * lwt_create (char *name, int argc, void *argv, lwt_fn fn);
void alarm_handler ();
void _check_ready (lwt *t);
void _check_awake (lwt *t);
void lwt_sleep (int s);
int thrd_wait (lwt *t);
void lwt_exit (int status);
void lwt_del(lwt *t);
char * get_curr_name();
smphr * smphr_create(int smphr);
void P(smphr *s);
void V(smphr *s);
int smphr_get_size(smphr *s);

#endif