// Baremetal task abstraction for F Prime on RP2040.
// Uses cooperative scheduling: each "task" is a function pointer stored in a
// global table. The main loop calls them in round-robin order.
// There is no preemption; components must yield by returning from their handler.

#include <Os/Task.hpp>
#include <Fw/Types/Assert.hpp>
#include "pico/time.h"

namespace Os {

// Maximum number of registered tasks (matches OS_MAX_NUM_TASKS in FpConfig.h)
static const uint32_t MAX_TASKS = 8U;

struct TaskEntry {
    Task::taskRoutine routine;
    void             *arg;
    bool              active;
};

static TaskEntry s_tasks[MAX_TASKS];
static uint32_t  s_task_count = 0U;

Task::Task() : m_handle(nullptr), m_identifier(0), m_affinity(-1),
               m_started(false), m_suspendedOnPurpose(false) {}

Task::TaskStatus Task::start(const Fw::StringBase &name,
                              taskRoutine          routine,
                              void                *arg,
                              NATIVE_UINT_TYPE      priority,
                              NATIVE_UINT_TYPE      stackSize,
                              NATIVE_UINT_TYPE      cpuAffinity,
                              NATIVE_UINT_TYPE      identifier) {
    FW_ASSERT(routine != nullptr);

    if (s_task_count >= MAX_TASKS) {
        return TASK_UNKNOWN_ERROR;
    }

    s_tasks[s_task_count].routine = routine;
    s_tasks[s_task_count].arg     = arg;
    s_tasks[s_task_count].active  = true;
    this->m_handle = reinterpret_cast<void *>(
        static_cast<uintptr_t>(s_task_count));
    s_task_count++;
    this->m_started = true;
    return TASK_OK;
}

// Called from the main cooperative loop to run all registered tasks once.
void Task::dispatch_all(void) {
    for (uint32_t i = 0U; i < s_task_count; i++) {
        if (s_tasks[i].active) {
            s_tasks[i].routine(s_tasks[i].arg);
        }
    }
}

Task::TaskStatus Task::delay(NATIVE_UINT_TYPE milliseconds) {
    // Cap at 1000 ms per coding standard
    uint32_t ms = (milliseconds > 1000U) ? 1000U : static_cast<uint32_t>(milliseconds);
    sleep_ms(ms);
    return TASK_OK;
}

Task::~Task() {}

} // namespace Os
