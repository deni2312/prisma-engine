#include "../../include/Handlers/ComponentsHandler.h"

void Prisma::ComponentsHandler::updateStart() {
        for(const auto& component: m_components){
            if(!component->isStart()){
                component->start();
                component->isStart(true);
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

void Prisma::ComponentsHandler::updateRender()
{
    for (const auto& component : m_components) {
        if (component->isStart()) {
            component->updateRender();
        }
    }
}

Prisma::ComponentsHandler::ComponentsHandler()
{
}
