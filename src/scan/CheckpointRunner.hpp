#pragma once
#include <Geode/Geode.hpp>

// 冻结/恢复关卡状态的最小封装，用于未来的自动帧窗口扫描功能。
// 目前仅验证快照/恢复本身是否可靠 —— 不包含快进或输入回放逻辑。
namespace CheckpointRunner {

    // 在当前状态创建一个检查点（不会加入 PlayLayer 自身的练习模式检查点列表，
    // 只由调用方持有引用，随时可以调用 restore() 还原）。
    // 返回 nullptr 表示当前没有正在进行的关卡。
    CheckpointObject* snapshot(PlayLayer* pl);

    // 将关卡恢复到给定检查点的状态。
    void restore(PlayLayer* pl, CheckpointObject* checkpoint);

}
