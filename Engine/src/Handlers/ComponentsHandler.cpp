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

    auto animateMeshes = currentGlobalScene->animateMeshes;
    for (const auto& mesh : animateMeshes) {
        auto components = mesh->components();
        for (const auto& component : components) {
            if (!component->isStart()) {
                component->start();
                component->isStart(true);
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
            if (!component->isUi()) {
                component->ui();
                component->isUi(true);
            }
        }
    }

    auto animateMeshes = currentGlobalScene->animateMeshes;
    for (const auto& mesh : animateMeshes) {
        auto components = mesh->components();
        for (const auto& component : components) {
            if (!component->isUi()) {
                component->ui();
                component->isUi(true);
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

    auto animateMeshes = currentGlobalScene->animateMeshes;
    for (const auto& mesh : animateMeshes) {
        auto components = mesh->components();
        for (const auto& component : components) {
            if (component->isStart()) {
                component->update();
            }
        }
    }
}
