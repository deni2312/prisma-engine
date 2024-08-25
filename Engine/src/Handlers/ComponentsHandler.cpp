#include "../../include/Handlers/ComponentsHandler.h"

std::shared_ptr<Prisma::ComponentsHandler> Prisma::ComponentsHandler::instance = nullptr;

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

Prisma::ComponentsHandler& Prisma::ComponentsHandler::getInstance()
{
    if (!instance) {
        instance = std::make_shared<ComponentsHandler>();
    }
    return *instance;
}

Prisma::ComponentsHandler::ComponentsHandler()
{
}
