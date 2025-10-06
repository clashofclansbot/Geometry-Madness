#pragma once
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/binding/Slider.hpp>

using namespace geode::prelude;

class ScrollableProgressManager {
public:
    static ScrollableProgressManager* getInstance() {
        static ScrollableProgressManager instance;
        return &instance;
    }
    
    bool m_isScrolling = false;
    float m_scrollSpeed = 0.f;
    Slider* m_progressSlider = nullptr;
    CCLabelBMFont* m_debugLabel = nullptr;
    
    void setProgressSlider(Slider* slider) {
        m_progressSlider = slider;
        
        // Deaktiviere die originale Progress Bar Funktionalität
        if (m_progressSlider) {
            m_progressSlider->setEnabled(false);
            m_progressSlider->setTouchEnabled(false);
        }
    }
    
    void handleScroll(float deltaY) {
        if (!m_progressSlider || !PlayLayer::get()) return;
        
        m_isScrolling = true;
        m_scrollSpeed = deltaY * 0.5f;
        
        auto playLayer = PlayLayer::get();
        float currentProgress = playLayer->m_progressTimer->getPercentage();
        
        // Berechne neue Progress basierend auf Scroll
        float newProgress = currentProgress + (deltaY * 2.0f);
        newProgress = std::max(0.f, std::min(100.f, newProgress));
        
        // Setze die neue Progress
        setGameProgress(newProgress);
        
        // Debug Info
        showDebugInfo(newProgress, deltaY);
    }
    
    void setGameProgress(float progress) {
        if (!PlayLayer::get()) return;
        
        auto playLayer = PlayLayer::get();
        
        // Setze Progress Timer
        playLayer->m_progressTimer->setPercentage(progress);
        
        // Aktualisiere Slider Position
        if (m_progressSlider) {
            m_progressSlider->setValue(progress / 100.f);
        }
        
        // Setze Spieler-Position basierend auf Progress
        updatePlayerPosition(progress);
    }
    
    void updatePlayerPosition(float progress) {
        auto playLayer = PlayLayer::get();
        if (!playLayer || !playLayer->m_level) return;
        
        // Berechne Position basierend auf Progress
        float levelLength = playLayer->m_level->m_levelLength;
        float targetX = (progress / 100.f) * levelLength;
        
        // Setze Spieler-Position
        playLayer->m_player1->setPositionX(targetX);
        
        // Update Camera
        playLayer->setStartPosObject(nullptr);
        playLayer->updateCamera(0.f);
    }
    
    void showDebugInfo(float progress, float deltaY) {
        if (!m_debugLabel) {
            auto director = CCDirector::sharedDirector();
            m_debugLabel = CCLabelBMFont::create("", "chatFont.fnt");
            m_debugLabel->setPosition(100, 100);
            m_debugLabel->setScale(0.6f);
            m_debugLabel->setColor({255, 255, 0});
            m_debugLabel->setZOrder(9999);
            director->getRunningScene()->addChild(m_debugLabel);
        }
        
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "Progress: %.1f%%\nScroll: %.2f", progress, deltaY);
        m_debugLabel->setString(buffer);
        
        // Verstecke Debug nach kurzer Zeit
        m_debugLabel->stopAllActions();
        m_debugLabel->runAction(CCSequence::create(
            CCDelayTime::create(2.0f),
            CCFadeOut::create(0.5f),
            nullptr
        ));
    }
    
    void stopScrolling() {
        m_isScrolling = false;
        m_scrollSpeed = 0.f;
    }
};

// Hook für PlayLayer um die Progress Bar zu finden
class $modify(PlayLayer) {
    struct Fields {
        bool m_hasHookedProgressBar = false;
    };
    
    bool init(GJGameLevel* level) {
        if (!PlayLayer::init(level)) return false;
        
        // Warte kurz und dann hooke die Progress Bar
        this->scheduleOnce(schedule_selector(PlayLayer::hookProgressBar), 0.5f);
        
        return true;
    }
    
    void hookProgressBar(float dt) {
        if (m_fields->m_hasHookedProgressBar) return;
        
        // Finde den Progress Slider im UILayer
        auto uiLayer = this->getChildByID("UILayer");
        if (uiLayer) {
            // Durchsuche Children nach dem Slider
            auto children = uiLayer->getChildren();
            for (auto* child : children) {
                if (auto* slider = dynamic_cast<Slider*>(child)) {
                    // Wahrscheinlich die Progress Bar gefunden
                    ScrollableProgressManager::getInstance()->setProgressSlider(slider);
                    m_fields->m_hasHookedProgressBar = true;
                    
                    // Füge "Scroll Enabled" Text hinzu
                    showScrollHint();
                    break;
                }
            }
        }
    }
    
    void showScrollHint() {
        auto hintLabel = CCLabelBMFont::create("Scroll to navigate!", "bigFont.fnt");
        hintLabel->setPosition(CCDirector::sharedDirector()->getWinSize().width / 2, 50);
        hintLabel->setScale(0.4f);
        hintLabel->setColor({0, 255, 255});
        hintLabel->setZOrder(9999);
        this->addChild(hintLabel);
        
        // Animierte Hinweis
        hintLabel->runAction(CCSequence::create(
            CCDelayTime::create(3.0f),
            CCFadeOut::create(1.0f),
            CCRemoveSelf::create(),
            nullptr
        ));
    }
    
    void update(float dt) {
        PlayLayer::update(dt);
        
        // Falls gescrollt wird, update die Progress
        auto scrollManager = ScrollableProgressManager::getInstance();
        if (scrollManager->m_isScrolling) {
            // Hier könnten zusätzliche Updates während Scroll passieren
        }
    }
};

// Hook für UILayer um Scroll-Events zu fangen
class $modify(UILayer) {
    void scrollWheel(float deltaX, float deltaY) {
        // UILayer::scrollWheel(deltaX, deltaY); // NICHT aufrufen!
        
        // Fange Scroll-Events ab und leite an Progress Manager weiter
        ScrollableProgressManager::getInstance()->handleScroll(deltaY);
        
        // Optional: "What the Heck" bei extremem Scroll
        if (fabs(deltaY) > 10.f) {
            WhatTheHackManager::getInstance()->playWhatTheHeck();
        }
    }
    
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) {
        // Verhindere Touch-Events auf der deaktivierten Progress Bar
        auto slider = ScrollableProgressManager::getInstance()->m_progressSlider;
        if (slider && slider->getParent()) {
            CCPoint touchLocation = touch->getLocation();
            CCRect sliderRect = slider->getRect();
            
            if (sliderRect.containsPoint(touchLocation)) {
                // Touch auf Progress Bar - ignoriere es
                return false;
            }
        }
        
        return UILayer::ccTouchBegan(touch, event);
    }
};

// Hook für den Slider selbst um ihn komplett zu deaktivieren
class $modify(Slider) {
    void setValue(float value) {
        // Erlaube nur Wertänderungen durch unseren Scroll Manager
        if (!ScrollableProgressManager::getInstance()->m_isScrolling) {
            Slider::setValue(value);
        }
    }
    
    bool touchPressed(CCTouch* touch) {
        // Blockiere alle Touch-Events auf dem Slider
        return false;
    }
    
    bool touchMoved(CCTouch* touch) {
        // Blockiere alle Touch-Events auf dem Slider  
        return false;
    }
    
    bool touchReleased(CCTouch* touch) {
        // Blockiere alle Touch-Events auf dem Slider
        return false;
    }
};

// Zusätzlicher Hook für Mouse-Scroll in Editor
class $modify(LevelEditorLayer) {
    void scrollWheel(float deltaX, float deltaY) {
        // Im Editor normal scrollen lassen
        LevelEditorLayer::scrollWheel(deltaX, deltaY);
    }
};
