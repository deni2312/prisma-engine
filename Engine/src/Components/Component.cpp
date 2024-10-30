#include "../../include/Components/Component.h"
#include <iostream>
#include "../../include/GlobalData/GlobalData.h"

static uint64_t uuidComponent = 0;

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

void Prisma::Component::updateRender(std::shared_ptr<FBO> fbo)
{
}

void Prisma::Component::updatePreRender(std::shared_ptr<FBO> fbo)
{
}

void Prisma::Component::destroy()
{
	sceneComponents.erase(m_uuid);
}

void Prisma::Component::parent(Node* parent)
{
	m_parent = parent;
}

Prisma::Node* Prisma::Component::parent()
{
	return m_parent;
}

void Prisma::Component::isStart(bool start)
{
	m_start = start;
}

bool Prisma::Component::isStart() const
{
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

Prisma::Component::ComponentTypeVector Prisma::Component::globalVars()
{
	return m_globalVars;
}

void Prisma::Component::name(std::string name)
{
	m_name = name;
}

std::string Prisma::Component::name()
{
	return m_name;
}

uint64_t Prisma::Component::uuid()
{
	return m_uuid;
}

Prisma::Component::Component()
{
	m_uuid = uuidComponent;
	uuidComponent = uuidComponent + 1;
	sceneComponents[m_uuid] = this;
}

Prisma::Component::~Component()
{
	//destroy();
}

void Prisma::Component::addGlobal(ComponentType globalVar)
{
	m_globalVars.push_back(globalVar);
}
