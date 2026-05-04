#ifndef PIPICUBE_FP_CONFIG_H
#define PIPICUBE_FP_CONFIG_H

// ── Integer base types ────────────────────────────────────────────────────────
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uint8_t   U8;
typedef uint16_t  U16;
typedef uint32_t  U32;
typedef uint64_t  U64;
typedef int8_t    I8;
typedef int16_t   I16;
typedef int32_t   I32;
typedef int64_t   I64;
typedef float     F32;
typedef double    F64;
typedef int32_t   NATIVE_INT_TYPE;
typedef uint32_t  NATIVE_UINT_TYPE;
typedef uint32_t  FwOpcodeType;
typedef uint32_t  FwChanIdType;
typedef uint32_t  FwEventIdType;
typedef uint32_t  FwPrmIdType;

// ── Object naming (disabled to save flash on embedded target) ─────────────────
#define FW_OBJECT_NAMES 0
#define FW_OBJECT_REGISTRATION 0

// ── Port/tracing (disabled on embedded) ──────────────────────────────────────
#define FW_PORT_TRACING 0
#define FW_PORT_SERIALIZATION 1

// ── String buffer size (80 chars covers NMEA fields + LoRa AT responses) ─────
#define FW_MAX_STRING_BUFFER_SIZE 80U

// ── Serialisation type tag (disabled to save bytes) ──────────────────────────
#define FW_SERIALIZATION_TYPE_ID 0

// ── Assert behaviour: spin on failure, letting watchdog recover ───────────────
#define FW_ASSERT_LEVEL FW_FILELINE_ASSERT

// ── Command / telemetry buffer ────────────────────────────────────────────────
#define FW_COM_BUFFER_MAX_SIZE 128U

// ── Log/event buffers ─────────────────────────────────────────────────────────
#define FW_LOG_TEXT_BUFFER_SIZE 80U

// ── Max number of OS tasks ────────────────────────────────────────────────────
#define OS_MAX_NUM_TASKS 8U

// ── Queue element size cap ────────────────────────────────────────────────────
#define FW_QUEUE_SIZING FwQueueSizeType

#endif // PIPICUBE_FP_CONFIG_H
