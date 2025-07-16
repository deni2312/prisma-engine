#include "Components/RegisterCreator.h"
#include "Components/PhysicsMeshComponent.h"
#include "Components/CloudComponent.h"
#include "Components/DecalComponent.h"


void Prisma::RegisterData::init() {
    Registrar<PhysicsMeshComponent> physicsComponet("Physics");
    Registrar<MaterialComponent> materialComponent("Material");
    Registrar<CloudComponent> cloudComponent("Cloud");
    Registrar<DecalComponent> decalComponent("Decal");
}