#include "ScanState.hpp"

namespace ScanState {
    std::atomic<bool> g_scanActive{ false };
    std::atomic<bool> g_scanDied{ false };
}
