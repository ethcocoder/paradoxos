#ifndef _SCHED_SCHEDULER_H
#define _SCHED_SCHEDULER_H

#include <stdint.h>

void scheduler_init(void);
void task_create(void (*entry)(void));
void schedule(void);
void task_switch(uint64_t **old_sp, uint64_t *new_sp);

#endif