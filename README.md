##LWT##
Light weight user-level(preemptive) thread.
###Test Environment###
Mac OSX 10.8.5
gcc version 4.2.1 (Based on Apple Inc. build 5658)

###Compile###
* For x64
	* gcc source.c lwt.c
* For x86
	* gcc source.c lwt.c -D _X86

###Test###
* For Producter-Consumer test
	* gcc test_exit.c lwt.c
* For lwt_exit and lwt_sleep
	* gcc lwt.c test_exit.c

###Update by Neil 11/01/13

* Merge lwt_run() to lwt_create(), for immediate execute
* Merge lwt_RUNNING to LWT_READY
* Import a Queue lib to store semaphore waiting list
* Change prior mechanism
* Change thrd_wait / check_ready implementation
* Add Makefile to compile Producer Consumer test
* Clean code
