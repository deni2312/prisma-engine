# include "Components/CloudComponent.h"

#include "GlobalData/PrismaFunc.h"
#include "Helpers/PrismaRender.h"
#include "Pipelines/PipelineHandler.h"
#include <Graphics/GraphicsTools/interface/MapHelper.hpp>
#include "Postprocess/Postprocess.h"

Prisma::CloudComponent::CloudComponent() { name("CloudComponent"); }

void Prisma::CloudComponent::ui() { Prisma::Component::ui(); }

void Prisma::CloudComponent::update() {

}

void Prisma::CloudComponent::start() {
    Prisma::Component::start();
    m_clouds = std::make_shared<CloudPostprocess>();
    Prisma::Postprocess::getInstance().addPostProcess(m_clouds);
}

void Prisma::CloudComponent::destroy() { 
    Prisma::Postprocess::getInstance().removePostProcess(m_clouds);
    Component::destroy(); 
}

