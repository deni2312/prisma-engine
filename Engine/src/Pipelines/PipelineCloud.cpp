#include "../../include/Pipelines/PipelineCloud.h"

static std::shared_ptr<Prisma::Shader> shader = nullptr;

Prisma::PipelineCloud::PipelineCloud()
{
	if (!shader) {
		shader = std::make_shared<Shader>("../../../Engine/Shaders/CloudPipeline/vertex.glsl", "../../../Engine/Shaders/CloudPipeline/fragment.glsl");
	}
}

void Prisma::PipelineCloud::render()
{
}
