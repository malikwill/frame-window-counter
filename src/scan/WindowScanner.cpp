#include "WindowScanner.hpp"
#include "CheckpointRunner.hpp"
#include "FastForwardRunner.hpp"
#include "InputReplayer.hpp"

using namespace geode::prelude;

namespace WindowScanner {

    // 测试单个候选帧：恢复到检查点，重放整段宏（被测动作改为候选帧），
    // 快进到候选帧之后再加上一段 lookahead，返回是否全程存活。
    static bool testCandidate(
        PlayLayer* pl,
        CheckpointObject* checkpoint,
        int startFrame,
        const std::string& testActionKey,
        int candidateFrame,
        int lookaheadTicks,
        float dt
    ) {
        int ticksNeeded = (candidateFrame - startFrame) + lookaheadTicks;
        if (ticksNeeded <= 0) return false; // 候选帧早于检查点本身，无效

        CheckpointRunner::restore(pl, checkpoint);

        auto events = InputReplayer::buildReplayList(testActionKey, candidateFrame);
        auto result = FastForwardRunner::advanceTicksWithReplay(pl, ticksNeeded, startFrame, events, dt);

        return result == FastForwardRunner::Result::Survived;
    }

    ScanResult scanWindow(
        PlayLayer* pl,
        CheckpointObject* checkpoint,
        int startFrame,
        const std::string& testActionKey,
        int centerFrame,
        int lookaheadTicks,
        int maxRadius,
        float dt
    ) {
        ScanResult res;
        res.centerFrame = centerFrame;

        if (!pl || !checkpoint) {
            geode::log::error(
                "[FrameWindowCounter] WindowScanner: scanWindow called with null {} for action '{}', aborting",
                !pl ? "PlayLayer" : "checkpoint", testActionKey
            );
            return res;
        }

        // 中心帧本身应当是已知可行的录制帧；如果连它都失败，
        // 说明测试环境（检查点/回放数据）本身有问题，直接中止。
        if (!testCandidate(pl, checkpoint, startFrame, testActionKey, centerFrame, lookaheadTicks, dt)) {
            geode::log::error(
                "[FrameWindowCounter] WindowScanner: center frame {} (action '{}') failed sanity check, aborting scan",
                centerFrame, testActionKey
            );
            CheckpointRunner::restore(pl, checkpoint);
            return res;
        }

        res.success = true;
        res.lowerBound = centerFrame;
        res.upperBound = centerFrame;

        // 向后扩张：中心 +1, +2, ... 直到失败
        res.upperHitSearchLimit = true;
        for (int offset = 1; offset <= maxRadius; offset++) {
            int candidate = centerFrame + offset;
            if (testCandidate(pl, checkpoint, startFrame, testActionKey, candidate, lookaheadTicks, dt)) {
                res.upperBound = candidate;
            }
            else {
                res.upperHitSearchLimit = false;
                break;
            }
        }
        if (res.upperHitSearchLimit) {
            geode::log::warn(
                "[FrameWindowCounter] WindowScanner: upper bound search hit maxRadius ({}) without failing, "
                "true upper bound may be higher than {}",
                maxRadius, res.upperBound
            );
        }

        // 向前扩张：中心 -1, -2, ... 直到失败
        res.lowerHitSearchLimit = true;
        for (int offset = 1; offset <= maxRadius; offset++) {
            int candidate = centerFrame - offset;
            if (testCandidate(pl, checkpoint, startFrame, testActionKey, candidate, lookaheadTicks, dt)) {
                res.lowerBound = candidate;
            }
            else {
                res.lowerHitSearchLimit = false;
                break;
            }
        }
        if (res.lowerHitSearchLimit) {
            geode::log::warn(
                "[FrameWindowCounter] WindowScanner: lower bound search hit maxRadius ({}) without failing, "
                "true lower bound may be lower than {}",
                maxRadius, res.lowerBound
            );
        }

        res.windowSize = (res.upperBound - res.lowerBound) + 1;

        geode::log::info(
            "[FrameWindowCounter] WindowScanner: finished scan for action '{}' - bounds [{}, {}], window size {}",
            testActionKey, res.lowerBound, res.upperBound, res.windowSize
        );

        // 扫描结束后恢复到检查点，不留下任何影响实际游玩状态的残留
        CheckpointRunner::restore(pl, checkpoint);

        return res;
    }

}
