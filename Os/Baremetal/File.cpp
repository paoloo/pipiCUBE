// Baremetal file stub for F Prime on RP2040.
// There is no filesystem in the MVP; all operations return an error.
// This satisfies the F' Os/File.hpp interface without pulling in flash/LFS.

#include <Os/File.hpp>

namespace Os {

File::File() : m_handle(nullptr), m_mode(OPEN_NO_MODE), m_lastError(OTHER_ERROR) {}
File::~File() {}

File::Status File::open(const char *fileName, File::Mode mode, bool include_excl) {
    (void)fileName; (void)mode; (void)include_excl;
    return OTHER_ERROR;
}

void File::close(void) {}

File::Status File::read(void *buffer, NATIVE_INT_TYPE &size, bool waitForFull) {
    (void)buffer; (void)waitForFull;
    size = 0;
    return OTHER_ERROR;
}

File::Status File::write(const void *buffer, NATIVE_INT_TYPE &size, bool waitForDone) {
    (void)buffer; (void)waitForDone;
    size = 0;
    return OTHER_ERROR;
}

File::Status File::seek(NATIVE_INT_TYPE offset, bool absolute) {
    (void)offset; (void)absolute;
    return OTHER_ERROR;
}

} // namespace Os
