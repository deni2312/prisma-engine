#include "../../include/Components/Component.h"
#include <iostream>

void Prisma::Component::ui()
{
}

void Prisma::Component::start()
{
    m_start = true;
}

void Prisma::Component::update()
{
}

void Prisma::Component::updateRender(std::shared_ptr<Prisma::FBO> fbo)
{
}

void Prisma::Component::updatePreRender(std::shared_ptr<Prisma::FBO> fbo)
{
}

void Prisma::Component::destroy()
{

}

void Prisma::Component::parent(Prisma::Node *parent) {
    m_parent = parent;
}

Prisma::Node *Prisma::Component::parent() {
    return m_parent;
}

void Prisma::Component::isStart(bool start) {
    m_start=start;
}

bool Prisma::Component::isStart() const {
    return m_start;
}

void Prisma::Component::isUi(bool ui)
{
    m_ui = ui;
}

bool Prisma::Component::isUi() const
{
    return m_ui;
}

Prisma::Component::ComponentTypeVector Prisma::Component::globalVars() {
    return m_globalVars;
}

void Prisma::Component::name(std::string name) {
    m_name=name;
}

std::string Prisma::Component::name() {
    return m_name;
}

void Prisma::Component::addGlobal(Prisma::Component::ComponentType globalVar) {
    m_globalVars.push_back(globalVar);
}
