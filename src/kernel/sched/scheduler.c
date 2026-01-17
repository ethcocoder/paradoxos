#include <paradox.h>
#include <lib/string.h>
#include <mm/pmm.h>
#include <drivers/serial.h>

#define STACK_SIZE 4096
#define MAX_TASKS 10

typedef struct {
    uint64_t *stack_ptr; // Saved RSP
    uint64_t pid;
    uint64_t state; // 0=Ready, 1=Running
} task_t;

task_t tasks[MAX_TASKS];
int current_task = 0;
int task_count = 0;

// Assembly helper to switch stacks
// void task_switch(uint64_t **old_sp, uint64_t *new_sp);
extern void task_switch(uint64_t **old_sp, uint64_t *new_sp);

void scheduler_init(void) {
    // Task 0 is the running kernel (main thread)
    tasks[0].pid = 0;
    tasks[0].state = 1;
    tasks[0].stack_ptr = 0; // Will be updated on switch
    task_count = 1;
    current_task = 0;
    serial_puts("[Scheduler] Initialized. Kernel is PID 0.\n");
}

void task_create(void (*entry)(void)) {
    if (task_count >= MAX_TASKS) return;

    task_t *t = &tasks[task_count];
    t->pid = task_count;
    t->state = 0;

    // Allocate stack
    uint64_t *stack_bottom = (uint64_t *)pmm_alloc(1); // 1 page
    if (!stack_bottom) return;
    
    uint64_t *stack_top = (uint64_t *)((uint8_t *)stack_bottom + PAGE_SIZE);

    // Setup initial stack frame (mimic what 'task_switch' expects)
    // We expect: R15, R14, R13, R12, R11, R10, R9, R8, RBP, RDI, RSI, RDX, RCX, RBX, RAX, RIP
    
    // NOTE: This depends on the specific assembly implementation of task_switch.
    // Let's assume callee-save registers: RBX, RBP, R12-R15 + RIP.
    // The C-calling convention (System V AMD64) says RBX, RBP, R12-R15 are preserved.
    
    *(--stack_top) = (uint64_t)entry; // RIP (Return Address)
    
    *(--stack_top) = 0; // R15
    *(--stack_top) = 0; // R14
    *(--stack_top) = 0; // R13
    *(--stack_top) = 0; // R12
    *(--stack_top) = 0; // RBP
    *(--stack_top) = 0; // RBX
    
    t->stack_ptr = stack_top;
    task_count++;
    serial_puts("[Scheduler] Created new task.\n");
}

void schedule(void) {
    if (task_count <= 1) return; // Nothing to switch to

    int next = (current_task + 1) % task_count;
    // Simple round robin, assume all ready

    if (next == current_task) return;

    task_t *curr = &tasks[current_task];
    task_t *nxt  = &tasks[next];

    current_task = next;
    
    // Switch!
    task_switch(&curr->stack_ptr, nxt->stack_ptr);
}
