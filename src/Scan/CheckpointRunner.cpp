#include "CheckpointRunner.hpp"

using namespace geode::prelude;

namespace CheckpointRunner {

    CheckpointObject* snapshot(PlayLayer* pl) {
        if (!pl) return nullptr;
        // createCheckpoint() 独立创建一个 CheckpointObject，捕获当前的游戏状态、
        // 玩家状态、音频状态等。不会自动加入 m_checkpointArray。
        return pl->createCheckpoint();
    }

    void restore(PlayLayer* pl, CheckpointObject* checkpoint) {
        if (!pl || !checkpoint) return;
        // loadFromCheckpoint() 会将关卡恢复到该检查点记录的完整状态。
        pl->loadFromCheckpoint(checkpoint);
    }

}
