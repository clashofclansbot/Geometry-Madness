#pragma once
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/EditorUI.hpp>
#include <Geode/binding/GameSoundManager.hpp>
#include <vector>
#include <random>

using namespace geode::prelude;

class MemeManager {
public:
    static MemeManager* getInstance() {
        static MemeManager instance;
        return &instance;
    }
    
    struct Meme {
        std::string soundName;
        std::string displayText;
        ccColor3B textColor;
        float duration;
    };
    
    std::vector<Meme> m_memes;
    float m_memeTimer = 0.f;
    bool m_isEnabled = true;
    CCLabelBMFont* m_memeText = nullptr;
    
    MemeManager() {
        setupMemes();
    }
    
    void setupMemes() {
        // Liste von ikonischen Meme-Sounds und Texten
        m_memes = {
            // Deutsche Memes
            {"boom", "BOOM!", {255, 0, 0}, 2.0f},
            {"ohno", "OH NEIN!", {255, 100, 0}, 1.5f},
            {"bruh", "BRUH MOMENT", {0, 255, 255}, 2.0f},
            {"sheesh", "SHEEEESH", {255, 215, 0}, 1.5f},
            {"sus", "SUSSY BAKA", {255, 0, 255}, 2.0f},
            
            // Geometry Dash Memes
            {"ready", "LET'S GO!", {0, 255, 0}, 1.5f},
            {"frame", "BAD PC FRAME", {100, 100, 100}, 2.0f},
            {"lag", "LAG SPIKEE", {255, 0, 0}, 1.5f},
            {"verify", "VERIFYING...", {0, 200, 255}, 2.0f},
            
            // International Memes
            {"vine", "VINE BOOM", {0, 255, 100}, 1.5f},
            {"rizz", "RIZZLER", {255, 100, 255}, 2.0f},
            {"sigma", "SIGMA GRINDSET", {0, 0, 255}, 2.0f},
            {"skill", "SKILL ISSUE", {255, 50, 50}, 1.5f},
            
            // Soundeffekte die in GD existieren
            {"explode_0", "KABOOM!", {255, 100, 0}, 1.5f},
            {"explode_1", "BOOM!", {255, 50, 0}, 1.5f},
            {"explode_2", "BLAST!", {255, 150, 0}, 1.5f},
            {"explode_3", "POW!", {255, 200, 0}, 1.0f},
            {"explode_4", "BANG!", {255, 0, 100}, 1.0f},
            {"explode_5", "SMASH!", {200, 0, 255}, 1.5f},
            {"explode_6", "CRASH!", {0, 100, 255}, 1.5f},
            {"explode_7", "WHAM!", {255, 0, 200}, 1.0f},
            {"explode_8", "KAPOW!", {100, 255, 0}, 1.5f},
            {"explode_9", "BLAM!", {255, 100, 100}, 1.0f},
            {"explode_10", "BOOF!", {100, 100, 255}, 1.0f},
            {"explode_11", "ZOOM!", {0, 255, 200}, 1.5f},
            {"explode_12", "YEET!", {255, 255, 0}, 1.0f},
            {"explode_13", "WOOSH!", {100, 255, 255}, 1.5f},
            {"explode_14", "SLAM!", {255, 0, 150}, 1.0f},
            {"explode_15", "POP!", {150, 0, 255}, 1.0f},
            {"explode_16", "FWOOSH!", {255, 150, 0}, 1.5f},
            {"explode_17", "KAPLOW!", {0, 150, 255}, 1.5f},
            {"explode_18", "ZAP!", {200, 255, 0}, 1.0f},
            {"explode_19", "VWIP!", {255, 0, 255}, 1.0f}
        };
    }
    
    void update(float dt) {
        if (!m_isEnabled) return;
        
        m_memeTimer += dt;
        if (m_memeTimer >= 10.0f) { // Alle 10 Sekunden
            m_memeTimer = 0.f;
            playRandomMeme();
        }
    }
    
    void playRandomMeme() {
        if (m_memes.empty()) return;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dis(0, m_memes.size() - 1);
        
        size_t randomIndex = dis(gen);
        Meme& randomMeme = m_memes[randomIndex];
        
        // Spiele Sound ab
        GameSoundManager::sharedManager()->playEffect(randomMeme.soundName.c_str());
        
        // Zeige Meme-Text an
        showMemeText(randomMeme.displayText, randomMeme.textColor, randomMeme.duration);
        
        // Zusätzliche visuelle Effekte
        createMemeEffects();
        
        // "What the Heck" bei besonders wilden Memes
        if (randomIndex % 7 == 0) { // Jedes 7. Meme
            WhatTheHackManager::getInstance()->playWhatTheHeck();
        }
    }
    
    void showMemeText(const std::string& text, ccColor3B color, float duration) {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;
        
        // Entferne vorherigen Text
        if (m_memeText) {
            m_memeText->removeFromParent();
            m_memeText = nullptr;
        }
        
        // Erstelle neuen Text
        m_memeText = CCLabelBMFont::create(text.c_str(), "bigFont.fnt");
        m_memeText->setPosition(CCDirector::sharedDirector()->getWinSize().width / 2, 
                               CCDirector::sharedDirector()->getWinSize().height / 2 + 100);
        m_memeText->setColor(color);
        m_memeText->setScale(0.8f);
        m_memeText->setZOrder(9999);
        scene->addChild(m_memeText);
        
        // Animierte Text-Erscheinung
        m_memeText->setOpacity(0);
        m_memeText->runAction(CCSequence::create(
            CCSpawn::create(
                CCFadeIn::create(0.3f),
                CCScaleTo::create(0.3f, 1.0f),
                CCMoveBy::create(0.3f, CCPoint(0, 20)),
                nullptr
            ),
            CCDelayTime::create(duration),
            CCSpawn::create(
                CCFadeOut::create(0.5f),
                CCScaleTo::create(0.5f, 1.2f),
                nullptr
            ),
            CCRemoveSelf::create(),
            nullptr
        ));
        
        m_memeText = nullptr; // Reset nach Animation
    }
    
    void createMemeEffects() {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        if (!scene) return;
        
        // Zufällige Partikel-Effekte
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dis(1, 5);
        
        switch (dis(gen)) {
            case 1:
                createExplosionEffect();
                break;
            case 2:
                createSparkleEffect();
                break;
            case 3:
                createRainbowEffect();
                break;
            case 4:
                createScreenShake();
                break;
            case 5:
                createColorFlash();
                break;
        }
    }
    
    void createExplosionEffect() {
        auto emitter = CCParticleExplosion::create();
        emitter->setPosition(CCDirector::sharedDirector()->getWinSize().width / 2,
                           CCDirector::sharedDirector()->getWinSize().height / 2);
        emitter->setScale(0.5f);
        emitter->setAutoRemoveOnFinish(true);
        emitter->setZOrder(9998);
        CCDirector::sharedDirector()->getRunningScene()->addChild(emitter);
    }
    
    void createSparkleEffect() {
        auto emitter = CCParticleSystemQuad::create("sparkle.plist");
        if (emitter) {
            emitter->setPosition(CCDirector::sharedDirector()->getWinSize().width / 2,
                               CCDirector::sharedDirector()->getWinSize().height / 2);
            emitter->setAutoRemoveOnFinish(true);
            emitter->setZOrder(9998);
            CCDirector::sharedDirector()->getRunningScene()->addChild(emitter);
        }
    }
    
    void createRainbowEffect() {
        auto layer = CCLayerColor::create({255, 0, 0, 50});
        layer->setContentSize(CCDirector::sharedDirector()->getWinSize());
        layer->setPosition(0, 0);
        layer->setZOrder(9997);
        CCDirector::sharedDirector()->getRunningScene()->addChild(layer);
        
        layer->runAction(CCSequence::create(
            CCTintTo::create(0.2f, 255, 0, 0),
            CCTintTo::create(0.2f, 255, 127, 0),
            CCTintTo::create(0.2f, 255, 255, 0),
            CCTintTo::create(0.2f, 0, 255, 0),
            CCTintTo::create(0.2f, 0, 0, 255),
            CCTintTo::create(0.2f, 75, 0, 130),
            CCTintTo::create(0.2f, 148, 0, 211),
            CCFadeOut::create(0.5f),
            CCRemoveSelf::create(),
            nullptr
        ));
    }
    
    void createScreenShake() {
        auto scene = CCDirector::sharedDirector()->getRunningScene();
        scene->runAction(CCSequence::create(
            CCMoveBy::create(0.05f, CCPoint(10, 5)),
            CCMoveBy::create(0.05f, CCPoint(-8, -7)),
            CCMoveBy::create(0.05f, CCPoint(6, 8)),
            CCMoveBy::create(0.05f, CCPoint(-5, -4)),
            CCMoveBy::create(0.05f, CCPoint(3, 2)),
            CCMoveBy::create(0.05f, CCPoint(-2, -1)),
            nullptr
        ));
    }
    
    void createColorFlash() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<GLubyte> colorDis(100, 255);
        
        ccColor3B flashColor = {
            colorDis(gen),
            colorDis(gen),
            colorDis(gen)
        };
        
        auto flash = CCLayerColor::create({flashColor.r, flashColor.g, flashColor.b, 80});
        flash->setContentSize(CCDirector::sharedDirector()->getWinSize());
        flash->setPosition(0, 0);
        flash->setZOrder(9996);
        CCDirector::sharedDirector()->getRunningScene()->addChild(flash);
        
        flash->runAction(CCSequence::create(
            CCFadeOut::create(0.3f),
            CCRemoveSelf::create(),
            nullptr
        ));
    }
    
    void setEnabled(bool enabled) {
        m_isEnabled = enabled;
        m_memeTimer = 0.f; // Reset timer
    }
};

// Hook für PlayLayer
class $modify(PlayLayer) {
    void update(float dt) {
        PlayLayer::update(dt);
        
        // Update Meme Manager
        MemeManager::getInstance()->update(dt);
    }
    
    void onExit() {
        MemeManager::getInstance()->setEnabled(false);
        PlayLayer::onExit();
    }
    
    bool init(GJGameLevel* level) {
        if (!PlayLayer::init(level)) return false;
        
        MemeManager::getInstance()->setEnabled(true);
        return true;
    }
};

// Hook für MenuLayer
class $modify(MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;
        
        MemeManager::getInstance()->setEnabled(true);
        return true;
    }
};

// Hook für EditorUI
class $modify(EditorUI) {
    bool init(LevelEditorLayer* editor) {
        if (!EditorUI::init(editor)) return false;
        
        MemeManager::getInstance()->setEnabled(true);
        return true;
    }
};

// Einstellungs-Button für Memes
class $modify(MenuLayer) {
    void onMoreOptions(CCObject* sender) {
        // Füge Meme-Toggle zu den Optionen hinzu
        MenuLayer::onMoreOptions(sender);
        
        // Hier könnte man einen Custom Options Layer hinzufügen
        // Für jetzt einfach per Schedule
        this->scheduleOnce(schedule_selector(MenuLayer::showMemeHint), 1.0f);
    }
    
    void showMemeHint(float dt) {
        auto hint = CCLabelBMFont::create("Memes aktiv! Alle 10s ertönt ein Meme!", "chatFont.fnt");
        hint->setPosition(CCDirector::sharedDirector()->getWinSize().width / 2, 120);
        hint->setScale(0.6f);
        hint->setColor({0, 255, 255});
        hint->setZOrder(9999);
        this->addChild(hint);
        
        hint->runAction(CCSequence::create(
            CCDelayTime::create(3.0f),
            CCFadeOut::create(1.0f),
            CCRemoveSelf::create(),
            nullptr
        ));
    }
};
