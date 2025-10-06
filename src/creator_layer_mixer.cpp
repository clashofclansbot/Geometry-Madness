#pragma once
#include <Geode/modify/EditorUI.hpp>
#include <Geode/modify/GameManager.hpp>
#include <random>

using namespace geode::prelude;

class $modify(EditorUI) {
    struct Fields {
        bool m_isMixing = false;
        float m_mixTimer = 0.f;
        float m_mixIntensity = 0.f;
        std::vector<CCNode*> m_creatorButtons;
        std::vector<CCPoint> m_originalPositions;
        CCMenu* m_creatorLayerMenu = nullptr;
    };
    
    bool init(LevelEditorLayer* editor) {
        if (!EditorUI::init(editor)) return false;
        
        // Creator Layer Menu finden
        m_fields->m_creatorLayerMenu = nullptr;
        
        // Warte kurz und dann starte den Mix-Effekt
        this->scheduleOnce(schedule_selector(EditorUI::startMixing), 0.5f);
        
        return true;
    }
    
    void startMixing(float dt) {
        m_fields->m_isMixing = true;
        m_fields->m_mixIntensity = 0.f;
        
        // Finde den Creator Layer (Button-Menu)
        findCreatorLayer();
        collectCreatorButtons();
    }
    
    void findCreatorLayer() {
        // Durchsuche Children nach dem Creator Layer Menu
        auto children = this->getChildren();
        for (auto* child : children) {
            if (auto* menu = dynamic_cast<CCMenu*>(child)) {
                // Prüfe ob es Creator-Buttons enthält
                bool hasCreatorButtons = false;
                auto menuChildren = menu->getChildren();
                for (auto* btn : menuChildren) {
                    if (auto* button = dynamic_cast<CCMenuItemSpriteExtra*>(btn)) {
                        if (button->getTag() == 1 || button->getTag() == 2 || 
                            button->getTag() == 3 || button->getTag() == 4) {
                            hasCreatorButtons = true;
                            break;
                        }
                    }
                }
                
                if (hasCreatorButtons) {
                    m_fields->m_creatorLayerMenu = menu;
                    break;
                }
            }
        }
    }
    
    void collectCreatorButtons() {
        if (!m_fields->m_creatorLayerMenu) return;
        
        m_fields->m_creatorButtons.clear();
        m_fields->m_originalPositions.clear();
        
        auto children = m_fields->m_creatorLayerMenu->getChildren();
        for (auto* child : children) {
            if (auto* button = dynamic_cast<CCMenuItemSpriteExtra*>(child)) {
                m_fields->m_creatorButtons.push_back(button);
                m_fields->m_originalPositions.push_back(button->getPosition());
            }
        }
    }
    
    void update(float dt) {
        EditorUI::update(dt);
        
        if (m_fields->m_isMixing && m_fields->m_creatorLayerMenu) {
            m_fields->m_mixTimer += dt;
            m_fields->m_mixIntensity += dt * 0.5f; // Intensität steigt langsam
            
            // Limbo-style Zittern und Wobbeln
            applyLimboEffect();
            
            // Extreme Verzögerung wie in Limbo
            if (m_fields->m_mixIntensity > 2.0f) {
                applyExtremeLag();
            }
        }
    }
    
    void applyLimboEffect() {
        if (m_fields->m_creatorButtons.empty()) return;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-m_fields->m_mixIntensity * 10.f, 
                                                   m_fields->m_mixIntensity * 10.f);
        
        // Bewege alle Buttons zufällig
        for (size_t i = 0; i < m_fields->m_creatorButtons.size(); i++) {
            if (auto* button = dynamic_cast<CCMenuItemSpriteExtra*>(m_fields->m_creatorButtons[i])) {
                float offsetX = dis(gen);
                float offsetY = dis(gen);
                
                // Originalposition + zufälliger Offset
                CCPoint newPos = m_fields->m_originalPositions[i] + CCPoint(offsetX, offsetY);
                button->setPosition(newPos);
                
                // Rotation hinzufügen bei hoher Intensität
                if (m_fields->m_mixIntensity > 1.0f) {
                    float rotation = dis(gen) * 0.5f;
                    button->setRotation(rotation);
                }
                
                // Skalierungseffekt
                float scale = 1.0f + sinf(m_fields->m_mixTimer * 10.f + i) * 0.1f;
                button->setScale(scale);
            }
        }
    }
    
    void applyExtremeLag() {
        // Simuliere extreme Verzögerung wie in Limbo
        if (rand() % 100 < 30) { // 30% Chance pro Frame zu laggen
            // Temporär Buttons deaktivieren
            for (auto* btn : m_fields->m_creatorButtons) {
                if (auto* button = dynamic_cast<CCMenuItemSpriteExtra*>(btn)) {
                    button->setEnabled(false);
                }
            }
            
            // Kurz verzögert wieder aktivieren
            this->scheduleOnce([this](float) {
                for (auto* btn : m_fields->m_creatorButtons) {
                    if (auto* button = dynamic_cast<CCMenuItemSpriteExtra*>(btn)) {
                        button->setEnabled(true);
                    }
                }
            }, 0.1f, "reenable_buttons");
        }
        
        // Farbwechsel-Effekt wie in Limbo
        if (rand() % 100 < 20) {
            for (auto* btn : m_fields->m_creatorButtons) {
                if (auto* button = dynamic_cast<CCMenuItemSpriteExtra*>(btn)) {
                    ccColor3B randomColor = {
                        static_cast<GLubyte>(rand() % 256),
                        static_cast<GLubyte>(rand() % 256), 
                        static_cast<GLubyte>(rand() % 256)
                    };
                    button->setColor(randomColor);
                }
            }
        }
    }
    
    // Reset beim Verlassen
    void onStopPlaytest(CCObject* sender) {
        resetCreatorLayer();
        EditorUI::onStopPlaytest(sender);
    }
    
    void resetCreatorLayer() {
        m_fields->m_isMixing = false;
        
        // Buttons zurücksetzen
        for (size_t i = 0; i < m_fields->m_creatorButtons.size(); i++) {
            if (auto* button = dynamic_cast<CCMenuItemSpriteExtra*>(m_fields->m_creatorButtons[i])) {
                button->setPosition(m_fields->m_originalPositions[i]);
                button->setRotation(0);
                button->setScale(1.0f);
                button->setColor({255, 255, 255});
                button->setEnabled(true);
            }
        }
    }
};
