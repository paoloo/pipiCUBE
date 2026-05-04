// Baremetal mutex for F Prime on RP2040.
// Implemented as a global interrupt disable/enable pair.
// Suitable only for single-core use (core 0). Core 1 is reserved.

#include <Os/Mutex.hpp>
#include <Fw/Types/Assert.hpp>
#include "hardware/sync.h"

namespace Os {

// Saved interrupt state pushed by lock(), restored by unLock()
static uint32_t s_saved_irq = 0U;
static uint32_t s_lock_depth = 0U;

Mutex::Mutex() : m_handle(nullptr) {}

Mutex::~Mutex() {}

void Mutex::lock(void) {
    if (s_lock_depth == 0U) {
        s_saved_irq = save_and_disable_interrupts();
    }
    s_lock_depth++;
    FW_ASSERT(s_lock_depth < 32U, static_cast<FwAssertArgType>(s_lock_depth));
}

void Mutex::unLock(void) {
    FW_ASSERT(s_lock_depth > 0U, static_cast<FwAssertArgType>(s_lock_depth));
    s_lock_depth--;
    if (s_lock_depth == 0U) {
        restore_interrupts(s_saved_irq);
    }
}

} // namespace Os
