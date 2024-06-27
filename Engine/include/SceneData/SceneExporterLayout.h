#pragma once

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../SceneObjects/Node.h"
#include "../SceneObjects/AnimatedMesh.h"

namespace Prisma {

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
                {"t", transformData}
            };
        }

        // Deserialize Transform from JSON
        friend void from_json(const json& j, Transform& t) {
            std::vector<float> transformData;
            j.at("t").get_to(transformData);
            t.transform = glm::make_mat4(transformData.data());
        }
    };

    void to_json(json& j, std::shared_ptr<Prisma::Node> n) {
            Transform t;
            t.transform = n->matrix();
            j = json{
                {"name", n->name()},
                {"t", t},
                {"c",n->children()}
            };
            j["type"] = "NODE";
            std::vector<std::pair<std::string, std::string>> textures;

            if (std::dynamic_pointer_cast<AnimatedMesh>(n)) {
            }
            else if (std::dynamic_pointer_cast<Mesh>(n)) {
                auto mesh = std::dynamic_pointer_cast<Mesh>(n);

                j["textures"] = textures;
                // Convert Vertex properties to arrays of floats
                std::vector<json> verticesJson;
                for (const auto& vertex : mesh->verticesData().vertices) {
                    verticesJson.push_back({
                        {"p", {vertex.position.x, vertex.position.y, vertex.position.z}},
                        {"n", {vertex.normal.x, vertex.normal.y, vertex.normal.z}},
                        {"texCoords", {vertex.texCoords.x, vertex.texCoords.y}},
                        {"ta", {vertex.tangent.x, vertex.tangent.y, vertex.tangent.z}},
                        {"bi", {vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z}}
                        });
                }
                j["type"] = "MESH";
                j["vertices"] = verticesJson;
                j["faces"] = mesh->verticesData().indices;
            }
    }
    // Deserialize NodeExport from JSON
    void from_json(json& j, std::shared_ptr<Prisma::Node> n) {
        std::string name;
        Transform t;
        std::string type;
        j.at("name").get_to(name);
        j.at("t").get_to(t);
        j.at("type").get_to(type);
        n->name(name);
        n->matrix(t.transform);
        std::vector<json> childrenJson;
        j.at("c").get_to(childrenJson);
        for (json& childJson : childrenJson) {
            auto child = std::make_shared<Prisma::Node>();
            if (childJson["type"] == "MESH") {
                child = std::make_shared<Prisma::Mesh>();
            }
            from_json(childJson, child);
            n->addChild(child,false);
        }
        if (type=="MESH") {
            auto mesh = std::dynamic_pointer_cast<Mesh>(n);
            // Deserialize textures
            if (j.contains("textures")) {
                auto texturesJson = j.at("textures").get<std::vector<std::pair<std::string, std::string>>>();
            }

            // Convert arrays of floats back to Vertex properties
            auto verticesJson = j.at("vertices").get<std::vector<json>>();
            std::vector<Prisma::Mesh::Vertex> vertices;
            vertices.resize(verticesJson.size());
            for (size_t i = 0; i < verticesJson.size(); ++i) {
                auto& vertexJson = verticesJson[i];
                vertices[i].position = glm::vec3(vertexJson.at("p").get<std::vector<float>>().at(0),
                    vertexJson.at("p").get<std::vector<float>>().at(1),
                    vertexJson.at("p").get<std::vector<float>>().at(2));
                vertices[i].normal = glm::vec3(vertexJson.at("n").get<std::vector<float>>().at(0),
                    vertexJson.at("n").get<std::vector<float>>().at(1),
                    vertexJson.at("n").get<std::vector<float>>().at(2));
                vertices[i].texCoords = glm::vec2(vertexJson.at("texCoords").get<std::vector<float>>().at(0),
                    vertexJson.at("texCoords").get<std::vector<float>>().at(1));
                vertices[i].tangent = glm::vec3(vertexJson.at("ta").get<std::vector<float>>().at(0),
                    vertexJson.at("ta").get<std::vector<float>>().at(1),
                    vertexJson.at("ta").get<std::vector<float>>().at(2));
                vertices[i].bitangent = glm::vec3(vertexJson.at("bi").get<std::vector<float>>().at(0),
                    vertexJson.at("bi").get<std::vector<float>>().at(1),
                    vertexJson.at("bi").get<std::vector<float>>().at(2));
            }

            auto verticesData = std::make_shared<Prisma::Mesh::VerticesData>();
            verticesData->vertices = vertices;
            verticesData->indices = j.at("faces").get<std::vector<unsigned int>>();

            mesh->loadModel(verticesData);
        }
        else {

        }
    }

}