// Bare-metal GCC atomic runtime for Cortex-M0+.
// Cortex-M0+ has no LL/SC or LDREX/STREX instructions, so GCC calls these
// __atomic_* helpers for any std::atomic<T> operation.  We implement them
// with PRIMASK-based interrupt masking, which is correct for single-core
// bare-metal code (RP2040 uses only core 0 in this firmware).
#include <stdbool.h>
#include <stdint.h>

static inline uint32_t primask_save_disable(void) {
    uint32_t pm;
    __asm volatile ("mrs %0, PRIMASK\n\t"
                    "cpsid i"
                    : "=r"(pm) :: "memory");
    return pm;
}
static inline void primask_restore(uint32_t pm) {
    __asm volatile ("msr PRIMASK, %0" :: "r"(pm) : "memory");
}

// ── 1-byte (bool / uint8_t) ───────────────────────────────────────────────
uint8_t __atomic_load_1(const volatile uint8_t *p, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t v = *p;
    primask_restore(s);
    return v;
}
void __atomic_store_1(volatile uint8_t *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    *p = v;
    primask_restore(s);
}
uint8_t __atomic_exchange_1(volatile uint8_t *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t old = *p; *p = v;
    primask_restore(s);
    return old;
}
bool __atomic_compare_exchange_1(volatile uint8_t *p, uint8_t *exp,
                                  uint8_t des, bool weak, int sm, int fm) {
    (void)weak; (void)sm; (void)fm;
    uint32_t s = primask_save_disable();
    bool ok = (*p == *exp);
    if (ok) *p = des; else *exp = *p;
    primask_restore(s);
    return ok;
}
uint8_t __atomic_fetch_add_1(volatile uint8_t *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t old = *p; *p = (uint8_t)(*p + v);
    primask_restore(s);
    return old;
}
uint8_t __atomic_fetch_sub_1(volatile uint8_t *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t old = *p; *p = (uint8_t)(*p - v);
    primask_restore(s);
    return old;
}
uint8_t __atomic_fetch_and_1(volatile uint8_t *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t old = *p; *p = (uint8_t)(*p & v);
    primask_restore(s);
    return old;
}
uint8_t __atomic_fetch_or_1(volatile uint8_t *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t old = *p; *p = (uint8_t)(*p | v);
    primask_restore(s);
    return old;
}

// ── 2-byte (uint16_t) ────────────────────────────────────────────────────
uint16_t __atomic_load_2(const volatile uint16_t *p, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t v = *p;
    primask_restore(s);
    return v;
}
void __atomic_store_2(volatile uint16_t *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    *p = v;
    primask_restore(s);
}
uint16_t __atomic_exchange_2(volatile uint16_t *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t old = *p; *p = v;
    primask_restore(s);
    return old;
}
bool __atomic_compare_exchange_2(volatile uint16_t *p, uint16_t *exp,
                                   uint16_t des, bool weak, int sm, int fm) {
    (void)weak; (void)sm; (void)fm;
    uint32_t s = primask_save_disable();
    bool ok = (*p == *exp);
    if (ok) *p = des; else *exp = *p;
    primask_restore(s);
    return ok;
}
uint16_t __atomic_fetch_add_2(volatile uint16_t *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t old = *p; *p = (uint16_t)(*p + v);
    primask_restore(s);
    return old;
}
uint16_t __atomic_fetch_sub_2(volatile uint16_t *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t old = *p; *p = (uint16_t)(*p - v);
    primask_restore(s);
    return old;
}
uint16_t __atomic_fetch_and_2(volatile uint16_t *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t old = *p; *p = (uint16_t)(*p & v);
    primask_restore(s);
    return old;
}
uint16_t __atomic_fetch_or_2(volatile uint16_t *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t old = *p; *p = (uint16_t)(*p | v);
    primask_restore(s);
    return old;
}

// ── 4-byte (uint32_t) ────────────────────────────────────────────────────
uint32_t __atomic_load_4(const volatile uint32_t *p, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t v = *p;
    primask_restore(s);
    return v;
}
void __atomic_store_4(volatile uint32_t *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    *p = v;
    primask_restore(s);
}
uint32_t __atomic_exchange_4(volatile uint32_t *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t old = *p; *p = v;
    primask_restore(s);
    return old;
}
bool __atomic_compare_exchange_4(volatile uint32_t *p, uint32_t *exp,
                                   uint32_t des, bool weak, int sm, int fm) {
    (void)weak; (void)sm; (void)fm;
    uint32_t s = primask_save_disable();
    bool ok = (*p == *exp);
    if (ok) *p = des; else *exp = *p;
    primask_restore(s);
    return ok;
}
uint32_t __atomic_fetch_add_4(volatile uint32_t *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t old = *p; *p = *p + v;
    primask_restore(s);
    return old;
}
uint32_t __atomic_fetch_sub_4(volatile uint32_t *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t old = *p; *p = *p - v;
    primask_restore(s);
    return old;
}
uint32_t __atomic_fetch_and_4(volatile uint32_t *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t old = *p; *p = *p & v;
    primask_restore(s);
    return old;
}
uint32_t __atomic_fetch_or_4(volatile uint32_t *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t old = *p; *p = *p | v;
    primask_restore(s);
    return old;
}
