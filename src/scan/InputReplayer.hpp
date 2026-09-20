#pragma once
#include <Geode/Geode.hpp>
#include <vector>
#include <string>

// 基于已导入/编辑的宏数据 (g_frameActions) 在扫描过程中重放输入。
//
// 重要限制：g_frameActions 只保留了"帧号 + 是否 P2"，并不记录具体是哪个按钮，
// 也不记录按下/释放的区别（导入时这些信息已被丢弃，参见 FileIO::importReplay）。
// 因此这里统一将每条记录视为一次 Jump 单击：在其帧按下，下一 tick 自动释放。
// 这覆盖了绝大多数普通关卡输入，但无法还原持续按住或左右移动类操作。
namespace InputReplayer {

    struct ReplayEvent {
        int frame = 0;          // 绝对帧号，与关卡 m_gameState 的帧计数对齐
        bool isPlayer2 = false;
    };

    // 从当前的 g_frameActions 构建回放事件列表，并将 testActionKey 对应的
    // 那一条动作的帧号替换为 overrideFrame —— 这就是扫描器本次测试的候选帧。
    // 其余动作保持原始录制帧号不变。返回结果按帧号升序排列。
    std::vector<ReplayEvent> buildReplayList(const std::string& testActionKey, int overrideFrame);

    // 在快进循环的每个 tick 调用一次（在 FastForwardRunner 每次 update() 之前）。
    // startFrame 为执行快照时关卡的绝对帧号（对应 tickIndex == 0）；
    // tickIndex 为相对快照点的 tick 序号，从 0 开始递增。
    void driveTick(GJBaseGameLayer* layer, const std::vector<ReplayEvent>& events, int startFrame, int tickIndex);

}
