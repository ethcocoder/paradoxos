/* ParadoxOS Global Definitions */
#ifndef _PARADOX_H
#define _PARADOX_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* Helper Macros */
#define UNUSED(x) (void)(x)
#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

/* Standard Types Re-exports (for convenience) */
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

#endif // _PARADOX_H
