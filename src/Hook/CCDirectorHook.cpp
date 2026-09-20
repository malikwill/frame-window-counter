#include <Geode/Geode.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <optional>
#include "../Data/State.hpp"
#include "../Common.hpp"
#include "../UI/AddFramePopup.hpp"
#include "../UI/FrameActionPopup.hpp"
#include "../UI/PrecisionSettingsPopup.hpp"
#include "../UI/LStarCalcSettingsPopup.hpp"
#include "../UI/LabelPresetPopup.hpp"
#include "../UI/WindowPresetPopup.hpp"

using namespace geode::prelude;

// 递归检测当前场景树中是否有任何文本输入框处于聚焦输入状态
static bool isAnyTextInputFocused(CCNode* root) {
    if (!root) return false;

    // CCTextInputNode 的选中焦点变量名为 m_selected
    if (auto textInput = typeinfo_cast<CCTextInputNode*>(root)) {
        if (textInput->m_selected) {
            return true;
        }
    }

    if (auto children = root->getChildren()) {
        for (int i = 0; i < children->count(); i++) {
            auto child = static_cast<CCNode*>(children->objectAtIndex(i));
            if (isAnyTextInputFocused(child)) {
                return true;
            }
        }
    }
    return false;
}

// 打开/关闭帧数编辑器弹窗，供快捷键调用
static void toggleFrameEditor() {
    auto scene = CCDirector::sharedDirector()->getRunningScene();
    if (!scene || typeinfo_cast<CCTransitionScene*>(scene)) return;

    // 如果用户当前正在任何输入框内打字，忽略快捷键触发
    if (isAnyTextInputFocused(scene)) return;

    bool closedAny = false;
    if (auto children = scene->getChildren()) {
        for (int i = children->count() - 1; i >= 0; i--) {
            auto child = static_cast<CCNode*>(children->objectAtIndex(i));

            // 关闭所有关联的弹窗
            if (typeinfo_cast<AddFramePopup*>(child) ||
                typeinfo_cast<FrameActionPopup*>(child) ||
                typeinfo_cast<PrecisionSettingsPopup*>(child) ||
                typeinfo_cast<LStarCalcSettingsPopup*>(child) ||
                typeinfo_cast<LabelPresetPopup*>(child) ||
                typeinfo_cast<WindowPresetPopup*>(child)) {
                child->removeFromParentAndCleanup(true);
                closedAny = true;
            }
        }
    }

    // 若未打开任何弹窗，则弹出帧数编辑器
    if (!closedAny) {
        if (auto popup = FrameActionPopup::create()) {
            popup->setID("FrameActionPopup"_spr);
            popup->showInstant();
        }
    }
}

// 将设置里的单个字母（A-Z）映射为对应的 enumKeyCodes 值。
// 未识别或为空时返回空值，快捷键将不会触发。
static std::optional<cocos2d::enumKeyCodes> letterToKeyCode(char c) {
    switch (std::toupper(static_cast<unsigned char>(c))) {
        case 'A': return cocos2d::KEY_A;
        case 'B': return cocos2d::KEY_B;
        case 'C': return cocos2d::KEY_C;
        case 'D': return cocos2d::KEY_D;
        case 'E': return cocos2d::KEY_E;
        case 'F': return cocos2d::KEY_F;
        case 'G': return cocos2d::KEY_G;
        case 'H': return cocos2d::KEY_H;
        case 'I': return cocos2d::KEY_I;
        case 'J': return cocos2d::KEY_J;
        case 'K': return cocos2d::KEY_K;
        case 'L': return cocos2d::KEY_L;
        case 'M': return cocos2d::KEY_M;
        case 'N': return cocos2d::KEY_N;
        case 'O': return cocos2d::KEY_O;
        case 'P': return cocos2d::KEY_P;
        case 'Q': return cocos2d::KEY_Q;
        case 'R': return cocos2d::KEY_R;
        case 'S': return cocos2d::KEY_S;
        case 'T': return cocos2d::KEY_T;
        case 'U': return cocos2d::KEY_U;
        case 'V': return cocos2d::KEY_V;
        case 'W': return cocos2d::KEY_W;
        case 'X': return cocos2d::KEY_X;
        case 'Y': return cocos2d::KEY_Y;
        case 'Z': return cocos2d::KEY_Z;
        default:  return std::nullopt;
    }
}

// 跨平台快捷键实现：通过 cocos2d 的按键分发器捕获按键，
// 取代旧版本仅限 Windows 的 GetAsyncKeyState 轮询实现。
// 具体按键由 mod 设置中的 "editor-hotkey" 字符串决定，默认 'O'，与旧版本一致。
class $modify(MyKeyboardDispatcher, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(cocos2d::enumKeyCodes key, bool isKeyDown, bool isKeyRepeat, double dt) {
        bool handled = CCKeyboardDispatcher::dispatchKeyboardMSG(key, isKeyDown, isKeyRepeat, dt);

        if (isKeyDown && !isKeyRepeat) {
            auto hotkeyStr = Mod::get()->getSettingValue<std::string>("editor-hotkey");
            if (!hotkeyStr.empty()) {
                if (auto hotkeyKey = letterToKeyCode(hotkeyStr[0]); hotkeyKey && key == *hotkeyKey) {
                    toggleFrameEditor();
                }
            }
        }

        return handled;
    }
};

class $modify(MyDirector, CCDirector) {
    void drawScene() {
        CCDirector::drawScene();

        auto scene = this->getRunningScene();
        if (!scene || typeinfo_cast<CCTransitionScene*>(scene)) {
            return;
        }

        // 自动保存逻辑
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - g_lastAutoSaveTime).count() >= AUTOSAVETIME) {
            g_lastAutoSaveTime = now;
            if (auto pl = PlayLayer::get(); pl && pl->m_level) {
                doAutoSave(pl->m_level);
            }
        }

        // 关卡运行中的实时帧高亮追踪
        if (g_modEnabled) {
            if (auto playLayer = PlayLayer::get()) {
                if (!playLayer->m_isPaused && playLayer->m_player1 && !playLayer->m_player1->m_isDead) {
                    if (auto popup = typeinfo_cast<FrameActionPopup*>(scene->getChildByID("FrameActionPopup"_spr))) {
                        int currentFrame = static_cast<int>(playLayer->m_gameState.m_levelTime * g_macroFps);
                        popup->doTrackingTick(currentFrame);
                    }
                }
            }
        }
    }
};
