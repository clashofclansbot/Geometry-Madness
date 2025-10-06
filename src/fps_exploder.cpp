#pragma once
#include <Geode/modify/PlayLayer.hpp>

using namespace geode::prelude;

class $modify(PlayLayer) {
    struct Fields {
        float m_fpsTimer = 0.f;
        bool m_shouldExplode = false;
        CCLabelBMFont* m_fpsLabel = nullptr;
    };
    
    bool init(GJGameLevel* level) {
        if (!PlayLayer::init(level)) return false;
        
        // FPS Label finden oder erstellen
        auto director = CCDirector::sharedDirector();
        m_fields->m_fpsLabel = static_cast<CCLabelBMFont*>(
            director->getRunningScene()->getChildByTag(999)
        );
        
        if (!m_fields->m_fpsLabel) {
            m_fields->m_fpsLabel = CCLabelBMFont::create("0", "bigFont.fnt");
            m_fields->m_fpsLabel->setTag(999);
            m_fields->m_fpsLabel->setPosition(50, 280);
            m_fields->m_fpsLabel->setScale(0.7f);
            director->getRunningScene()->addChild(m_fields->m_fpsLabel);
        }
        
        return true;
    }
    
    void update(float dt) {
        PlayLayer::update(dt);
        
        m_fields->m_fpsTimer += dt;
        if (m_fields->m_fpsTimer >= 0.1f) {
            m_fields->m_fpsTimer = 0.f;
            
            // FPS abrufen
            auto director = CCDirector::sharedDirector();
            float fps = director->getFrameRate();
            
            // FPS künstlich erhöhen für den Effekt
            float boostedFPS = fps * 100.f + 5000.f;
            
            // Farbe basierend auf FPS (grün → gelb → rot)
            float ratio = boostedFPS / 10000.f;
            ccColor3B color;
            
            if (ratio < 0.5f) {
                // Grün zu Gelb
                color = {static_cast<GLubyte>(255 * (ratio * 2)), 255, 0};
            } else {
                // Gelb zu Rot
                color = {255, static_cast<GLubyte>(255 * (1.f - (ratio - 0.5f) * 2)), 0};
            }
            
            // Label aktualisieren
            m_fields->m_fpsLabel->setString(std::to_string(static_cast<int>(boostedFPS)).c_str());
            m_fields->m_fpsLabel->setColor(color);
            
            // Vibrationseffekt bei hohen FPS
            if (boostedFPS > 7000.f) {
                float shake = (boostedFPS - 7000.f) / 3000.f * 5.f;
                m_fields->m_fpsLabel->setPosition(
                    50 + (rand() % static_cast<int>(shake * 2 + 1) - shake),
                    280 + (rand() % static_cast<int>(shake * 2 + 1) - shake)
                );
            }
            
            // Explosion bei 10000 FPS
            if (boostedFPS >= 10000.f && !m_fields->m_shouldExplode) {
                m_fields->m_shouldExplode = true;
                triggerExplosion();
            }
        }
    }
    
    void triggerExplosion() {
        // Explosionspartikel
        auto explosion = CCParticleExplosion::create();
        explosion->setPosition(m_fields->m_fpsLabel->getPosition());
        explosion->setScale(0.5f);
        explosion->setLife(1.0f);
        explosion->setLifeVar(0.5f);
        explosion->setStartColor({255, 0, 0, 255});
        explosion->setEndColor({255, 255, 0, 0});
        CCDirector::sharedDirector()->getRunningScene()->addChild(explosion);
        
        // Bildschirm flackern
        auto flash = CCLayerColor::create({255, 0, 0, 100});
        CCDirector::sharedDirector()->getRunningScene()->addChild(flash, 999);
        
        flash->runAction(CCSequence::create(
            CCFadeTo::create(0.1f, 50),
            CCFadeTo::create(0.2f, 0),
            CCRemoveSelf::create(),
            nullptr
        ));
        
        // Label verschwinden lassen
        m_fields->m_fpsLabel->runAction(CCSequence::create(
            CCScaleTo::create(0.2f, 2.0f),
            CCFadeOut::create(0.1f),
            CCRemoveSelf::create(),
            nullptr
        ));
        
        // Soundeffekt (falls verfügbar)
        GameSoundManager::sharedManager()->playEffect("explosion.ogg");
    }
};
