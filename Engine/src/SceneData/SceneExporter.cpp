#include "../../include/SceneData/SceneExporter.h"
#include "../../include/GlobalData/GlobalData.h"
#include <assimp/Exporter.hpp>
#include "../../../vcpkg_installed/x64-windows/include/assimp/DefaultLogger.hpp"

std::shared_ptr<Prisma::Exporter> Prisma::Exporter::instance = nullptr;
Assimp::Importer importer;
Assimp::Exporter exporter;
std::map<unsigned int, std::shared_ptr<Prisma::MaterialComponent>> materials;


Prisma::Exporter::Exporter()
{

}

std::string Prisma::Exporter::getFileName(const std::string& filePath) {
    size_t pos = filePath.find_last_of("/\\");
    if (pos != std::string::npos) {
        return filePath.substr(pos + 1);
    }
    return filePath;
}

void Prisma::Exporter::addNodesExport(const std::shared_ptr<Prisma::Node>& sceneNode, std::shared_ptr<NodeExport> nodeNext) {
    if (!sceneNode) {
        return;
    }

    nodeNext->transform.transform = sceneNode->matrix();

    unsigned int childrenSize = sceneNode->children().size();

    for (unsigned int i = 0; i < childrenSize; i++) {
        std::shared_ptr<NodeExport> node = std::make_shared<NodeExport>();
        node->type = ExportTypes::NODE;
        auto currentNode = sceneNode->children()[i];
        if (std::dynamic_pointer_cast<Prisma::Mesh>(currentNode) && !std::dynamic_pointer_cast<Prisma::AnimatedMesh>(currentNode)) {
            node->type = ExportTypes::MESH;
            auto currentMesh = std::dynamic_pointer_cast<Prisma::Mesh>(currentNode);
            node->transform.transform = glm::mat4(1.0f);
            node->vertices = currentMesh->verticesData().vertices;
            node->faces = currentMesh->verticesData().indices;
            if (currentMesh->material()->diffuse().size() > 0) {
                std::string textureName = getFileName(currentMesh->material()->diffuse()[0].name());
                node->textures.push_back({ "DIFFUSE", textureName });
            }

            // Add the normal texture property
            if (currentMesh->material()->normal().size() > 0) {
                std::string textureName = getFileName(currentMesh->material()->normal()[0].name());
                node->textures.push_back({ "NORMAL", textureName });
            }

            // Add the roughness/metalness texture property
            if (currentMesh->material()->roughness_metalness().size() > 0) {
                std::string textureName = getFileName(currentMesh->material()->roughness_metalness()[0].name());
                node->textures.push_back({ "ROUGHNESS_METALNESS", textureName });
            }
        }

        node->name = sceneNode->children()[i]->name();
        nodeNext->children.push_back(node);
        addNodesExport(sceneNode->children()[i], node);
    }
}

void printScene(std::shared_ptr<Prisma::NodeExport> nodeNext, int depth = 0) {
    if (!nodeNext) {
        return;
    }

    // Print the current node's name with indentation based on the depth
    for (int i = 0; i < depth; ++i) {
        std::cout << " "; // Print a tab for each level of depth
    }
    std::cout << nodeNext->name << std::endl;

    // Recursively print each child node, increasing the depth
    unsigned int childrenSize = nodeNext->children.size();
    for (unsigned int i = 0; i < childrenSize; i++) {
        printScene(nodeNext->children[i], depth + 1);
    }
}

void Prisma::Exporter::exportScene()
{
    if (!currentGlobalScene || !currentGlobalScene->root) {
        std::cerr << "Error: No scene data available to export." << std::endl;
        return;
    }

    std::shared_ptr<NodeExport> root = std::make_shared<NodeExport>();
    root->name = currentGlobalScene->root->name();

    addNodesExport(currentGlobalScene->root,root);
    // Serialize rootNode to JSON
    json j = *root;

    // Write JSON to file
    std::ofstream outFile("output.prisma");
    outFile << j.dump();
    outFile.close();


    // Read JSON from file
    std::ifstream inFile("output.prisma");
    json jIn;
    inFile >> jIn;

    auto newRootNode = std::make_shared<NodeExport>();
    from_json(jIn, *newRootNode);

    printScene(newRootNode);
}

Prisma::Exporter& Prisma::Exporter::getInstance()
{
    if (!instance) {
        instance = std::make_shared<Prisma::Exporter>();
    }
    return *instance;
}

void Prisma::Exporter::addNodesRecursively(const std::shared_ptr<Prisma::Node>& sceneNode) {
    if (!sceneNode) {
        return;
    }

    unsigned int childrenSize = sceneNode->children().size();

    for (unsigned int i = 0; i < childrenSize; i++) {
        addNodesRecursively(sceneNode->children()[i]);
    }
}