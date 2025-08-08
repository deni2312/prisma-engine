#pragma once

#include "../GlobalData/GlobalData.h"
#include "../Components/Component.h"
#include "../Components/RenderComponent.h"
#include "../GlobalData/InstanceData.h"
#include "../Components/RegisterCreator.h"


namespace Prisma {
class ComponentsHandler : public InstanceData<ComponentsHandler> {
public:
    void updateStart();

    void updateUi();

    void updateComponents();

    //void updateRender(std::shared_ptr<FBO> fbo = nullptr);

    void updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth);

    void updateTransparentRender(Diligent::RefCntAutoPtr<Diligent::ITexture> accum, Diligent::RefCntAutoPtr<Diligent::ITexture> reveal, Diligent::RefCntAutoPtr<Diligent::ITexture> depth);

    void addComponent(std::shared_ptr<Component> component);

    void removeComponent(std::shared_ptr<Component> component);

    ComponentsHandler();

private:
    std::vector<std::shared_ptr<Component>> m_components;
    std::vector<std::shared_ptr<Component>> m_start;
    std::vector<std::shared_ptr<Component>> m_ui;
    std::vector<std::shared_ptr<RenderComponent>> m_renderComponents;
    RegisterData m_registerData;
};
}