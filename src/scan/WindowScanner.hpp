#pragma once
#include <Geode/Geode.hpp>
#include <string>

// 自动帧窗口扫描器：以已知可用的录制帧为中心，向两侧扩张探测，
// 找出该点击实际的成功边界，取代手动逐帧试验。
//
// 使用前提：调用方需要先在合适的时机（该点击发生之前）用
// CheckpointRunner::snapshot() 拍摄一个检查点，并记录下拍摄时关卡的
// 绝对帧号（startFrame）。扫描过程中会反复恢复到这个检查点重新测试，
// 结束后也会恢复回该检查点，不会留下任何脚步影响到玩家当前的实际进度。
//
// 整个扫描是同步阻塞执行的（不能跨线程操作关卡状态），但由于每次测试
// 都是自行推进物理 tick 而非等待真实时间，实际耗时通常远低于一帧的
// 感知阈值，即使探测半径较大也是如此。
namespace WindowScanner {

    struct ScanResult {
        bool success = false;           // 中心帧本身測試失败/输入无效时为 false
        int centerFrame = 0;
        int lowerBound = 0;             // 找到的最早仍然成功的帧
        int upperBound = 0;             // 找到的最晚仍然成功的帧
        int windowSize = 0;             // (upperBound - lowerBound) + 1
        bool lowerHitSearchLimit = false; // 达到 maxRadius 仍未找到失败帧（结果可能不完整）
        bool upperHitSearchLimit = false;
    };

    // testActionKey  : g_frameActions 中被测试的那条动作的键
    // checkpoint      : 该点击发生前拍摄的检查点
    // startFrame      : 拍摄检查点时关卡的绝对帧号
    // lookaheadTicks  : 落点判定为"成功"后，还要继续模拟这么多 tick 才能确认
    //                    没有发生延迟失败（例如落地位置细微偏差导致的后续死亡）
    // maxRadius       : 单侧最多探测多少帧，超出仍未失败则视为达到搜索上限
    ScanResult scanWindow(
        PlayLayer* pl,
        CheckpointObject* checkpoint,
        int startFrame,
        const std::string& testActionKey,
        int centerFrame,
        int lookaheadTicks = 30,
        int maxRadius = 60,
        float dt = 1.0f / 240.0f
    );

}
