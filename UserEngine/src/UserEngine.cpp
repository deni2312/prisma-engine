#include "../include/UserEngine.h"
#include "Pipelines/PipelineSkybox.h"
#include "Components/PhysicsMeshComponent.h"
#include "SceneData/SceneLoader.h"

#include <memory>

void UserEngine::start() {
    Prisma::Engine::getInstance().getScene("../../../Resources/DefaultScene/default.prisma", {
                                               true, [&](std::shared_ptr<Prisma::Scene> scene) {
                                                   m_player = std::make_shared<PlayerController>(scene);
                                                   m_player->scene(scene);
                                                   //When no gui set the callback on run
                                                   //Prisma::Engine::getInstance().setCallback(m_player->callback());
                                                   m_init = true;
                                               }
                                           });
}

void UserEngine::update() {
    if (m_init) {
        m_player->update();
    }
}

void UserEngine::finish() {
    //
}

std::shared_ptr<Prisma::CallbackHandler> UserEngine::callbacks() {
    return m_player->callback();
}