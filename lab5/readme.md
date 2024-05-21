# Lab5 Thread and User Process

## Goal
- program instance, process/threads creation
- scheduling, context switch, multitasking
- system calls

## Requirement
```c
int thread_create((void*)f_ptr);
// in this function, create thread data_structure, TCB, thread stack, run queue

int schedule()
// call by current thread, it pick the next thread from run queue to run
// context switch save thread register, round robin

int idle_thread()
// a task always runable
// while true, call kill_zombies and schedule
```

## Appendix
### Linux List
Credit: [Linux Linked List](https://hackmd.io/@RinHizakura/HkEuhNwGO)
