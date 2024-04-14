#include "../../include/Handlers/ComponentsHandler.h"

void Prisma::ComponentsHandler::updateStart() {
    auto meshes=currentGlobalScene->meshes;
    for(const auto& mesh: meshes){
        auto components=mesh->components();
        for(const auto& component: components){
            if(!component->isStart()){
                component->start();
                component->isStart(true);
            }
        }
    }
}

void Prisma::ComponentsHandler::updateComponents() {
    auto meshes=currentGlobalScene->meshes;
    for(const auto& mesh: meshes){
        auto components=mesh->components();
        for(const auto& component: components){
            if(component->isStart()) {
                component->update();
            }
        }
    }
}
