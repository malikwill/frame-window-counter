#include "InputReplayer.hpp"
#include "../Data/State.hpp"
#include <algorithm>

using namespace geode::prelude;

namespace InputReplayer {

    std::vector<ReplayEvent> buildReplayList(const std::string& testActionKey, int overrideFrame) {
        std::vector<ReplayEvent> events;
        events.reserve(g_frameActions.size());

        for (auto& [key, act] : g_frameActions) {
            ReplayEvent ev;
            ev.isPlayer2 = act.isPlayer2;
            // 被测试的那一条动作使用候选帧，其余全部保持原始录制帧号
            ev.frame = (key == testActionKey) ? overrideFrame : act.frame;
            events.push_back(ev);
        }

        std::sort(events.begin(), events.end(), [](const ReplayEvent& a, const ReplayEvent& b) {
            return a.frame < b.frame;
            });

        return events;
    }

    void driveTick(GJBaseGameLayer* layer, const std::vector<ReplayEvent>& events, int startFrame, int tickIndex) {
        if (!layer) return;
        int absoluteFrame = startFrame + tickIndex;

        for (auto& ev : events) {
            bool isPlayer1 = !ev.isPlayer2;

            // 按下：动作记录的那一帧
            if (ev.frame == absoluteFrame) {
                layer->handleButton(true, static_cast<int>(PlayerButton::Jump), isPlayer1);
            }
            // 释放：记录帧的下一 tick（视为标准单击，而非按住）
            else if (ev.frame + 1 == absoluteFrame) {
                layer->handleButton(false, static_cast<int>(PlayerButton::Jump), isPlayer1);
            }
        }
    }

}
