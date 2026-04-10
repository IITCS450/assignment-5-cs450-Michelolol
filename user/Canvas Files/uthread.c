#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

// TODO: Implement cooperative user-level threads.

enum tstate { FREE, RUNNABLE, RUNNING, ZOMBIE };

struct context {
    uint edi;
    uint esi;
    uint ebx;
    uint ebp;
    uint eip;
};

struct thread {
    tid_t tid;
    enum tstate state;
    char *stack;
    struct context *context;
  
    // Storing the function and argument for the stub
    void (*func)(void*);
    void *arg;
};

struct thread threads[MAX_THREADS];
struct thread *current_thread;
tid_t next_tid = 1;

// Wrapper function to execute the thread and clean up afterward
void thread_stub(void) {
    // Execute the user's function
    current_thread->func(current_thread->arg);
  
    // Mark as ZOMBIE and yield to the scheduler
    current_thread->state = ZOMBIE;
    thread_yield();
}


void thread_init(void){// Initialize all thread slots to FREE
    for(int i = 0; i < MAX_THREADS; i++) {
        threads[i].state = FREE;
  }}


tid_t thread_create(void (*fn)(void*), void *arg){ 
    struct thread *t = 0;
  
    // Find a FREE thread slot
    for(int i = 0; i < MAX_THREADS; i++) {
        if(threads[i].state == FREE) {
        t = &threads[i];
        break;
        }
    }
    if(t == 0) return -1; // No free slots available
  
    // Allocate stack memory
    t->stack = malloc(STACK_SIZE);
    if(t->stack == 0) return -1;
  
    t->tid = next_tid++;
    t->func = fn;
    t->arg = arg;
  
    // Set up the stack pointer and context
    char *sp = t->stack + STACK_SIZE;
  
    // Allocate space for struct context on the thread's stack
    sp -= sizeof(struct context);
    t->context = (struct context*)sp;
  
    // Zero out the context 
    memset(t->context, 0, sizeof(struct context));
  
    // Set the instruction pointer to wrapper
    t->context->eip = (uint)thread_stub;
  
    t->state = RUNNABLE;
  
    return t->tid;
}


void thread_yield(void){
    struct thread *old_thread = current_thread;
    struct thread *next_thread = 0;
  
    // Round-robin scheduler: Find the next RUNNABLE thread
    int current_idx = old_thread - threads;
    for(int i = 1; i <= MAX_THREADS; i++) {
        int idx = (current_idx + i) % MAX_THREADS;
        if(threads[idx].state == RUNNABLE) {
            next_thread = &threads[idx];
            break;
        }
    }
  
    // If no other threads are RUNNABLE, just keep running the current one
    if(next_thread == 0) {
        return;
    }
  
    // Update states
    if(old_thread->state == RUNNING) {
        old_thread->state = RUNNABLE;
    }
    next_thread->state = RUNNING;
    current_thread = next_thread;
  
    // Performs the low-level context switch
    uswtch(&old_thread->context, next_thread->context);
}

int thread_join(tid_t tid){
    struct thread *target = 0;
  
    // Find the thread with the matching tid
    for(int i = 0; i < MAX_THREADS; i++) {
        if(threads[i].tid == tid && threads[i].state != FREE) {
            target = &threads[i];
            break;
        }
    }
  
    if(target == 0) return -1; // Thread not found
  
    // Wait until the thread finishes execution
    while(target->state != ZOMBIE) {
        thread_yield();
    }
  
    // Clean up resources to prevent memory leaks
    free(target->stack);
    target->state = FREE;
    target->tid = 0;
  
    return 0;
}