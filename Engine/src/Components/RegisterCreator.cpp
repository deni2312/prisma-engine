#include "Components/RegisterCreator.h"
#include "Components/PhysicsMeshComponent.h"
#include "Components/CloudComponent.h"
#include "Components/CullingComponent.h"
#include "Components/DecalComponent.h"


void Prisma::RegisterData::init() {
    Registrar<PhysicsMeshComponent> physicsComponet("Physics");
    Registrar<MaterialComponent> materialComponent("Material");
    Registrar<CloudComponent> cloudComponent("Cloud");
    Registrar<CullingComponent> cullingComponent("Culling");
    Registrar<DecalComponent> decalComponent("Decal");
}