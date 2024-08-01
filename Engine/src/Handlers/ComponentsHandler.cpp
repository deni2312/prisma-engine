#include "../../include/Handlers/ComponentsHandler.h"

void Prisma::ComponentsHandler::updateStart() {
    auto meshes=currentGlobalScene->meshes;
    for(const auto& mesh: meshes){
        auto components=mesh->components();
        for(const auto& component: components){
            if(!component.second->isStart()){
                component.second->start();
                component.second->isStart(true);
            }
        }
    }

    auto animateMeshes = currentGlobalScene->animateMeshes;
    for (const auto& mesh : animateMeshes) {
        auto components = mesh->components();
        for (const auto& component : components) {
            if (!component.second->isStart()) {
                component.second->start();
                component.second->isStart(true);
            }
        }
    }
}

void Prisma::ComponentsHandler::updateUi()
{
    auto meshes = currentGlobalScene->meshes;
    for (const auto& mesh : meshes) {
        auto components = mesh->components();
        for (const auto& component : components) {
            if (!component.second->isUi()) {
                component.second->ui();
                component.second->isUi(true);
            }
        }
    }

    auto animateMeshes = currentGlobalScene->animateMeshes;
    for (const auto& mesh : animateMeshes) {
        auto components = mesh->components();
        for (const auto& component : components) {
            if (!component.second->isUi()) {
                component.second->ui();
                component.second->isUi(true);
            }
        }
    }
}

void Prisma::ComponentsHandler::updateComponents() {
    auto meshes=currentGlobalScene->meshes;
    for(const auto& mesh: meshes){
        auto components=mesh->components();
        for(const auto& component: components){
            if(component.second->isStart()) {
                component.second->update();
            }
        }
    }

    auto animateMeshes = currentGlobalScene->animateMeshes;
    for (const auto& mesh : animateMeshes) {
        auto components = mesh->components();
        for (const auto& component : components) {
            if (component.second->isStart()) {
                component.second->update();
            }
        }
    }
}
