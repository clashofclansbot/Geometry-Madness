#pragma once
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/loader/Mod.hpp>
#include <Geode/ui/GeodeUI.hpp>

using namespace geode::prelude;

class WhatTheHackManager {
public:
    static WhatTheHackManager* getInstance() {
        static WhatTheHackManager instance;
        return &instance;
    }
    
    void playWhatTheHeck() {
        // Versuche den "what the heck" Sound abzuspielen
        GameSoundManager::sharedManager()->playEffect("what_the_heck.ogg");
        
        // Fallback: Eigenen komischen Sound machen
        if (!GameSoundManager::sharedManager()->isBackgroundMusicPlaying()) {
            // Erstelle einen seltsamen Sound-Effekt
            createWeirdSound();
        }
        
        // Text-Effekt auf dem Screen
        showWhatTheHeckText();
    }
    
    void createWeirdSound() {
        // Erstelle einen seltsamen Audio-Effekt programmatisch
        auto action = CCSequence::create(
            CCDelayTime::create(0.5f),
            CCCallFunc::create([]() {
                // Spiele einen Standard-Sound ab
                GameSoundManager::sharedManager()->playEffect("explode_0.ogg");
            }),
            nullptr
        );
        CCDirector::sharedDirector()->getRunningScene()->runAction(action);
    }
    
    void showWhatTheHeckText() {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;
        
        // Erstelle "WHAT THE HECK" Text
        auto text = CCLabelBMFont::create("WHAT THE HECK", "bigFont.fnt");
        text->setPosition(CCDirector::sharedDirector()->getWinSize().width / 2, 
                         CCDirector::sharedDirector()->getWinSize().height / 2);
        text->setScale(0.1f);
        text->setColor({255, 0, 0});
        text->setZOrder(9999);
        scene->addChild(text);
        
        // Animierte Skalierung und Rotation
        text->runAction(CCSequence::create(
            CCSpawn::create(
                CCScaleTo::create(0.5f, 1.2f),
                CCRotateBy::create(0.5f, 360),
                nullptr
            ),
            CCScaleTo::create(0.2f, 1.0f),
            CCDelayTime::create(1.0f),
            CCFadeOut::create(0.5f),
            CCRemoveSelf::create(),
            nullptr
        ));
        
        // Zusätzliche Partikel-Effekte
        createConfettiEffect();
    }
    
    void createConfettiEffect() {
        auto emitter = CCParticleSystemQuad::create("confetti.plist");
        if (emitter) {
            emitter->setPosition(CCDirector::sharedDirector()->getWinSize().width / 2,
                               CCDirector::sharedDirector()->getWinSize().height);
            emitter->setAutoRemoveOnFinish(true);
            emitter->setZOrder(9998);
            CCDirector::sharedDirector()->getRunningScene()->addChild(emitter);
        }
    }
};

// Hook für den Limbo-Start
class $modify(GlobalLimboEffect) {
    void startLimbo() {
        GlobalLimboEffect::startLimbo();
        
        // Spiele "What the Heck" Sound beim Limbo-Start
        WhatTheHackManager::getInstance()->playWhatTheHeck();
    }
};

// Hook für FPS Explosion
class $modify(PlayLayer) {
    struct Fields {
        bool m_hasTriggeredLimbo = false;
        bool m_hasPlayedWhatTheHeck = false;
    };
    
    void triggerExplosion() {
        PlayLayer::triggerExplosion();
        
        // "What the Heck" bei der FPS-Explosion
        if (!m_fields->m_hasPlayedWhatTheHeck) {
            m_fields->m_hasPlayedWhatTheHeck = true;
            WhatTheHackManager::getInstance()->playWhatTheHeck();
        }
    }
    
    void update(float dt) {
        PlayLayer::update(dt);
        
        auto director = CCDirector::sharedDirector();
        float fps = director->getFrameRate();
        float boostedFPS = fps * 100.f + 5000.f;
        
        // "What the Heck" bei extrem hohen FPS
        if (boostedFPS > 9500.f && !m_fields->m_hasPlayedWhatTheHeck) {
            m_fields->m_hasPlayedWhatTheHeck = true;
            WhatTheHackManager::getInstance()->playWhatTheHeck();
        }
        
        // Reset nach der Explosion
        if (boostedFPS < 1000.f) {
            m_fields->m_hasPlayedWhatTheHeck = false;
        }
    }
};

// Zusätzlicher Hook für zufällige "What the Heck" Ausrufe
class $modify(CCNode) {
    void onExit() {
        // Zufällige Chance auf "What the Heck" wenn Nodes das Spiel verlassen
        if (GlobalLimboEffect::getInstance()->m_isActive && rand() % 100 < 5) {
            WhatTheHackManager::getInstance()->playWhatTheHeck();
        }
        CCNode::onExit();
    }
};

// Spezieller "What the Heck" Button im Menü
class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        
        // Füge einen geheimen "What the Heck" Button hinzu
        auto wtbButton = CCMenuItemSpriteExtra::create(
            ButtonSprite::create("WTH"),
            this,
            menu_selector(MenuLayer::onWhatTheHeck)
        );
        
        auto menu = this->getChildByID("bottom-menu");
        if (menu) {
            wtbButton->setPosition(0, -120);
            wtbButton->setScale(0.6f);
            menu->addChild(wtbButton);
        }
        
        return true;
    }
    
    void onWhatTheHeck(CCObject* sender) {
        WhatTheHackManager::getInstance()->playWhatTheHeck();
        
        // Zusätzlicher Chaos-Effekt beim Klick
        GlobalLimboEffect::getInstance()->startLimbo();
    }
};

// Editor "What the Heck" Trigger
class $modify(EditorUI) {
    void onDuplicate(CCObject* sender) {
        EditorUI::onDuplicate(sender);
        
        // "What the Heck" wenn zu viele Objekte dupliziert werden
        static int duplicateCount = 0;
        duplicateCount++;
        
        if (duplicateCount > 10) {
            duplicateCount = 0;
            WhatTheHackManager::getInstance()->playWhatTheHeck();
        }
    }
    
    void onPaste(CCObject* sender) {
        EditorUI::onPaste(sender);
        
        // Zufälliger "What the Heck" beim Einfügen
        if (rand() % 100 < 15) {
            WhatTheHackManager::getInstance()->playWhatTheHeck();
        }
    }
};
