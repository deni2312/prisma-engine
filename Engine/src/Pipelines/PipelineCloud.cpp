#include "../../include/Pipelines/PipelineCloud.h"

std::shared_ptr<Prisma::PipelineCloud> Prisma::PipelineCloud::instance = nullptr;

Prisma::PipelineCloud::PipelineCloud()
{
    m_shader = std::make_shared<Shader>("../../../Engine/Shaders/CloudPipeline/vertex.glsl", "../../../Engine/Shaders/CloudPipeline/fragment.glsl");
    Prisma::SceneLoader loader;
    auto scene = loader.loadScene("../../../Resources/Cube/cube.gltf", {true});
    m_root = scene->root;
    std::cout << m_root->children()[0]->name();
}

void Prisma::PipelineCloud::render()
{
}
