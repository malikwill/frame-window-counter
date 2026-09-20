#include "FastForwardRunner.hpp"
#include "ScanState.hpp"

using namespace geode::prelude;

namespace FastForwardRunner {

    Result advanceTicks(PlayLayer* pl, int ticks, float dt) {
        if (!pl || ticks <= 0) return Result::Survived;

        ScanState::g_scanDied.store(false);
        ScanState::g_scanActive.store(true);

        Result result = Result::Survived;

        for (int i = 0; i < ticks; i++) {
            pl->update(dt);

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
            pl->update(dt);

            if (ScanState::g_scanDied.load()) {
                result = Result::Died;
                break;
            }
        }

        ScanState::g_scanActive.store(false);
        return result;
    }

}
