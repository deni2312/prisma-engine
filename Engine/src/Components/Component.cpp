#include "Components/Component.h"
#include <iostream>
#include "GlobalData/GlobalData.h"

static uint64_t uuidComponent = 0;

void Prisma::Component::ui() {
}

void Prisma::Component::start() {
    m_start = true;
}

void Prisma::Component::update() {
}

//void Prisma::Component::updateRender(std::shared_ptr<FBO> fbo)
//{
//}

//void Prisma::Component::updatePreRender(std::shared_ptr<FBO> fbo)
//{
//}

//void Prisma::Component::updatePostRender(std::shared_ptr<FBO> fbo)
//{
//}

void Prisma::Component::onParent(std::shared_ptr<Node> parent) {
}

void Prisma::Component::destroy() {
    GlobalData::getInstance().sceneComponents().erase(m_uuid);
}

void Prisma::Component::parent(std::weak_ptr<Node> parent) {
    m_parent = parent;
}

std::shared_ptr<Prisma::Node> Prisma::Component::parent() {
    return m_parent.lock();
}

void Prisma::Component::isStart(bool start) {
    m_start = start;
}

bool Prisma::Component::isStart() const {
    return m_start;
}

void Prisma::Component::isUi(bool ui) {
    m_ui = ui;
}

bool Prisma::Component::isUi() const {
    return m_ui;
}

Prisma::Component::ComponentTypeVector Prisma::Component::globalVars() {
    return m_globalVars;
}

void Prisma::Component::name(std::string name) {
    m_name = name;
}

std::string Prisma::Component::name() {
    return m_name;
}

uint64_t Prisma::Component::uuid() {
    return m_uuid;
}

Prisma::Component::Component() {
    name("BaseComponent");
    m_uuid = uuidComponent;
    uuidComponent = uuidComponent + 1;
    GlobalData::getInstance().sceneComponents()[m_uuid] = this;
}

nlohmann::json Prisma::Component::serialize() {
    return nlohmann::json();
}

void Prisma::Component::deserialize(nlohmann::json& data) {
}


Prisma::Component::~Component() {
    //destroy();
}

void Prisma::Component::uiRemovable(bool uiRemovable) {
    m_uiRemovable = uiRemovable;
}

bool Prisma::Component::uiRemovable() {
    return m_uiRemovable;
}

void Prisma::Component::addGlobal(Options globalVar) {
    m_globalVars.push_back(globalVar);
}