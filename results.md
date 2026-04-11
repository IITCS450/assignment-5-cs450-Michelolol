 CS450 Assignment 5: User-Level Threads + Mutex Results

 Context-Switching Approach
The context-switching mechanism relies on a cooperative model where threads explicitly yield control of the CPU. 
1. Thread State: A Thread Control Block (TCB) manages each thread, storing its state (RUNNABLE, RUNNING, ZOMBIE), its allocated stack, and a pointer to a struct context.
2. Context Switching (uswtch.S): When thread_yield() is called, it selects the next RUNNABLE thread and invokes the assembly function uswtch. This low-level function saves the callee-saved registers (%ebp, %ebx, %esi, %edi) of the current thread onto its stack, saves the current stack pointer (%esp), loads the new thread's stack pointer, and restores its registers before returning.
3. Thread Creation: thread_create allocates a 4KB stack and manually sets up the struct context. It points the instruction pointer (eip) to a wrapper function (thread_stub). To prevent General Protection Faults (trap 13) upon the first return to the new thread, a dummy return address (0) is pushed to the top of the stack before the context is loaded.
4. Cooperative Mutex: The mutex uses a simple while(m->locked) { thread_yield(); } spin-yield loop. Because the scheduler is cooperative, this safely yields the CPU to the lock holder without needing atomic hardware instructions.

 Limitations
 Maximum Threads: The system supports a hard limit of 16 concurrent threads (MAX_THREADS = 16).
 Stack Size: Each thread is limited to a fixed 4096-byte (1 page) stack (STACK_SIZE = 4096). Exceeding this will cause a stack overflow into neighboring heap memory.
 Scheduling: Because it relies entirely on cooperative scheduling (no timer interrupts), a thread that enters an infinite loop without calling thread_yield() will permanently starve all other threads.
 Join Semantics: thread_join uses an active cooperative polling loop (while(target->state != ZOMBIE) thread_yield();) rather than a blocked sleep queue.