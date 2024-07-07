#pragma once

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "../SceneObjects/Node.h"
#include "../SceneObjects/AnimatedMesh.h"
#include "../SceneObjects/Light.h"

namespace Prisma {



    using json = nlohmann::json;

    // Define a Transform structure
    struct Transform {
        glm::mat4 transform;

        // Serialize Transform to JSON
        friend void to_json(json& j, const Transform& t) {
            j = json{ {"t", std::vector<float>(glm::value_ptr(t.transform), glm::value_ptr(t.transform) + 16)} };
        }

        // Deserialize Transform from JSON
        friend void from_json(const json& j, Transform& t) {
            std::vector<float> transformData = j.at("t").get<std::vector<float>>();
            t.transform = glm::make_mat4(transformData.data());
        }
    };

    void to_json(json& j, std::shared_ptr<Prisma::Node> n) {
            Transform t;
            Transform k;
            t.transform = n->matrix();
            k.transform = n->finalMatrix();
            j = json{
                {"name", n->name()},
                {"t", t},
                {"k", k},
                {"c",n->children()}
            };
            j["type"] = "NODE";
            std::vector<std::pair<std::string, std::string>> textures;

            if (std::dynamic_pointer_cast<AnimatedMesh>(n)) {
                auto mesh = std::dynamic_pointer_cast<AnimatedMesh>(n);

                // Add the diffuse texture property
                if (mesh->material()->diffuse().size() > 0) {
                    std::string textureName = mesh->material()->diffuse()[0].name();
                    if (textureName == "") {
                        textures.push_back({ "DIFFUSE", "NO_TEXTURE" });
                    }
                    else {
                        textures.push_back({ "DIFFUSE", textureName });
                    }
                }

                // Add the normal texture property
                if (mesh->material()->normal().size() > 0) {
                    std::string textureName = mesh->material()->normal()[0].name();
                    if (textureName == "") {
                        textures.push_back({ "NORMAL", "NO_TEXTURE" });
                    }
                    else {
                        textures.push_back({ "NORMAL", textureName });
                    }
                }

                // Add the roughness/metalness texture property
                if (mesh->material()->roughness_metalness().size() > 0) {
                    std::string textureName = mesh->material()->roughness_metalness()[0].name();
                    if (textureName == "") {
                        textures.push_back({ "ROUGHNESS", "NO_TEXTURE" });
                    }
                    else {
                        textures.push_back({ "ROUGHNESS", textureName });
                    }
                }

                j["textures"] = textures;
                // Convert Vertex properties to arrays of floats
                std::vector<json> verticesJson;
                for (const auto& vertex : mesh->animateVerticesData()->vertices) {
                    verticesJson.push_back({
                        {"p", {vertex.position.x, vertex.position.y, vertex.position.z}},
                        {"boneId", {vertex.m_BoneIDs[0], vertex.m_BoneIDs[1], vertex.m_BoneIDs[2],vertex.m_BoneIDs[3]}},
                        {"weight", {vertex.m_Weights[0], vertex.m_Weights[1], vertex.m_Weights[2],vertex.m_Weights[3]}},
                        {"n", {vertex.normal.x, vertex.normal.y, vertex.normal.z}},
                        {"texCoords", {vertex.texCoords.x, vertex.texCoords.y}},
                        {"ta", {vertex.tangent.x, vertex.tangent.y, vertex.tangent.z}},
                        {"bi", {vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z}}
                        });
                }
                std::vector<std::vector<float>> data;

                for (const auto& vertex : mesh->animateVerticesData()->vertices) {

                    data.push_back({ vertex.position.x, vertex.position.y, vertex.position.z });

                    data.push_back({ vertex.normal.x, vertex.normal.y, vertex.normal.z });

                    data.push_back({ vertex.texCoords.x, vertex.texCoords.y });

                    data.push_back({ vertex.tangent.x, vertex.tangent.y, vertex.tangent.z });

                    data.push_back({ vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z });

                    data.push_back({ (float)vertex.m_BoneIDs[0], (float)vertex.m_BoneIDs[1], (float)vertex.m_BoneIDs[2],(float)vertex.m_BoneIDs[3] });

                    data.push_back({ vertex.m_Weights[0], vertex.m_Weights[1], vertex.m_Weights[2],vertex.m_Weights[3] });
                }

                j["type"] = "MESH_ANIMATE";
                j["vertices"] = data;
                j["boneCount"] = mesh->boneInfoCounter();

                int i = 0;

                for (const auto& boneData : mesh->boneInfoMap()) {
                    j["boneData"][i]["name"] = boneData.first;
                    auto data = boneData.second;
                    j["boneData"][i]["data"]["id"] = data.id;
                    Transform transform;
                    transform.transform = data.offset;
                    j["boneData"][i]["data"]["offset"] = transform;
                    i++;
                }

                j["faces"] = mesh->animateVerticesData()->indices;
            }
            else if (std::dynamic_pointer_cast<Mesh>(n)) {
                auto mesh = std::dynamic_pointer_cast<Mesh>(n);

                // Add the diffuse texture property
                if (mesh->material()->diffuse().size() > 0) {
                    std::string textureName = mesh->material()->diffuse()[0].name();
                    if (textureName == "") {
                        textures.push_back({ "DIFFUSE", "NO_TEXTURE"});
                    }
                    else {
                        textures.push_back({ "DIFFUSE", textureName });
                    }
                }

                // Add the normal texture property
                if (mesh->material()->normal().size() > 0) {
                    std::string textureName = mesh->material()->normal()[0].name();
                    if (textureName == "") {
                        textures.push_back({ "NORMAL", "NO_TEXTURE" });
                    }
                    else {
                        textures.push_back({ "NORMAL", textureName });
                    }
                }

                // Add the roughness/metalness texture property
                if (mesh->material()->roughness_metalness().size() > 0) {
                    std::string textureName = mesh->material()->roughness_metalness()[0].name();
                    if (textureName == "") {
                        textures.push_back({ "ROUGHNESS", "NO_TEXTURE" });
                    }
                    else {
                        textures.push_back({ "ROUGHNESS", textureName });
                    }
                }

                j["textures"] = textures;
                // Convert Vertex properties to arrays of floats
                std::vector<std::vector<float>> data;

                for (const auto& vertex : mesh->verticesData().vertices) {

                    data.push_back({ vertex.position.x, vertex.position.y, vertex.position.z });

                    data.push_back({ vertex.normal.x, vertex.normal.y, vertex.normal.z });

                    data.push_back({ vertex.texCoords.x, vertex.texCoords.y });

                    data.push_back({ vertex.tangent.x, vertex.tangent.y, vertex.tangent.z });

                    data.push_back({ vertex.bitangent.x, vertex.bitangent.y, vertex.bitangent.z });
                }
                j["type"] = "MESH";
                j["vertices"] = data;
                j["faces"] = mesh->verticesData().indices;
            }
            else if (std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightDir>>(n)) {
                j["type"] = "LIGHT_DIRECTIONAL";
                auto light = std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightDir>>(n);
                j["direction"] = { light->type().direction.x,light->type().direction.y,light->type().direction.z };
                j["diffuse"] = { light->type().diffuse.x,light->type().diffuse.y,light->type().diffuse.z };
                j["specular"] = { light->type().specular.x,light->type().specular.y,light->type().specular.z };
                j["padding"] = { light->type().padding.x,light->type().padding.y };
            }
            else if (std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightOmni>>(n)) {
                j["type"] = "LIGHT_OMNI";
                auto light = std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightOmni>>(n);
                j["position"] = { light->type().position.x,light->type().position.y,light->type().position.z };
                j["diffuse"] = { light->type().diffuse.x,light->type().diffuse.y,light->type().diffuse.z };
                j["specular"] = { light->type().specular.x,light->type().specular.y,light->type().specular.z };
                j["radius"] = light->type().radius;
                j["attenuation"] = { light->type().attenuation.x,light->type().attenuation.y,light->type().attenuation.z, light->type().attenuation.w };
                j["farPlane"] = light->type().farPlane.x;
            }
    }
    // Deserialize NodeExport from JSON
    void from_json(json& j, std::shared_ptr<Prisma::Node> n) {
        std::string name;
        Transform t;
        Transform k;
        std::string type;
        j.at("name").get_to(name);
        j.at("t").get_to(t);
        j.at("k").get_to(k);
        j.at("type").get_to(type);
        n->name(name);
        n->matrix(t.transform);
        n->finalMatrix(k.transform);
        std::vector<json> childrenJson;
        j.at("c").get_to(childrenJson);
        for (json& childJson : childrenJson) {
            auto child = std::make_shared<Prisma::Node>();
            if (childJson["type"] == "MESH") {
                child = std::make_shared<Prisma::Mesh>();
            }
            else if (childJson["type"] == "LIGHT_DIRECTIONAL") {
                child = std::make_shared<Prisma::Light<Prisma::LightType::LightDir>>();
            }
            else if (childJson["type"] == "LIGHT_OMNI") {
                child = std::make_shared<Prisma::Light<Prisma::LightType::LightOmni>>();
            }
            else if (childJson["type"] == "MESH_ANIMATE") {
                child = std::make_shared<Prisma::AnimatedMesh>();
            }
            child->parent(n);
            from_json(childJson, child);
            n->addChild(child,false);
        }
        if (type=="MESH") {
            auto mesh = std::dynamic_pointer_cast<Mesh>(n);
            // Deserialize textures
            if (j.contains("textures")) {
                auto texturesJson = j.at("textures").get<std::vector<std::pair<std::string, std::string>>>();
                std::shared_ptr<Prisma::MaterialComponent> material = std::make_shared<Prisma::MaterialComponent>();
                for (const auto& t : texturesJson) {
                    if (t.first == "DIFFUSE") {
                        std::vector<Prisma::Texture> textures;
                        Prisma::Texture texture;
                        if (t.second == "NO_TEXTURE") {
                            textures.push_back(defaultBlack);
                        }
                        else {
                            texture.name(t.second);
                            texture.loadTexture(t.second,true);
                            textures.push_back(texture);
                        }
                        material->diffuse(textures);
                    }
                    else if (t.first == "NORMAL") {
                        std::vector<Prisma::Texture> textures;
                        Prisma::Texture texture;
                        if (t.second == "NO_TEXTURE") {
                            textures.push_back(defaultNormal);
                        }
                        else {
                            texture.name(t.second);
                            texture.loadTexture(t.second);
                            textures.push_back(texture);
                        }
                        material->normal(textures);
                    }
                    else if (t.first == "ROUGHNESS") {
                        std::vector<Prisma::Texture> textures;
                        Prisma::Texture texture;
                        if (t.second == "NO_TEXTURE") {
                            textures.push_back(defaultBlack);
                        }
                        else {
                            texture.name(t.second);
                            texture.loadTexture(t.second);
                            textures.push_back(texture);
                        }
                        material->roughness_metalness(textures);
                    }
                }
                mesh->material(material);
            }
            // Convert arrays of floats back to Vertex properties
            auto verticesJson = j.at("vertices").get<std::vector<std::vector<float>>>();
            std::vector<Prisma::Mesh::Vertex> vertices;
            vertices.resize(verticesJson.size() / 5);
            for (size_t i = 0; i < verticesJson.size(); i = i + 5) {

                int vertexIndex = i / 5;

                vertices[vertexIndex].position = glm::vec3(verticesJson[i][0], verticesJson[i][1], verticesJson[i][2]);
                vertices[vertexIndex].normal = glm::vec3(verticesJson[i + 1][0], verticesJson[i + 1][1], verticesJson[i + 1][2]);
                vertices[vertexIndex].texCoords = glm::vec2(verticesJson[i + 2][0], verticesJson[i + 2][1]);
                vertices[vertexIndex].tangent = glm::vec3(verticesJson[i + 3][0], verticesJson[i + 3][1], verticesJson[i + 3][2]);
                vertices[vertexIndex].bitangent = glm::vec3(verticesJson[i + 4][0], verticesJson[i + 4][1], verticesJson[i + 4][2]);

            }

            auto verticesData = std::make_shared<Prisma::Mesh::VerticesData>();
            verticesData->vertices = vertices;
            verticesData->indices = j.at("faces").get<std::vector<unsigned int>>();
            mesh->loadModel(verticesData);
            mesh->computeAABB();
        }
        else if(type == "LIGHT_DIRECTIONAL"){
            auto light = std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightDir>>(n);
            Prisma::LightType::LightDir lightType;
            lightType.direction = glm::vec4(j.at("direction").get<std::vector<float>>().at(0), j.at("direction").get<std::vector<float>>().at(1), j.at("direction").get<std::vector<float>>().at(2),1.0);
            lightType.diffuse = glm::vec4(j.at("diffuse").get<std::vector<float>>().at(0), j.at("diffuse").get<std::vector<float>>().at(1), j.at("diffuse").get<std::vector<float>>().at(2), 1.0);
            lightType.specular = glm::vec4(j.at("specular").get<std::vector<float>>().at(0), j.at("specular").get<std::vector<float>>().at(1), j.at("specular").get<std::vector<float>>().at(2), 1.0);
            lightType.padding = glm::vec2(j.at("padding").get<std::vector<float>>().at(0), j.at("padding").get<std::vector<float>>().at(1));
            light->type(lightType);
            light->createShadow(MAX_SHADOW_DIR, MAX_SHADOW_DIR);
        }
        else if (type == "LIGHT_OMNI") {
            auto light = std::dynamic_pointer_cast<Prisma::Light<Prisma::LightType::LightOmni>>(n);
            Prisma::LightType::LightOmni lightType;
            lightType.position = glm::vec4(j.at("position").get<std::vector<float>>().at(0), j.at("position").get<std::vector<float>>().at(1), j.at("position").get<std::vector<float>>().at(2), 1.0);
            lightType.diffuse = glm::vec4(j.at("diffuse").get<std::vector<float>>().at(0), j.at("diffuse").get<std::vector<float>>().at(1), j.at("diffuse").get<std::vector<float>>().at(2), 1.0);
            lightType.specular = glm::vec4(j.at("specular").get<std::vector<float>>().at(0), j.at("specular").get<std::vector<float>>().at(1), j.at("specular").get<std::vector<float>>().at(2), 1.0);
            lightType.radius = j.at("radius").get<float>();
            lightType.attenuation = glm::vec4(j.at("attenuation").get<std::vector<float>>().at(0), j.at("attenuation").get<std::vector<float>>().at(1), j.at("attenuation").get<std::vector<float>>().at(2), j.at("attenuation").get<std::vector<float>>().at(3));
            lightType.farPlane.x = j.at("farPlane").get<float>();
            light->type(lightType);
            light->createShadow(MAX_SHADOW_OMNI, MAX_SHADOW_OMNI);
        }
        else if (type == "MESH_ANIMATE") {
            auto mesh = std::dynamic_pointer_cast<AnimatedMesh>(n);
            // Deserialize textures
            if (j.contains("textures")) {
                auto texturesJson = j.at("textures").get<std::vector<std::pair<std::string, std::string>>>();
                std::shared_ptr<Prisma::MaterialComponent> material = std::make_shared<Prisma::MaterialComponent>();
                for (const auto& t : texturesJson) {
                    if (t.first == "DIFFUSE") {
                        std::vector<Prisma::Texture> textures;
                        Prisma::Texture texture;
                        if (t.second == "NO_TEXTURE") {
                            textures.push_back(defaultBlack);
                        }
                        else {
                            texture.name(t.second);
                            texture.loadTexture(t.second, true);
                            textures.push_back(texture);
                        }
                        material->diffuse(textures);
                    }
                    else if (t.first == "NORMAL") {
                        std::vector<Prisma::Texture> textures;
                        Prisma::Texture texture;
                        if (t.second == "NO_TEXTURE") {
                            textures.push_back(defaultNormal);
                        }
                        else {
                            texture.name(t.second);
                            texture.loadTexture(t.second);
                            textures.push_back(texture);
                        }
                        material->normal(textures);
                    }
                    else if (t.first == "ROUGHNESS") {
                        std::vector<Prisma::Texture> textures;
                        Prisma::Texture texture;
                        if (t.second == "NO_TEXTURE") {
                            textures.push_back(defaultBlack);
                        }
                        else {
                            texture.name(t.second);
                            texture.loadTexture(t.second);
                            textures.push_back(texture);
                        }
                        material->roughness_metalness(textures);
                    }
                }
                mesh->material(material);
            }
            // Convert arrays of floats back to Vertex properties
            auto verticesJson = j.at("vertices").get<std::vector<json>>();
            

            std::vector<Prisma::AnimatedMesh::AnimateVertex> vertices;
            vertices.resize(verticesJson.size() / 7);
            for (size_t i = 0; i < verticesJson.size(); i = i + 7) {

                int vertexIndex = i / 7;

                vertices[vertexIndex].position = glm::vec3(verticesJson[i][0], verticesJson[i][1], verticesJson[i][2]);
                vertices[vertexIndex].normal = glm::vec3(verticesJson[i + 1][0], verticesJson[i + 1][1], verticesJson[i + 1][2]);
                vertices[vertexIndex].texCoords = glm::vec2(verticesJson[i + 2][0], verticesJson[i + 2][1]);
                vertices[vertexIndex].tangent = glm::vec3(verticesJson[i + 3][0], verticesJson[i + 3][1], verticesJson[i + 3][2]);
                vertices[vertexIndex].bitangent = glm::vec3(verticesJson[i + 4][0], verticesJson[i + 4][1], verticesJson[i + 4][2]);
                vertices[vertexIndex].m_BoneIDs[0] = verticesJson[i + 5][0];
                vertices[vertexIndex].m_BoneIDs[1] = verticesJson[i + 5][1];
                vertices[vertexIndex].m_BoneIDs[2] = verticesJson[i + 5][2];
                vertices[vertexIndex].m_BoneIDs[3] = verticesJson[i + 5][3];

                vertices[vertexIndex].m_Weights[0] = verticesJson[i + 6][0];
                vertices[vertexIndex].m_Weights[1] = verticesJson[i + 6][1];
                vertices[vertexIndex].m_Weights[2] = verticesJson[i + 6][2];
                vertices[vertexIndex].m_Weights[3] = verticesJson[i + 6][3];


            }

            auto verticesData = std::make_shared<Prisma::AnimatedMesh::AnimateVerticesData>();
            verticesData->vertices = vertices;
            verticesData->indices = j.at("faces").get<std::vector<unsigned int>>();
            auto& counter = mesh->boneInfoCounter();
            counter = j.at("boneCount").get<int>();

            auto boneDataJson = j.at("boneData");
            for (json::iterator it = boneDataJson.begin(); it != boneDataJson.end(); ++it) {
                std::string boneName = it.value().at("name");
                Prisma::BoneInfo data;
                data.id = it.value().at("data").at("id");
                Transform transform;
                from_json(it.value().at("data").at("offset"), transform);
                data.offset = transform.transform;

                auto& boneMap = mesh->boneInfoMap();

                boneMap[boneName] = data;
            }


            mesh->loadAnimateModel(verticesData);
            mesh->computeAABB();
        }
    }

}