#include "Components/ParticleComponent.h"
#include "GlobalData/GlobalShaderNames.h"
#include "GlobalData/PrismaFunc.h"
#include "Handlers/LightHandler.h"
#include "Pipelines/PipelineDiffuseIrradiance.h"
#include "Pipelines/PipelineHandler.h"
#include "Pipelines/PipelineLUT.h"
#include "Pipelines/PipelinePrefilter.h"
#include "helpers/StringHelper.h"

using namespace Diligent;

Prisma::ParticleComponent::ParticleComponent() : Component{} { name("ParticleComponent"); }

void Prisma::ParticleComponent::start() {
    Component::start();
    auto spriteFire = std::make_shared<Prisma::Texture>();
    spriteFire->loadTexture({"../../../Resources/DefaultScene/sprites/fire_sequence.png", true});

    m_sprite = std::make_shared<Prisma::Sprite>(Prisma::Sprite::BLENDING::ALPHA, Prisma::Sprite::DEPTH_WRITE::FALSE);


    m_width = 15;
    m_height = 4;
    m_speed = 20;
    m_size = glm::vec2(0.1f, 0.1f);

    m_sprite->loadSprites({spriteFire});
    m_sprite->numSprites(1, {m_width, m_height, m_speed, glm::vec3(1)});
    m_sprite->size(m_size);
    m_sprite->name("Sprite");
    /*m_compute = std::make_shared<Prisma::Shader>("../../../UserEngine/Shaders/SpriteCompute/compute.glsl");
    m_compute->use();
    m_deltaPos = m_compute->getUniformPosition("deltaTime");
    m_timePos = m_compute->getUniformPosition("time");*/
    parent()->addChild(m_sprite);
}

void Prisma::ParticleComponent::destroy()
{
    Component::destroy();
}

void Prisma::ParticleComponent::ui() { 
    Component::ui(); 


    m_updateUI = [&]() {
        if (!isStart()) {
            start();
        }

    };

    m_loadTexture = [&]() {
        if (!isStart()) {
            start();
        }

        auto openFolder = WindowsHelper::getInstance().openFolder("All Files");
        if (!openFolder.empty() && Prisma::StringHelper::getInstance().endsWith(openFolder,"png")) {
                auto sprite = std::make_shared<Prisma::Texture>();
                sprite->loadTexture({openFolder, true});
                m_sprite->loadSprites({sprite});
            }
        };

    m_apply = [&]() {
        if (!isStart()) {
            start();
        }

        m_sprite->numSprites(1, {m_width, m_height, m_speed, glm::vec3(1)});
        m_sprite->size(m_size);
    };

    ComponentType componentSize;
    componentSize = std::make_tuple(TYPES::VEC2, "Size", &m_size);

    ComponentType componentWidth;
    componentWidth = std::make_tuple(TYPES::INT, "Width", &m_width);

    ComponentType componentHeight;
    componentHeight = std::make_tuple(TYPES::INT, "Height", &m_height);

    ComponentType componentSpeed;
    componentSpeed = std::make_tuple(TYPES::INT, "Speed", &m_speed);

    ComponentType componentApply;
    componentApply = std::make_tuple(TYPES::BUTTON, "Apply", &m_apply);

    ComponentType componentTexture;
    componentTexture = std::make_tuple(TYPES::BUTTON, "Load Texture", &m_loadTexture);
    
    ComponentType componentButton;
    componentButton = std::make_tuple(TYPES::BUTTON, "Update UI", &m_updateUI);
    
    addGlobal({componentSize, false});
    addGlobal({componentWidth, false});
    addGlobal({componentHeight, false});
    addGlobal({componentSpeed, false});
    addGlobal({componentApply, false});
    addGlobal({componentTexture, false});
    addGlobal({componentButton, false});

}