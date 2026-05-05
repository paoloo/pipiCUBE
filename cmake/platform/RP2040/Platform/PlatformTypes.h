#ifndef PLATFORM_TYPES_H_
#define PLATFORM_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// RP2040 has 32-bit pointers
typedef uint32_t PlatformPointerCastType;
#define PRI_PlatformPointerCastType PRIx32

#ifdef __cplusplus
}
#endif

#endif // PLATFORM_TYPES_H_
