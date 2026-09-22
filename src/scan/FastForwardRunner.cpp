#include "FastForwardRunner.hpp"
#include "ScanState.hpp"

using namespace geode::prelude;

namespace FastForwardRunner {

    // 驱动一个完整的模拟 tick。
    //
    // 早期版本这里直接调用 pl->update(dt)，但这只会执行
    // GJBaseGameLayer 自身注册的那部分逐帧逻辑 —— cocos2d 的调度器
    // (CCScheduler) 才是真正的顶层驱动入口，一次真实渲染帧靠它才会
    // 级联触发场景中所有注册了逐帧回调的节点（GJBaseGameLayer 只是
    // 其中之一，玩家对象、特效等可能各自还注册了独立的回调）。
    // 直接调用 pl->update() 会跳过这些其他回调，导致死亡判定等
    // 依赖它们的逻辑无法可靠触发 —— 这正是扫描结果异常宽松、
    // 上边界几乎测不到失败的根本原因。参考 GDMegaOverlay 的
    // Speedhack.cpp：它们钩的是 CCScheduler::update，而不是
    // GJBaseGameLayer::update，这里改为同样驱动顶层调度器。
    static void stepOneTick(float dt) {
        CCDirector::sharedDirector()->getScheduler()->update(dt);
    }

    Result advanceTicks(PlayLayer* pl, int ticks, float dt) {
        if (!pl || ticks <= 0) return Result::Survived;

        ScanState::g_scanDied.store(false);
        ScanState::g_scanActive.store(true);

        Result result = Result::Survived;

        for (int i = 0; i < ticks; i++) {
            stepOneTick(dt);

            if (ScanState::g_scanDied.load()) {
                result = Result::Died;
                break;
            }
        }

        ScanState::g_scanActive.store(false);
        return result;
    }

    Result advanceTicksWithReplay(
        PlayLayer* pl,
        int ticks,
        int startFrame,
        const std::vector<InputReplayer::ReplayEvent>& events,
        float dt
    ) {
        if (!pl || ticks <= 0) return Result::Survived;

        ScanState::g_scanDied.store(false);
        ScanState::g_scanActive.store(true);

        Result result = Result::Survived;

        for (int i = 0; i < ticks; i++) {
            // 先重放本 tick 应该发生的按键，再推进物理，
            // 这样按键会在正确的那个 tick 之内生效。
            InputReplayer::driveTick(pl, events, startFrame, i);
            stepOneTick(dt);

            if (ScanState::g_scanDied.load()) {
                result = Result::Died;
                break;
            }
        }

        ScanState::g_scanActive.store(false);
        return result;
    }

}
