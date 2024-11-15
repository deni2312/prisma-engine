#include "../../include/Components/RegisterCreator.h"
#include "../../include/Components/PhysicsMeshComponent.h"
#include "../../include/Components/CloudComponent.h"
#include "../../include/Components/CullingComponent.h"


void Prisma::RegisterData::init()
{
	Registrar<PhysicsMeshComponent> physicsComponet("Physics");
	Registrar<MaterialComponent> materialComponent("Material");
	Registrar<CloudComponent> cloudComponent("Cloud");
	Registrar<CullingComponent> cullingComponent("Culling");
}
