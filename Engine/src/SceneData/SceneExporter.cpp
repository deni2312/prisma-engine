#include "../../include/SceneData/SceneExporter.h"
#include <assimp/Exporter.hpp>
#include <iostream>

std::shared_ptr<Prisma::Exporter> Prisma::Exporter::instance = nullptr;

Prisma::Exporter::Exporter()
{

}

void Prisma::Exporter::exportScene()
{

}

Prisma::Exporter& Prisma::Exporter::getInstance()
{
    if (!instance) {
        instance = std::make_shared<Prisma::Exporter>();
    }
    return *instance;
}
