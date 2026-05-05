@ Unsigned type for memory/file sizes. RP2040 is 32-bit.
type PlatformSizeType = U32

@ Signed counterpart used for offsets (e.g. file seek).
type PlatformSignedSizeType = I32

@ Signed type for port/array indices.
type PlatformIndexType = I16

@ Type passed to assert macros.
type PlatformAssertArgType = I32

@ Type for task priorities.
type PlatformTaskPriorityType = U8

@ Type for task identifiers.
type PlatformTaskIdType = I32

@ Type for message-queue priorities.
type PlatformQueuePriorityType = U8
