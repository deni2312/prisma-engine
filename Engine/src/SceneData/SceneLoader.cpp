#include "SceneData/SceneLoader.h"
#include "GlobalData/Defines.h"
#include <iostream>
#include <memory>
#include "glm/gtx/string_cast.hpp"
#include "Components/Component.h"
#include "Components/MaterialComponent.h"
#include "Helpers/Logger.h"
#include "Helpers/PrismaMath.h"
#include "Helpers/StringHelper.h"
#include "Pipelines/PipelineHandler.h"

std::shared_ptr<Prisma::Scene> Prisma::SceneLoader::loadScene(std::string scene, SceneParameters sceneParameters) {
    m_sceneParameters = sceneParameters;
    m_path = scene;
    // Extracting the directory to the last folder
    size_t lastSlash = scene.find_last_of("/");

    if (lastSlash != std::string::npos) {
        m_folder = scene.substr(0, lastSlash) + "/";
    } else {
        lastSlash = scene.find_last_of("\\");
        if (lastSlash != std::string::npos) {
            m_folder = scene.substr(0, lastSlash) + "\\";
        } else {
            // Handle the case where there is no directory
            m_folder = "";
        }
    }

    auto prismaScene = StringHelper::getInstance().endsWith(scene, ".prisma");
    m_scene = std::make_shared<Scene>();
    m_scene->name = scene;
    Exporter exporter;

    if (prismaScene) {
        auto newRootNode = exporter.importScene(scene);
        newRootNode->parent(nullptr);
        m_scene->root = newRootNode;

        NodeHelper nodeHelper;

        nodeHelper.nodeIterator(m_scene->root, [&](auto node, auto parent) {
            auto isAnimateMesh = std::dynamic_pointer_cast<AnimatedMesh>(node);
            auto isMesh = std::dynamic_pointer_cast<Mesh>(node);
            auto isLightDir = std::dynamic_pointer_cast<Light<LightType::LightDir>>(node);
            auto isLightOmni = std::dynamic_pointer_cast<Light<LightType::LightOmni>>(node);
            auto isLightArea = std::dynamic_pointer_cast<Light<LightType::LightArea>>(node);
            if (isAnimateMesh) {
                m_scene->animateMeshes.push_back(isAnimateMesh);
            } else if (isMesh) {
                m_scene->meshes.push_back(isMesh);
            } else if (isLightDir) {
                m_scene->dirLights.push_back(isLightDir);
            } else if (isLightOmni) {
                m_scene->omniLights.push_back(isLightOmni);
            } else if (isLightArea) {
                m_scene->areaLights.push_back(isLightArea);
            }
        });

        return m_scene;
    }
    Assimp::Importer importer;
    m_aScene = importer.ReadFile(
        scene,
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
        aiProcess_TransformUVCoords);
    if (m_aScene) {
        m_scene = std::make_shared<Scene>();
        auto root = std::make_shared<Node>();
        root->name(m_aScene->mRootNode->mName.C_Str());
        glm::mat4 transform = getTransform(m_aScene->mRootNode->mTransformation);
        root->matrix(transform, false);
        root->finalMatrix(transform, false);
        root->parent(nullptr);
        m_scene->root = root;
        nodeIteration(root, m_aScene->mRootNode, m_aScene);
        loadLights(m_aScene, root);
        exporter.postLoad(root, false);
        return m_scene;
    }
    CacheScene::getInstance().updateAllCaches();
    std::cerr << "Could not find the directory" << std::endl;
    return nullptr;
}

void Prisma::SceneLoader::loadSceneAsync(std::string scene, SceneParameters sceneParameters) {
    m_sceneParameters = sceneParameters;
    // Extracting the directory to the last folder
    size_t lastSlash = scene.find_last_of("/");

    if (lastSlash != std::string::npos) {
        m_folder = scene.substr(0, lastSlash) + "/";
    } else {
        lastSlash = scene.find_last_of("\\");
        if (lastSlash != std::string::npos) {
            m_folder = scene.substr(0, lastSlash) + "\\";
        } else {
            // Handle the case where there is no directory
            m_folder = "";
        }
    }

    auto endsWith = [](const std::string& value, const std::string& ending) {
        if (ending.size() > value.size()) return false;
        return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
    };

    auto prismaScene = endsWith(scene, ".prisma");
    m_scene = std::make_shared<Scene>();
    m_scene->name = scene;
    if (prismaScene) {
        m_exporter.importSceneAsync(scene);
    }
}

std::shared_ptr<Prisma::Scene> Prisma::SceneLoader::hasFinish() {
    if (m_exporter.hasFinish()) {
        auto newRootNode = std::make_shared<Node>();
        newRootNode = m_exporter.newRootNode();
        newRootNode->parent(nullptr);
        m_scene->root = newRootNode;

        NodeHelper nodeHelper;

        nodeHelper.nodeIterator(m_scene->root, [&](auto node, auto parent) {
            auto isAnimateMesh = std::dynamic_pointer_cast<AnimatedMesh>(node);
            auto isMesh = std::dynamic_pointer_cast<Mesh>(node);
            auto isLightDir = std::dynamic_pointer_cast<Light<LightType::LightDir>>(node);
            auto isLightOmni = std::dynamic_pointer_cast<Light<LightType::LightOmni>>(node);
            auto isLightArea = std::dynamic_pointer_cast<Light<LightType::LightArea>>(node);
            if (isAnimateMesh) {
                m_scene->animateMeshes.push_back(isAnimateMesh);
            } else if (isMesh) {
                m_scene->meshes.push_back(isMesh);
            } else if (isLightDir) {
                m_scene->dirLights.push_back(isLightDir);
            } else if (isLightOmni) {
                m_scene->omniLights.push_back(isLightOmni);
            } else if (isLightArea) {
                m_scene->areaLights.push_back(isLightArea);
            }
        });
        return m_scene;
    }
    if (m_loading) {
        m_loading();
    }
    return nullptr;
}

const aiScene* Prisma::SceneLoader::assimpScene() {
    return m_aScene;
}

Prisma::Exporter& Prisma::SceneLoader::exporter() {
    return m_exporter;
}

void Prisma::SceneLoader::onLoading(std::function<void()> loading) {
    m_loading = loading;
}

float Prisma::SceneLoader::calculateOmniLightRadius(float Kc, float Kl, float Kq, float I_threshold) {
    // Calculate coefficients for the quadratic equation
    float a = Kq;
    float b = Kl;
    float c = Kc - sqrt(I_threshold) / fmax(Kc, 1);

    // Calculate the discriminant
    float discriminant = b * b - 4 * a * c;

    // Check if the discriminant is negative (no real roots)
    if (discriminant < 0) {
        return -1; // No real roots, light never reaches threshold
    }

    // Calculate the roots using the quadratic formula
    float root1 = (-b + sqrt(discriminant)) / (2 * a);
    float root2 = (-b - sqrt(discriminant)) / (2 * a);

    // The radius should be positive, so we take the positive root if it exists
    float radius = fmax(root1, root2);

    return radius;
}

void Prisma::SceneLoader::nodeIteration(std::shared_ptr<Node> nodeRoot, aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        auto currentNode = std::make_shared<Node>();
        glm::mat4 transform = getTransform(node->mChildren[i]->mTransformation);
        currentNode->matrix(transform, false);
        currentNode->finalMatrix(nodeRoot->finalMatrix() * transform, false);
        currentNode->name(node->mChildren[i]->mName.C_Str());
        nodeRoot->addChild(currentNode, false);
        nodeIteration(nodeRoot->children()[i], node->mChildren[i], scene);
    }
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto currentMesh = getMesh(mesh, scene);
        auto isAnimate = std::dynamic_pointer_cast<AnimatedMesh>(currentMesh);
        currentMesh->matrix(glm::mat4(1.0f), false);
        currentMesh->finalMatrix(glm::mat4(1.0f), false);

        nodeRoot->addChild(currentMesh, false);
        if (isAnimate) {
            if (m_scene->animateMeshes.size() < Define::MAX_ANIMATION_MESHES) {
                m_scene->animateMeshes.push_back(isAnimate);
            } else {
                std::cerr << "MAX ANIMATION MESHES REACHED" << std::endl;
            }
        } else {
            m_scene->meshes.push_back(currentMesh);
        }
    }
}

void Prisma::SceneLoader::setVertexBoneDataToDefault(AnimatedMesh::AnimateVertex& vertex) {
    for (int i = 0; i < Define::MAX_BONE_INFLUENCE; i++) {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

std::shared_ptr<Prisma::Mesh> Prisma::SceneLoader::getMesh(aiMesh* mesh, const aiScene* scene) {
    std::shared_ptr<Mesh> currentMesh = nullptr;
    auto data = std::make_shared<Mesh::VerticesData>();
    auto animeteData = std::make_shared<AnimatedMesh::AnimateVerticesData>();

    if (mesh->mNumBones > 0) {
        currentMesh = std::make_shared<AnimatedMesh>();
    } else {
        currentMesh = std::make_shared<Mesh>();
    }

    currentMesh->name(mesh->mName.C_Str());

    if (!mesh->HasBones()) {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            Mesh::Vertex vertex{};
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;
            if (mesh->HasNormals()) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }

            if (mesh->mTextureCoords[0]) {
                glm::vec2 vec;

                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;

                vertex.texCoords = vec;
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent = vector;
            } else
                vertex.texCoords = glm::vec2(0.0f, 0.0f);

            data->vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                data->indices.push_back(face.mIndices[j]);
        }
    } else {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            AnimatedMesh::AnimateVertex vertex{};

            setVertexBoneDataToDefault(vertex);
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;
            if (mesh->HasNormals()) {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }

            if (mesh->mTextureCoords[0]) {
                glm::vec2 vec;

                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.texCoords = vec;

                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.bitangent = vector;
            } else
                vertex.texCoords = glm::vec2(0.0f, 0.0f);

            animeteData->vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                animeteData->indices.push_back(face.mIndices[j]);
        }
        extractBoneWeightForVertices(std::dynamic_pointer_cast<AnimatedMesh>(currentMesh), animeteData, mesh,
                                     scene);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    auto currentMaterial = std::make_shared<MaterialComponent>();
    currentMaterial->diffuse(loadMaterialTextures(material, aiTextureType_DIFFUSE, m_sceneParameters.srgb));
    currentMaterial->normal(loadMaterialTextures(material, aiTextureType_NORMALS));
    currentMaterial->roughnessMetalness(loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS));
    currentMaterial->specular(loadMaterialTextures(material, aiTextureType_SPECULAR));
    currentMaterial->ambientOcclusion(loadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION));
    float roughness = 0;
    float metalness = 0;
    material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
    material->Get(AI_MATKEY_METALLIC_FACTOR, metalness);
    currentMaterial->roughness(roughness);
    currentMaterial->metalness(metalness);
    /*std::vector<Texture> emptyVector;
    if (currentMaterial->diffuse().empty())
    {
            emptyVector.clear();
            emptyVector.push_back(Prisma::GlobalData::getInstance().defaultBlack());
            currentMaterial->diffuse(emptyVector);
            Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
                                              "No diffuse texture " + currentMesh->name() + " MaterialComponent name: " +
                                              material->GetName().
                                                        C_Str());
    }
    if (currentMaterial->normal().empty())
    {
            emptyVector.clear();
            emptyVector.push_back(Prisma::GlobalData::getInstance().defaultNormal());
            currentMaterial->normal(emptyVector);
            Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
                                              "No normal texture " + currentMesh->name() + " MaterialComponent name: " +
                                              material->GetName().
                                                        C_Str());
    }
    if (currentMaterial->roughnessMetalness().empty())
    {
            emptyVector.clear();
            emptyVector.push_back(Prisma::GlobalData::getInstance().defaultBlack());
            currentMaterial->roughnessMetalness(emptyVector);
            Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
                                              "No roughness or metalness texture " + currentMesh->name() +
                                              " MaterialComponent name: " +
                                              material->GetName().
                                                        C_Str());
    }
    if (currentMaterial->specular().empty())
    {
            emptyVector.clear();
            emptyVector.push_back(Prisma::GlobalData::getInstance().defaultWhite());
            currentMaterial->specular(emptyVector);
            Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
                                              "No specular texture " + currentMesh->name() + " MaterialComponent name: " +
                                              material->GetName().
                                                        C_Str());
    }
    if (currentMaterial->ambientOcclusion().empty())
    {
            emptyVector.clear();
            emptyVector.push_back(Prisma::GlobalData::getInstance().defaultWhite());
            currentMaterial->ambientOcclusion(emptyVector);
            Prisma::Logger::getInstance().log(Prisma::LogLevel::WARN,
                                              "No ambient occlusion texture " + currentMesh->name() +
                                              " MaterialComponent name: " +
                                              material->GetName().
                                                        C_Str());
    }*/

    currentMaterial->name(material->GetName().C_Str());

    currentMesh->material(currentMaterial);

    if (mesh->HasBones()) {
        auto animatedMesh = std::dynamic_pointer_cast<AnimatedMesh>(currentMesh);
        animatedMesh->loadAnimateModel(animeteData);
        animatedMesh->path(m_path);
    } else {
        currentMesh->loadModel(data);
    }
    return currentMesh;
}

std::vector<Prisma::Texture> Prisma::SceneLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, bool srgb) {
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        std::string name = m_folder + str.C_Str();
        texture.name(name);
        texture.parameters({name, srgb, Define::DEFAULT_MIPS, true});
        textures.push_back(texture);
    }

    return textures;
}

void Prisma::SceneLoader::loadLights(const aiScene* currentScene, std::shared_ptr<Node> root) {
    for (int i = 0; i < currentScene->mNumLights; i++) {
        auto assimpLight = currentScene->mLights[i];
        switch (assimpLight->mType) {
            case aiLightSource_DIRECTIONAL: {
                auto light = std::make_shared<Light<LightType::LightDir>>();
                LightType::LightDir lightDir;

                glm::vec3 color;
                color.r = assimpLight->mColorSpecular.r;
                color.g = assimpLight->mColorSpecular.g;
                color.b = assimpLight->mColorSpecular.b;
                lightDir.specular = glm::vec4(color, 1.0f);

                color.r = assimpLight->mColorDiffuse.r;
                color.g = assimpLight->mColorDiffuse.g;
                color.b = assimpLight->mColorDiffuse.b;
                lightDir.diffuse = glm::vec4(color, 1.0f);

                color.r = assimpLight->mDirection.x;
                color.g = assimpLight->mDirection.y;
                color.b = assimpLight->mDirection.z;
                lightDir.direction = glm::vec4(color, 1.0f);

                light->type(lightDir);

                light->name(assimpLight->mName.C_Str());
                auto lightNode = m_nodeFinder.find(root, light->name());
                lightNode->addChild(light, false);
                light->finalMatrix(lightNode->finalMatrix(), false);
                light->createShadow(Define::MAX_SHADOW_DIR_TEXTURE_SIZE,
                                    Define::MAX_SHADOW_DIR_TEXTURE_SIZE);

                m_scene->dirLights.push_back(light);

                break;
            }
            case aiLightSource_POINT: {
                auto light = std::make_shared<Light<LightType::LightOmni>>();
                LightType::LightOmni lightOmni;

                glm::vec3 color;
                color.r = assimpLight->mColorSpecular.r;
                color.g = assimpLight->mColorSpecular.g;
                color.b = assimpLight->mColorSpecular.b;
                lightOmni.specular = glm::vec4(color, 1.0f);

                color.r = assimpLight->mColorDiffuse.r;
                color.g = assimpLight->mColorDiffuse.g;
                color.b = assimpLight->mColorDiffuse.b;
                lightOmni.diffuse = glm::vec4(color, 1.0f);

                color.r = assimpLight->mPosition.x;
                color.g = assimpLight->mPosition.y;
                color.b = assimpLight->mPosition.z;
                lightOmni.position = glm::vec4(color, 1.0f);
                lightOmni.farPlane.x = 100.0f;
                lightOmni.radius = 5;

                lightOmni.attenuation = glm::vec4(assimpLight->mAttenuationConstant,
                                                  assimpLight->mAttenuationLinear,
                                                  assimpLight->mAttenuationQuadratic, 1.0f);

                light->type(lightOmni);
                light->name(assimpLight->mName.C_Str());
                auto lightNode = m_nodeFinder.find(root, light->name());
                lightNode->addChild(light, false);
                light->finalMatrix(lightNode->finalMatrix(), false);
                light->createShadow(Define::MAX_SHADOW_OMNI_TEXTURE_SIZE,
                                    Define::MAX_SHADOW_OMNI_TEXTURE_SIZE);

                m_scene->omniLights.push_back(light);
                break;
            }
            case aiLightSource_SPOT:
            case aiLightSource_UNDEFINED:
            case aiLightSource_AMBIENT:
            case aiLightSource_AREA:
            case _aiLightSource_Force32Bit:
                break;
        }
    }
}

void Prisma::SceneLoader::setVertexBoneData(AnimatedMesh::AnimateVertex& vertex, int boneID, float weight) {
    // Try to find an empty slot
    for (int i = 0; i < Define::MAX_BONE_INFLUENCE; ++i) {
        if (vertex.m_BoneIDs[i] < 0 || vertex.m_Weights[i] == 0.0f) {
            vertex.m_BoneIDs[i] = boneID;
            vertex.m_Weights[i] = weight;
            return;
        }
    }

    // All slots are filled, replace the smallest weight if the new one is larger
    int minIndex = 0;
    for (int i = 1; i < Define::MAX_BONE_INFLUENCE; ++i) {
        if (vertex.m_Weights[i] < vertex.m_Weights[minIndex]) {
            minIndex = i;
        }
    }

    if (weight > vertex.m_Weights[minIndex]) {
        vertex.m_BoneIDs[minIndex] = boneID;
        vertex.m_Weights[minIndex] = weight;
        // Optional: Log replacement
        // std::cerr << "Replaced lowest weight bone influence for vertex.\n";
    } else {
        // Optional: Log skipped weight
        // std::cerr << "Skipped adding bone ID " << boneID << " with weight " << weight << " due to low influence.\n";
    }
}

void Prisma::SceneLoader::extractBoneWeightForVertices(std::shared_ptr<AnimatedMesh> animatedMesh, std::shared_ptr<AnimatedMesh::AnimateVerticesData> vertices, aiMesh* mesh, const aiScene* scene) {
    auto& boneInfoMap = animatedMesh->boneInfoMap();
    int& boneCount = animatedMesh->boneInfoCounter();

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();

        // Assign or retrieve bone ID
        if (boneInfoMap.find(boneName) == boneInfoMap.end()) {
            BoneInfo newBoneInfo;
            newBoneInfo.id = boneCount;
            newBoneInfo.offset = getTransform(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID = boneCount++;
        } else {
            boneID = boneInfoMap[boneName].id;
        }

        assert(boneID != -1);

        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;

            assert(vertexId < vertices->vertices.size());  // Fixed comparison
            setVertexBoneData(vertices->vertices[vertexId], boneID, weight);
        }
    }
}