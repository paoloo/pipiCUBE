#ifndef PIPICUBE_FP_CONFIG_H
#define PIPICUBE_FP_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

// BasicTypes.h provides U8/U16/U32/U64/I8/../F32/F64/NATIVE_INT_TYPE etc.
// PlatformTypes.h provides PlatformPointerCastType for this platform.
// Both are included by Fw/FPrimeBasicTypes.h before this file, but guard
// against direct includes of FpConfig.h.
#include <Fw/Types/BasicTypes.h>
#include <Platform/PlatformTypes.h>

// ── Flash-saving options for bare-metal embedded target ───────────────────────

// Disable per-object name storage and registration to save RAM/flash
#define FW_OBJECT_NAMES         (0)
#define FW_OBJECT_REGISTRATION  (0)
#define FW_QUEUE_REGISTRATION   (0)

// Required by autocoded topology Ac.cpp — expands to "" when names are off
#if FW_OBJECT_NAMES == 1
#define FW_OPTIONAL_NAME(name) name
#else
#define FW_OPTIONAL_NAME(name) ""
#endif

// Disable port call tracing
#define FW_PORT_TRACING         (0)

// Keep port serialization (required for cross-component communication)
#define FW_PORT_SERIALIZATION   (1)

// Use file-CRC + line-number in asserts (smaller than full filename strings)
#define FW_ASSERT_LEVEL         FW_FILEID_ASSERT

// Required: we use Fw_StringFormat_snprintf, so printf family must be enabled
#define FW_USE_PRINTF_FAMILY_FUNCTIONS_IN_STRING_FORMATTING (1)

// Disable text logging output port to save code/flash
// Requires FPRIME_ENABLE_TEXT_LOGGERS=OFF in cmake
#define FW_ENABLE_TEXT_LOGGING  (0)

// Disable toString() on serializables (implied by text logging off)
#define FW_SERIALIZABLE_TO_STRING (0)

#ifdef __cplusplus
}
#endif

#endif // PIPICUBE_FP_CONFIG_H
