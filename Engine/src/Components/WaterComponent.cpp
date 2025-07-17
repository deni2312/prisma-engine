#include "Components/WaterComponent.h"

#include <GlobalData/GlobalShaderNames.h>
#include <Helpers/NoiseGenerator.h>

#include <Graphics/GraphicsTools/interface/MapHelper.hpp>

#include "GlobalData/PrismaFunc.h"
#include "Helpers/Logger.h"
#include "Helpers/PrismaRender.h"
#include "Helpers/SettingsLoader.h"
#include "Pipelines/PipelineHandler.h"
#include "Postprocess/Postprocess.h"
#include "TextureLoader/interface/TextureLoader.h"
#include "TextureLoader/interface/TextureUtilities.h"

Prisma::WaterComponent::WaterComponent() { name("WaterComponent"); }

void Prisma::WaterComponent::ui() {
    Prisma::Component::ui();

}

void Prisma::WaterComponent::update() {  }

void Prisma::WaterComponent::start() {
    Component::start();
}

void Prisma::WaterComponent::destroy() { Component::destroy(); }

void Prisma::WaterComponent::updatePreRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {}

void Prisma::WaterComponent::updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {}

void Prisma::WaterComponent::updateTransparentRender(Diligent::RefCntAutoPtr<Diligent::ITexture> accum, Diligent::RefCntAutoPtr<Diligent::ITexture> reveal, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {
}
