#include <Geode/Geode.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include "../Data/State.hpp"
#include "../UI/FrameActionPopup.hpp"

using namespace geode::prelude;

class $modify(MyPauseLayer, PauseLayer) {

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

        auto rightMenu = this->getChildByID("right-button-menu");
        if (rightMenu) {
            rightMenu->addChild(btn);
            rightMenu->updateLayout();
        }
        else {
            // Fallback: "right-button-menu" is a Node ID provided by geode.node-ids
            // (declared as a required dependency), so this should always exist.
            // Just in case some level/game-mode variant of PauseLayer doesn't have
            // it though, add our own small menu so the button is never silently lost.
            geode::log::warn("Frame Window Counter: \"right-button-menu\" not found on PauseLayer, using fallback position");

            auto fallbackMenu = CCMenu::create();
            fallbackMenu->setID("frame-editor-fallback-menu"_spr);
            fallbackMenu->addChild(btn);
            fallbackMenu->setContentSize(btn->getContentSize());
            btn->setPosition(fallbackMenu->getContentSize() / 2);

            auto winSize = CCDirector::sharedDirector()->getWinSize();
            fallbackMenu->setPosition({ winSize.width - 30.f, winSize.height - 30.f });
            this->addChild(fallbackMenu, 100);
        }
    }

    //点击暂停界面按钮打开帧数编辑窗口
    void onOpenEditor(CCObject*) {
        if (auto popup = FrameActionPopup::create()) {
            popup->setID("FrameActionPopup"_spr);
            popup->showInstant();
        }
    }
};