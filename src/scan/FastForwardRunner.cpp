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

}
