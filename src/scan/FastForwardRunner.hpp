#pragma once
#include <Geode/Geode.hpp>

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

    // 推进恰好 `ticks` 个物理 tick。dt 应为 1.0f / TPS（通常 TPS = 240，
    // 即 dt ≈ 1/240）。若中途检测到死亡会立即停止，不会继续推进剩余 tick。
    //
    // 注意：这只推进物理模拟本身，不会注入任何按键输入 —— 这是有意为之，
    // 输入回放是独立的下一步（需要在每个 tick 前调用
    // GJBaseGameLayer::handleButton 来重放已录制的宏数据）。
    Result advanceTicks(PlayLayer* pl, int ticks, float dt = 1.0f / 240.0f);

}
