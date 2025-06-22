#include "../include/TreeRendererComponent.h"

Prisma::TreeRendererComponent::TreeRendererComponent() : RenderComponent{} { name("Tree"); }

void Prisma::TreeRendererComponent::start() { Component::start(); }

void Prisma::TreeRendererComponent::ui() { Component::ui(); }