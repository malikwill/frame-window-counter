#pragma once
#include <atomic>

// 扫描过程中的共享状态标志。
// g_scanActive 由扫描循环在推进物理帧时设为 true；
// 期间若 PlayLayer::resetLevel() 被触发（即玩家死亡/重置），
// 钩子会将 g_scanDied 设为 true 作为失败信号。
namespace ScanState {
    extern std::atomic<bool> g_scanActive;
    extern std::atomic<bool> g_scanDied;
}
