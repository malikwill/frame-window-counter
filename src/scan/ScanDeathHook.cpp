#include <Geode/Geode.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include "ScanState.hpp"

using namespace geode::prelude;

// 仅在扫描进行期间（g_scanActive）监听死亡/重置信号。
// 正常游玩时完全不受影响 —— resetLevel() 照常执行，我们只是额外做一次标记。
// 参考 GDMegaOverlay 的 Macrobot 实现：resetLevel() 是死亡触发重置的实际入口。
class $modify(ScanDeathListener, PlayLayer) {
    void resetLevel() {
        if (ScanState::g_scanActive.load()) {
            ScanState::g_scanDied.store(true);
        }
        PlayLayer::resetLevel();
    }
};
