// Bare-metal GCC atomic runtime for Cortex-M0+.
// Cortex-M0+ has no LL/SC or LDREX/STREX instructions, so GCC calls these
// __atomic_* helpers for std::atomic<T> operations.  We implement them with
// PRIMASK-based interrupt masking (single-core bare-metal, core 0 only).
//
// Signatures match the GCC libatomic ABI exactly: pointer arguments are
// volatile void * (not typed) to avoid -Wbuiltin-declaration-mismatch.
#include <stdint.h>

// _Bool without stdbool.h to avoid any header pollution
typedef _Bool atomic_bool_t;

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

// ── 1-byte ───────────────────────────────────────────────────────────────
uint8_t __atomic_load_1(const volatile void *p, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t v = *(const volatile uint8_t *)p;
    primask_restore(s);
    return v;
}
void __atomic_store_1(volatile void *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    *(volatile uint8_t *)p = v;
    primask_restore(s);
}
uint8_t __atomic_exchange_1(volatile void *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t old = *(volatile uint8_t *)p;
    *(volatile uint8_t *)p = v;
    primask_restore(s);
    return old;
}
atomic_bool_t __atomic_compare_exchange_1(volatile void *p, void *exp,
                                          uint8_t des, atomic_bool_t weak,
                                          int sm, int fm) {
    (void)weak; (void)sm; (void)fm;
    uint32_t s = primask_save_disable();
    uint8_t cur = *(volatile uint8_t *)p;
    atomic_bool_t ok = (cur == *(uint8_t *)exp);
    if (ok) *(volatile uint8_t *)p = des;
    else    *(uint8_t *)exp = cur;
    primask_restore(s);
    return ok;
}
uint8_t __atomic_fetch_add_1(volatile void *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t old = *(volatile uint8_t *)p;
    *(volatile uint8_t *)p = (uint8_t)(old + v);
    primask_restore(s);
    return old;
}
uint8_t __atomic_fetch_sub_1(volatile void *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t old = *(volatile uint8_t *)p;
    *(volatile uint8_t *)p = (uint8_t)(old - v);
    primask_restore(s);
    return old;
}
uint8_t __atomic_fetch_and_1(volatile void *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t old = *(volatile uint8_t *)p;
    *(volatile uint8_t *)p = (uint8_t)(old & v);
    primask_restore(s);
    return old;
}
uint8_t __atomic_fetch_or_1(volatile void *p, uint8_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint8_t old = *(volatile uint8_t *)p;
    *(volatile uint8_t *)p = (uint8_t)(old | v);
    primask_restore(s);
    return old;
}

// ── 2-byte ───────────────────────────────────────────────────────────────
uint16_t __atomic_load_2(const volatile void *p, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t v = *(const volatile uint16_t *)p;
    primask_restore(s);
    return v;
}
void __atomic_store_2(volatile void *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    *(volatile uint16_t *)p = v;
    primask_restore(s);
}
uint16_t __atomic_exchange_2(volatile void *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t old = *(volatile uint16_t *)p;
    *(volatile uint16_t *)p = v;
    primask_restore(s);
    return old;
}
atomic_bool_t __atomic_compare_exchange_2(volatile void *p, void *exp,
                                          uint16_t des, atomic_bool_t weak,
                                          int sm, int fm) {
    (void)weak; (void)sm; (void)fm;
    uint32_t s = primask_save_disable();
    uint16_t cur = *(volatile uint16_t *)p;
    atomic_bool_t ok = (cur == *(uint16_t *)exp);
    if (ok) *(volatile uint16_t *)p = des;
    else    *(uint16_t *)exp = cur;
    primask_restore(s);
    return ok;
}
uint16_t __atomic_fetch_add_2(volatile void *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t old = *(volatile uint16_t *)p;
    *(volatile uint16_t *)p = (uint16_t)(old + v);
    primask_restore(s);
    return old;
}
uint16_t __atomic_fetch_sub_2(volatile void *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t old = *(volatile uint16_t *)p;
    *(volatile uint16_t *)p = (uint16_t)(old - v);
    primask_restore(s);
    return old;
}
uint16_t __atomic_fetch_and_2(volatile void *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t old = *(volatile uint16_t *)p;
    *(volatile uint16_t *)p = (uint16_t)(old & v);
    primask_restore(s);
    return old;
}
uint16_t __atomic_fetch_or_2(volatile void *p, uint16_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint16_t old = *(volatile uint16_t *)p;
    *(volatile uint16_t *)p = (uint16_t)(old | v);
    primask_restore(s);
    return old;
}

// ── 4-byte ───────────────────────────────────────────────────────────────
uint32_t __atomic_load_4(const volatile void *p, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t v = *(const volatile uint32_t *)p;
    primask_restore(s);
    return v;
}
void __atomic_store_4(volatile void *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    *(volatile uint32_t *)p = v;
    primask_restore(s);
}
uint32_t __atomic_exchange_4(volatile void *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t old = *(volatile uint32_t *)p;
    *(volatile uint32_t *)p = v;
    primask_restore(s);
    return old;
}
atomic_bool_t __atomic_compare_exchange_4(volatile void *p, void *exp,
                                          uint32_t des, atomic_bool_t weak,
                                          int sm, int fm) {
    (void)weak; (void)sm; (void)fm;
    uint32_t s = primask_save_disable();
    uint32_t cur = *(volatile uint32_t *)p;
    atomic_bool_t ok = (cur == *(uint32_t *)exp);
    if (ok) *(volatile uint32_t *)p = des;
    else    *(uint32_t *)exp = cur;
    primask_restore(s);
    return ok;
}
uint32_t __atomic_fetch_add_4(volatile void *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t old = *(volatile uint32_t *)p;
    *(volatile uint32_t *)p = old + v;
    primask_restore(s);
    return old;
}
uint32_t __atomic_fetch_sub_4(volatile void *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t old = *(volatile uint32_t *)p;
    *(volatile uint32_t *)p = old - v;
    primask_restore(s);
    return old;
}
uint32_t __atomic_fetch_and_4(volatile void *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t old = *(volatile uint32_t *)p;
    *(volatile uint32_t *)p = old & v;
    primask_restore(s);
    return old;
}
uint32_t __atomic_fetch_or_4(volatile void *p, uint32_t v, int m) {
    (void)m;
    uint32_t s = primask_save_disable();
    uint32_t old = *(volatile uint32_t *)p;
    *(volatile uint32_t *)p = old | v;
    primask_restore(s);
    return old;
}
