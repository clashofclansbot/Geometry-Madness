#pragma once
#include <Geode/modify/CCNode.hpp>
#include <Geode/modify/CCMenu.hpp>
#include <Geode/modify/CCMenuItem.hpp>
#include <Geode/Bindings.hpp>
#include <random>
#include <unordered_set>

using namespace geode::prelude;

class GlobalLimboEffect {
public:
    static GlobalLimboEffect* getInstance() {
        static GlobalLimboEffect instance;
        return &instance;
    }
    
    bool m_isActive = false;
    float m_mixTimer = 0.f;
    float m_mixIntensity = 0.f;
    std::unordered_set<CCNode*> m_affectedNodes;
    std::unordered_map<CCNode*, CCPoint> m_originalPositions;
    std::unordered_map<CCNode*, float> m_originalRotations;
    std::unordered_map<CCNode*, float> m_originalScales;
    
    void startLimbo() {
        m_isActive = true;
        m_mixTimer = 0.f;
        m_mixIntensity = 0.f;
        m_affectedNodes.clear();
        m_originalPositions.clear();
        m_originalRotations.clear();
        m_originalScales.clear();
    }
    
    void stopLimbo() {
        m_isActive = false;
        resetAllNodes();
    }
    
    void update(float dt) {
        if (!m_isActive) return;
        
        m_mixTimer += dt;
        m_mixIntensity += dt * 0.3f;
        
        applyGlobalLimboEffect();
    }
    
    void applyGlobalLimboEffect() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(-m_mixIntensity * 8.f, m_mixIntensity * 8.f);
        
        for (auto* node : m_affectedNodes) {
            if (!node || node->getTag() == 69420) continue; // Skip Geode Button
            
            auto originalPos = m_originalPositions[node];
            float offsetX = dis(gen);
            float offsetY = dis(gen);
            
            // Positionseffekt
            node->setPosition(originalPos + CCPoint(offsetX, offsetY));
            
            // Rotationseffekt bei hoher Intensität
            if (m_mixIntensity > 0.8f) {
                float rotation = dis(gen) * 0.3f;
                node->setRotation(rotation);
            }
            
            // Skalierungseffekt
            float scale = m_originalScales[node] + sinf(m_mixTimer * 8.f + (uintptr_t)node) * 0.15f;
            node->setScale(scale);
            
            // Farbwechsel bei sehr hoher Intensität
            if (m_mixIntensity > 1.5f && rand() % 100 < 15) {
                ccColor3B randomColor = {
                    static_cast<GLubyte>(rand() % 256),
                    static_cast<GLubyte>(rand() % 256),
                    static_cast<GLubyte>(rand() % 256)
                };
                node->setColor(randomColor);
            }
        }
        
        // Extreme Verzögerung simulieren
        if (m_mixIntensity > 2.0f && rand() % 100 < 25) {
            simulateInputLag();
        }
    }
    
    void simulateInputLag() {
        // Temporär Buttons deaktivieren (außer Geode)
        for (auto* node : m_affectedNodes) {
            if (auto* button = dynamic_cast<CCMenuItem*>(node)) {
                if (node->getTag() != 69420) {
                    button->setEnabled(false);
                }
            }
        }
        
        // Nach kurzer Verzögerung wieder aktivieren
        CCDirector::sharedDirector()->getScheduler()->scheduleSelector(
            schedule_selector(GlobalLimboEffect::reenableButtons), 
            this, 0.1f, false
        );
    }
    
    void reenableButtons(float dt) {
        for (auto* node : m_affectedNodes) {
            if (auto* button = dynamic_cast<CCMenuItem*>(node)) {
                button->setEnabled(true);
            }
        }
    }
    
    void registerNode(CCNode* node) {
        if (!node || node->getTag() == 69420) return; // Keine Geode Buttons
        
        // Prüfe ob es ein Button ist
        bool isButton = dynamic_cast<CCMenuItem*>(node) != nullptr;
        bool isMenu = dynamic_cast<CCMenu*>(node) != nullptr;
        
        if (isButton || isMenu) {
            m_affectedNodes.insert(node);
            m_originalPositions[node] = node->getPosition();
            m_originalRotations[node] = node->getRotation();
            m_originalScales[node] = node->getScale();
        }
    }
    
    void unregisterNode(CCNode* node) {
        m_affectedNodes.erase(node);
        m_originalPositions.erase(node);
        m_originalRotations.erase(node);
        m_originalScales.erase(node);
    }
    
    void resetAllNodes() {
        for (auto* node : m_affectedNodes) {
            if (!node) continue;
            
            auto itPos = m_originalPositions.find(node);
            auto itRot = m_originalRotations.find(node);
            auto itScale = m_originalScales.find(node);
            
            if (itPos != m_originalPositions.end()) {
                node->setPosition(itPos->second);
            }
            if (itRot != m_originalRotations.end()) {
                node->setRotation(itRot->second);
            }
            if (itScale != m_originalScales.end()) {
                node->setScale(itScale->second);
            }
            
            node->setColor({255, 255, 255});
            
            if (auto* button = dynamic_cast<CCMenuItem*>(node)) {
                button->setEnabled(true);
            }
        }
    }
};

// Hook für CCNode um alle Nodes zu erfassen
class $modify(CCNode) {
    void onEnter() {
        CCNode::onEnter();
        
        // Registriere diesen Node beim Limbo Effect
        GlobalLimboEffect::getInstance()->registerNode(this);
    }
    
    void onExit() {
        GlobalLimboEffect::getInstance()->unregisterNode(this);
        CCNode::onExit();
    }
};

// Hook für CCMenuItem um speziell Buttons zu handlen
class $modify(CCMenuItem) {
    bool init() {
        if (!CCMenuItem::init()) return false;
        
        // Markiere Geode Buttons mit speziellem Tag
        if (m_pfnSelector == menu_selector(GeodeButtonCallback)) {
            this->setTag(69420); // Magic number für Geode Buttons
        }
        
        return true;
    }
};

// Haupt-Modifikation für den globalen Update
class $modify(CCApplication) {
    void update(double dt) {
        CCApplication::update(dt);
        
        // Update den Limbo Effect
        GlobalLimboEffect::getInstance()->update(dt);
    }
};

// Separater Mod für den FPS-Trigger
class $modify(PlayLayer) {
    struct Fields {
        bool m_hasTriggeredLimbo = false;
    };
    
    void update(float dt) {
        PlayLayer::update(dt);
        
        // Trigger Limbo bei hohen FPS
        auto director = CCDirector::sharedDirector();
        float fps = director->getFrameRate();
        float boostedFPS = fps * 100.f + 5000.f;
        
        if (boostedFPS >= 8000.f && !m_fields->m_hasTriggeredLimbo) {
            m_fields->m_hasTriggeredLimbo = true;
            GlobalLimboEffect::getInstance()->startLimbo();
        }
        
        // Stoppe Limbo wenn FPS normalisiert
        if (boostedFPS < 5000.f && m_fields->m_hasTriggeredLimbo) {
            m_fields->m_hasTriggeredLimbo = false;
            GlobalLimboEffect::getInstance()->stopLimbo();
        }
    }
    
    void onExit() {
        GlobalLimboEffect::getInstance()->stopLimbo();
        m_fields->m_hasTriggeredLimbo = false;
        PlayLayer::onExit();
    }
};

// Geode Button Schutz - Stelle sicher dass Geode UI nicht betroffen ist
class $modify(CCMenu) {
    bool init() {
        if (!CCMenu::init()) return false;
        
        // Markiere Geode Menüs
        if (this->getChildrenCount() > 0) {
            bool hasGeodeButton = false;
            auto children = this->getChildren();
            for (auto* child : children) {
                if (child->getTag() == 69420) {
                    hasGeodeButton = true;
                    break;
                }
            }
            
            if (hasGeodeButton) {
                this->setTag(69420); // Markiere gesamtes Menu als Geode
            }
        }
        
        return true;
    }
};
