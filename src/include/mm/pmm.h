#ifndef _MM_PMM_H
#define _MM_PMM_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 4096

void pmm_init(void);
void *pmm_alloc(size_t count);
void pmm_free(void *ptr, size_t count);

#endif
