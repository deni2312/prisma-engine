#include "../include/RegisterCreator.h"

void Prisma::RegisterData::init()
{
	Registrar<PhysicsMeshComponent> physicsComponet("Physics");
	Registrar<MaterialComponent> materialComponent("Material");
	Registrar<CloudComponent> cloudComponent("Cloud");
	Registrar<CullingComponent> cullingComponent("Culling");
}
