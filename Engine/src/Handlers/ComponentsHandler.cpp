#include "../../include/Handlers/ComponentsHandler.h"

void Prisma::ComponentsHandler::updateStart() {
        for(const auto& component: m_components){
            if(!component->isStart()){
                component->start();
            }
        }
}

void Prisma::ComponentsHandler::updateUi()
{
        for (const auto& component : m_components) {
            if (!component->isUi()) {
                component->ui();
                component->isUi(true);
            }
        }
}

void Prisma::ComponentsHandler::updateComponents() {
        for(const auto& component: m_components){
            if(component->isStart()) {
                component->update();
            }
        }
}

void Prisma::ComponentsHandler::updateRender(std::shared_ptr<Prisma::FBO> fbo)
{
    for (const auto& component : m_components) {
        if (component->isStart()) {
            component->updateRender(fbo);
        }
    }
}

void Prisma::ComponentsHandler::updatePreRender(std::shared_ptr<Prisma::FBO> fbo)
{
    for (const auto& component : m_components) {
        if (component->isStart()) {
            component->updatePreRender(fbo);
        }
    }
}

Prisma::ComponentsHandler::ComponentsHandler()
{
}
