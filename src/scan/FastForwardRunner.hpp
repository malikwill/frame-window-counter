#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include "InputReplayer.hpp"

// 在不等待真实渲染帧的情况下，直接驱动物理模拟前进指定的 tick 数。
// 依据：GDMegaOverlay 的 frame-step 功能证实 GJBaseGameLayer::update(dt)
// 每调用一次即推进恰好一个物理 tick，因此我们可以自行连续调用它来实现
// 瞬间快进，而不依赖真实经过的时间。
namespace FastForwardRunner {

    // 结果：Survived 表示这些 tick 内玩家未死亡；Died 表示在推进过程中
    // 触发了 PlayLayer::resetLevel()（即死亡/重置）。
    enum class Result {
        Survived,
        Died
    };

    // 推进恰好 `ticks` 个物理 tick，不注入任何按键输入。
    // dt 应为 1.0f / TPS（通常 TPS = 240，即 dt ≈ 1/240）。
    // 若中途检测到死亡会立即停止，不会继续推进剩余 tick。
    Result advanceTicks(PlayLayer* pl, int ticks, float dt = 1.0f / 240.0f);

    // 与上面相同，但每个 tick 会先通过 InputReplayer 重放 events 中
    // 落在该 tick 的按键，再推进物理。startFrame 是快照时刻关卡的绝对帧号
    // （对应 tick 0），用于把 events 里的绝对帧号对齐到本次推进的相对 tick。
    Result advanceTicksWithReplay(
        PlayLayer* pl,
        int ticks,
        int startFrame,
        const std::vector<InputReplayer::ReplayEvent>& events,
        float dt = 1.0f / 240.0f
    );

}

