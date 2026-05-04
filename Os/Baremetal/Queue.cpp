// Baremetal message queue for F Prime on RP2040.
// Backed by a statically allocated ring buffer per queue instance.
// All storage is allocated from a global pool at init time — no heap after init.

#include <Os/Queue.hpp>
#include <Fw/Types/Assert.hpp>
#include "hardware/sync.h"

namespace Os {

static const uint32_t MAX_QUEUES       = 8U;
static const uint32_t QUEUE_POOL_BYTES = 8192U;  // total storage for all queues

static uint8_t  s_pool[QUEUE_POOL_BYTES];
static uint32_t s_pool_offset = 0U;

struct QueueImpl {
    uint8_t  *buf;
    uint32_t  capacity;    // total pool bytes allocated for this queue
    uint32_t  msg_size;    // maximum bytes per message
    uint32_t  depth;       // max messages
    uint32_t  head;        // write index
    uint32_t  tail;        // read index
    uint32_t  count;       // current number of messages
};

static QueueImpl s_queues[MAX_QUEUES];
static uint32_t  s_queue_count = 0U;

Queue::Queue() : m_handle(nullptr) {}

Queue::~Queue() {}

Queue::QueueStatus Queue::create(const Fw::StringBase &name,
                                  NATIVE_INT_TYPE       depth,
                                  NATIVE_INT_TYPE       msgSize) {
    FW_ASSERT(depth > 0, static_cast<FwAssertArgType>(depth));
    FW_ASSERT(msgSize > 0, static_cast<FwAssertArgType>(msgSize));

    if (s_queue_count >= MAX_QUEUES) {
        return QUEUE_UNINITIALIZED;
    }

    uint32_t needed = static_cast<uint32_t>(depth) *
                      static_cast<uint32_t>(msgSize + static_cast<NATIVE_INT_TYPE>(sizeof(uint32_t)));
    if (s_pool_offset + needed > QUEUE_POOL_BYTES) {
        return QUEUE_UNINITIALIZED;
    }

    QueueImpl *q = &s_queues[s_queue_count];
    q->buf       = &s_pool[s_pool_offset];
    q->capacity  = needed;
    q->msg_size  = static_cast<uint32_t>(msgSize);
    q->depth     = static_cast<uint32_t>(depth);
    q->head      = 0U;
    q->tail      = 0U;
    q->count     = 0U;

    s_pool_offset += needed;
    this->m_handle = reinterpret_cast<void *>(
        static_cast<uintptr_t>(s_queue_count));
    s_queue_count++;
    return QUEUE_OK;
}

Queue::QueueStatus Queue::send(const Fw::SerializeBufferBase &buffer,
                                NATIVE_INT_TYPE               priority,
                                QueueBlocking                 block) {
    FW_ASSERT(this->m_handle != nullptr);
    uint32_t idx = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(this->m_handle));
    QueueImpl *q = &s_queues[idx];

    uint32_t irq = save_and_disable_interrupts();

    if (q->count >= q->depth) {
        restore_interrupts(irq);
        return QUEUE_FULL;
    }

    uint32_t len = static_cast<uint32_t>(buffer.getBuffLength());
    if (len > q->msg_size) {
        restore_interrupts(irq);
        return QUEUE_SIZE_MISMATCH;
    }

    // Store [4-byte length][payload]
    uint32_t slot = (q->head % q->depth) * (q->msg_size + sizeof(uint32_t));
    uint8_t *dst  = &q->buf[slot];

    dst[0] = static_cast<uint8_t>((len >> 24U) & 0xFFU);
    dst[1] = static_cast<uint8_t>((len >> 16U) & 0xFFU);
    dst[2] = static_cast<uint8_t>((len >>  8U) & 0xFFU);
    dst[3] = static_cast<uint8_t>( len         & 0xFFU);
    for (uint32_t i = 0U; i < len; i++) {
        dst[4U + i] = buffer.getBuffAddr()[i];
    }

    q->head++;
    q->count++;
    restore_interrupts(irq);
    return QUEUE_OK;
}

Queue::QueueStatus Queue::receive(Fw::SerializeBufferBase &buffer,
                                   NATIVE_INT_TYPE         &priority,
                                   QueueBlocking            block) {
    FW_ASSERT(this->m_handle != nullptr);
    uint32_t idx = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(this->m_handle));
    QueueImpl *q = &s_queues[idx];

    uint32_t irq = save_and_disable_interrupts();

    if (q->count == 0U) {
        restore_interrupts(irq);
        return QUEUE_NO_MORE_MSGS;
    }

    uint32_t slot = (q->tail % q->depth) * (q->msg_size + sizeof(uint32_t));
    uint8_t *src  = &q->buf[slot];

    uint32_t len = (static_cast<uint32_t>(src[0]) << 24U) |
                   (static_cast<uint32_t>(src[1]) << 16U) |
                   (static_cast<uint32_t>(src[2]) <<  8U) |
                    static_cast<uint32_t>(src[3]);

    Fw::SerializeStatus ss = buffer.setBuff(&src[4], len);
    q->tail++;
    q->count--;
    restore_interrupts(irq);

    priority = 0;
    return (ss == Fw::FW_SERIALIZE_OK) ? QUEUE_OK : QUEUE_SIZE_MISMATCH;
}

NATIVE_INT_TYPE Queue::getNumMsgs(void) const {
    FW_ASSERT(this->m_handle != nullptr);
    uint32_t idx = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(this->m_handle));
    return static_cast<NATIVE_INT_TYPE>(s_queues[idx].count);
}

NATIVE_INT_TYPE Queue::getMaxMsgs(void) const {
    return getNumMsgs();
}

NATIVE_INT_TYPE Queue::getQueueSize(void) const {
    FW_ASSERT(this->m_handle != nullptr);
    uint32_t idx = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(this->m_handle));
    return static_cast<NATIVE_INT_TYPE>(s_queues[idx].depth);
}

NATIVE_INT_TYPE Queue::getMsgSize(void) const {
    FW_ASSERT(this->m_handle != nullptr);
    uint32_t idx = static_cast<uint32_t>(
        reinterpret_cast<uintptr_t>(this->m_handle));
    return static_cast<NATIVE_INT_TYPE>(s_queues[idx].msg_size);
}

} // namespace Os
