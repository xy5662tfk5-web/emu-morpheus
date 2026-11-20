#include "morpheus/MorpheusCore.h"

namespace morpheus {

void MorpheusCore::reset() noexcept {
    // Hardware reset sequencing is intentionally unimplemented until
    // authoritative Morpheus documentation and behavior are established.
    initialized_ = false;
}

bool MorpheusCore::isInitialized() const noexcept {
    return initialized_;
}

} // namespace morpheus
