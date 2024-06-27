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

    virtual void a() {}
};

// Define MeshExport structure inheriting from NodeExport
struct MeshExport : public NodeExport {
    // Additional properties for mesh export
    std::vector<Prisma::Mesh::Vertex> vertices;
    std::vector<unsigned int> faces;

    virtual void a() override {}

    // Serialize MeshExport to JSON
    friend void to_json(json& j, const MeshExport& m) {
        // First, serialize the base NodeExport properties
        json baseJson;
        to_json(baseJson, static_cast<const NodeExport&>(m));

        // Then serialize the additional mesh properties
        j = baseJson;

        // Convert Vertex properties to arrays of floats
        std::vector<json> verticesJson;
        for (const auto& vertex : m.vertices) {
            verticesJson.push_back({
                {"position", {vertex.position.x, vertex.position.y, vertex.position.z}},
                {"normal", {vertex.normal.x, vertex.normal.y, vertex.normal.z}},
                {"texCoords", {vertex.texCoords.x, vertex.texCoords.y}},
                {"tangent", {vertex.tangent.x, vertex.tangent.y, vertex.tangent.z}},
                {"bitangent", {vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z}}
                });
        }

        j["vertices"] = verticesJson;
        j["faces"] = m.faces;
    }

    // Deserialize MeshExport from JSON
    friend void from_json(const json& j, MeshExport& m) {
        // Deserialize base NodeExport properties
        from_json(j, static_cast<NodeExport&>(m));

        // Convert arrays of floats back to Vertex properties
        auto verticesJson = j.at("vertices").get<std::vector<json>>();
        m.vertices.resize(verticesJson.size());
        for (size_t i = 0; i < verticesJson.size(); ++i) {
            auto& vertexJson = verticesJson[i];
            m.vertices[i].position = glm::vec3(vertexJson.at("position").get<std::vector<float>>().at(0),
                vertexJson.at("position").get<std::vector<float>>().at(1),
                vertexJson.at("position").get<std::vector<float>>().at(2));
            m.vertices[i].normal = glm::vec3(vertexJson.at("normal").get<std::vector<float>>().at(0),
                vertexJson.at("normal").get<std::vector<float>>().at(1),
                vertexJson.at("normal").get<std::vector<float>>().at(2));
            m.vertices[i].texCoords = glm::vec2(vertexJson.at("texCoords").get<std::vector<float>>().at(0),
                vertexJson.at("texCoords").get<std::vector<float>>().at(1));
            m.vertices[i].tangent = glm::vec3(vertexJson.at("tangent").get<std::vector<float>>().at(0),
                vertexJson.at("tangent").get<std::vector<float>>().at(1),
                vertexJson.at("tangent").get<std::vector<float>>().at(2));
            m.vertices[i].bitangent = glm::vec3(vertexJson.at("bitangent").get<std::vector<float>>().at(0),
                vertexJson.at("bitangent").get<std::vector<float>>().at(1),
                vertexJson.at("bitangent").get<std::vector<float>>().at(2));
        }

        m.faces = j.at("faces").get<std::vector<unsigned int>>();
    }
};

std::shared_ptr<Prisma::Exporter> Prisma::Exporter::instance = nullptr;
Assimp::Importer importer;
Assimp::Exporter exporter;
std::map<unsigned int, std::shared_ptr<Prisma::MaterialComponent>> materials;


Prisma::Exporter::Exporter()
{

}

std::shared_ptr<MeshExport> getMesh(const std::shared_ptr<Prisma::Node>& sceneNode) {
    std::shared_ptr<MeshExport> mesh = std::make_shared<MeshExport>();
    auto currentMesh = std::dynamic_pointer_cast<Prisma::Mesh>(sceneNode);
    mesh->transform.transform = glm::mat4(1.0f);
    mesh->vertices = currentMesh->verticesData().vertices;
    mesh->faces = currentMesh->verticesData().indices;
    return mesh;
}

void addNodesExport(const std::shared_ptr<Prisma::Node>& sceneNode, std::shared_ptr<NodeExport> nodeNext) {
    if (!sceneNode) {
        return;
    }

    nodeNext->transform.transform = sceneNode->matrix();

    unsigned int childrenSize = sceneNode->children().size();

    for (unsigned int i = 0; i < childrenSize; i++) {
        std::shared_ptr<NodeExport> node = std::make_shared<NodeExport>();

        auto currentNode = sceneNode->children()[i];
        if (std::dynamic_pointer_cast<Prisma::Mesh>(currentNode) && !std::dynamic_pointer_cast<Prisma::AnimatedMesh>(currentNode)) {
            node = getMesh(currentNode);
        }

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

    //printScene(newRootNode);
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