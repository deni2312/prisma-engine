#include "Handlers/ComponentsHandler.h"

void Prisma::ComponentsHandler::updateStart() {
    for (const auto& component : m_start) {
        if (component && !component->isStart()) {
            component->start();
        }
    }
    m_start.clear();
}

void Prisma::ComponentsHandler::updateUi() {
    for (const auto& component : m_ui) {
        if (component && !component->isUi()) {
            component->ui();
            component->isUi(true);
        }
    }
    m_ui.clear();
}

void Prisma::ComponentsHandler::updateComponents() {
    for (const auto& component : m_components) {
        if (component && component->isStart()) {
            component->update();
        }
    }
}

void Prisma::ComponentsHandler::updatePreRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {
    for (const auto& component : m_renderComponents) {
        if (component && component->isStart()) {
            component->updatePreRender(texture, depth);
        }
    }
}

void Prisma::ComponentsHandler::updatePostRender(Diligent::RefCntAutoPtr<Diligent::ITexture> texture, Diligent::RefCntAutoPtr<Diligent::ITexture> depth) {
    for (const auto& component : m_renderComponents) {
        if (component && component->isStart()) {
            component->updatePostRender(texture, depth);
        }
    }
}

//
//void Prisma::ComponentsHandler::updateRender(std::shared_ptr<FBO> fbo)
//{
//	for (const auto& component : m_components)
//	{
//		if (component && component->isStart())
//		{
//			component->updateRender(fbo);
//		}
//	}
//}
//
//void Prisma::ComponentsHandler::updatePreRender(std::shared_ptr<FBO> fbo)
//{
//	for (const auto& component : m_components)
//	{
//		if (component && component->isStart())
//		{
//			component->updatePreRender(fbo);
//		}
//	}
//}
//
//void Prisma::ComponentsHandler::updatePostRender(std::shared_ptr<FBO> fbo)
//{
//	for (const auto& component : m_components)
//	{
//		if (component && component->isStart())
//		{
//			component->updatePostRender(fbo);
//		}
//	}
//}

void Prisma::ComponentsHandler::addComponent(std::shared_ptr<Component> component) {
    m_components.push_back(component);
    m_start.push_back(component);
    m_ui.push_back(component);
    auto renderComponent = std::dynamic_pointer_cast<RenderComponent>(component);
    if (renderComponent) {
        m_renderComponents.push_back(renderComponent);
    }
}

void Prisma::ComponentsHandler::removeComponent(std::shared_ptr<Component> component) {
    // Find and remove the component from the vector
    auto it = std::find(m_components.begin(), m_components.end(), component);
    auto itStart = std::find(m_start.begin(), m_start.end(), component);
    auto itUi = std::find(m_ui.begin(), m_ui.end(), component);
    auto renderComponent = std::dynamic_pointer_cast<RenderComponent>(component);

    // Erase the removed elements (if any)
    if (it != m_components.end()) {
        m_components.erase(it);
    }

    // Erase the removed elements (if any)
    if (itStart != m_start.end()) {
        m_start.erase(itStart);
    }

    // Erase the removed elements (if any)
    if (itUi != m_ui.end()) {
        m_ui.erase(itUi);
    }

    if (renderComponent) {
        auto itRender = std::find(m_renderComponents.begin(), m_renderComponents.end(), renderComponent);
        // Erase the removed elements (if any)
        if (itRender != m_renderComponents.end()) {
            m_renderComponents.erase(itRender);
        }
    }
}

Prisma::ComponentsHandler::ComponentsHandler() {
    m_registerData.init();
}