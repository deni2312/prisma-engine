#include "Components/RegisterCreator.h"
#include "Components/PhysicsMeshComponent.h"
#include "Components/CloudComponent.h"
#include "Components/DecalComponent.h"
#include <Components/WaterComponent.h>


void Prisma::RegisterData::init() {
    Registrar<PhysicsMeshComponent> physicsComponent("Physics");
    Registrar<MaterialComponent> materialComponent("Material");
    Registrar<CloudComponent> cloudComponent("Cloud");
    Registrar<DecalComponent> decalComponent("Decal");
    Registrar<WaterComponent> waterComponent("Water");
}