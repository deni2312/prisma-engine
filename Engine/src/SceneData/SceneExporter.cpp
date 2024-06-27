#include "../../include/SceneData/SceneExporter.h"
#include "../../include/GlobalData/GlobalData.h"
#include <assimp/Exporter.hpp>
#include "../../../vcpkg_installed/x64-windows/include/assimp/DefaultLogger.hpp"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using json = nlohmann::json;

// Define a Transform structure
struct Transform {
    glm::mat4 transform;

    // Serialize Transform to JSON
    friend void to_json(json& j, const Transform& t) {
        std::vector<float> transformData(16);
        const float* matrix = glm::value_ptr(t.transform);
        for (int i = 0; i < 16; ++i) {
            transformData[i] = matrix[i];
        }
        j = json{
            {"transform", transformData}
        };
    }

    // Deserialize Transform from JSON
    friend void from_json(const json& j, Transform& t) {
        std::vector<float> transformData;
        j.at("transform").get_to(transformData);
        t.transform = glm::make_mat4(transformData.data());
    }
};

// Define a Node structure
struct NodeExport {
    std::string name;
    Transform transform;
    std::vector<std::shared_ptr<NodeExport>> children;

    // Serialize NodeExport to JSON
    friend void to_json(json& j, const NodeExport& n) {

        std::vector<NodeExport> nodes;

        for (auto node : n.children) {
            nodes.push_back(*node);
        }

        j = json{
            {"name", n.name},
            {"transform", n.transform},
            {"children", nodes}
        };
    }

    // Deserialize NodeExport from JSON
    friend void from_json(const json& j, NodeExport& n) {
        j.at("name").get_to(n.name);
        j.at("transform").get_to(n.transform);
        std::vector<json> childrenJson;
        j.at("children").get_to(childrenJson);
        for (const auto& childJson : childrenJson) {
            auto child = std::make_shared<NodeExport>();
            from_json(childJson, *child);
            n.children.push_back(child);
        }
    }
};


std::shared_ptr<Prisma::Exporter> Prisma::Exporter::instance = nullptr;
Assimp::Importer importer;
Assimp::Exporter exporter;
std::map<unsigned int, std::shared_ptr<Prisma::MaterialComponent>> materials;


Prisma::Exporter::Exporter()
{

}

void addNodesExport(const std::shared_ptr<Prisma::Node>& sceneNode, std::shared_ptr<NodeExport> nodeNext) {
    if (!sceneNode) {
        return;
    }

    nodeNext->transform.transform = sceneNode->matrix();

    unsigned int childrenSize = sceneNode->children().size();

    for (unsigned int i = 0; i < childrenSize; i++) {
        std::shared_ptr<NodeExport> node = std::make_shared<NodeExport>();
        node->name = sceneNode->children()[i]->name();
        nodeNext->children.push_back(node);
        addNodesExport(sceneNode->children()[i], node);
    }
}
void printScene(std::shared_ptr<NodeExport> nodeNext, int depth = 0) {
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
    outFile << j.dump(4);
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