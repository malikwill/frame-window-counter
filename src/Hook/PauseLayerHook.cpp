#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "../Data/State.hpp"
#include "../UI/FrameActionPopup.hpp"

using namespace geode::prelude;

class $modify(MyPauseLayer, PauseLayer) {
    struct Fields {
        Ref<CCMenuItemSpriteExtra> m_editorBtn;
    };

    //暂停界面初始化与自定义按钮构建
    void customSetup() {
        PauseLayer::customSetup();
        loadModData();

        auto btn = CCMenuItemSpriteExtra::create(
            CCSprite::createWithSpriteFrameName("GJ_timeIcon_001.png"),
            this,
            menu_selector(MyPauseLayer::onOpenEditor)
        );
        btn->setScale(0.8f);
        btn->setID("frame-editor-btn"_spr);
        m_fields->m_editorBtn = btn;

        this->attachEditorButton();

        // Online levels can fetch extra metadata (rating/leaderboard/etc.) after
        // customSetup runs, which can rebuild/clear the right-button-menu and
        // silently drop our button. Periodically make sure it's still attached
        // for as long as the pause layer is open, self-healing if it ever gets
        // knocked out.
        this->schedule(schedule_selector(MyPauseLayer::ensureEditorButtonAttached), 0.3f);
    }

    // (Re)attaches the editor button to right-button-menu, falling back to a
    // manually-positioned menu of our own if that node truly doesn't exist.
    void attachEditorButton() {
        auto btn = m_fields->m_editorBtn;
        if (!btn || btn->getParent()) return;

        auto rightMenu = this->getChildByID("right-button-menu");
        if (rightMenu) {
            rightMenu->addChild(btn);
            rightMenu->updateLayout();
            return;
        }

        // Fallback: "right-button-menu" is a Node ID provided by geode.node-ids
        // (declared as a required dependency), so this should always exist.
        // Just in case some level/game-mode variant of PauseLayer doesn't have
        // it though, add our own small menu so the button is never silently lost.
        geode::log::warn("Frame Window Counter: \"right-button-menu\" not found on PauseLayer, using fallback position");

        auto fallbackMenu = this->getChildByID("frame-editor-fallback-menu"_spr);
        if (!fallbackMenu) {
            auto menu = CCMenu::create();
            menu->setID("frame-editor-fallback-menu"_spr);
            menu->setContentSize(btn->getContentSize());

            auto winSize = CCDirector::sharedDirector()->getWinSize();
            menu->setPosition({ winSize.width - 30.f, winSize.height - 30.f });
            this->addChild(menu, 100);
            fallbackMenu = menu;
        }

        btn->setPosition(fallbackMenu->getContentSize() / 2);
        fallbackMenu->addChild(btn);
    }

    // Re-attaches the button if some other UI (e.g. online level metadata
    // finishing a late fetch) rebuilt right-button-menu and dropped it.
    void ensureEditorButtonAttached(float) {
        this->attachEditorButton();
    }

    //点击暂停界面按钮打开帧数编辑窗口
    void onOpenEditor(CCObject*) {
        if (auto popup = FrameActionPopup::create()) {
            popup->setID("FrameActionPopup"_spr);
            popup->showInstant();
        }
    }
};