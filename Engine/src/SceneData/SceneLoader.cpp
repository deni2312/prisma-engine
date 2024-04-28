#include "../../include/SceneData/SceneLoader.h"
#include "../../include/GlobalData/Defines.h"
#include <iostream>
#include <memory>
#include "glm/gtx/string_cast.hpp"
#include "../../include/Components/Component.h"
#include "../../include/Components/MaterialComponent.h"
#include "../../include/Helpers/PrismaMath.h"

std::shared_ptr<Prisma::Scene> Prisma::SceneLoader::loadScene(std::string scene, SceneParameters sceneParameters)
{

    black.loadTexture(DIR_DEFAULT_BLACK);
    normal.loadTexture(DIR_DEFAULT_NORMAL);
    m_sceneParameters = sceneParameters;
    // Extracting the directory to the last folder
    size_t lastSlash = scene.find_last_of("/");

    if (lastSlash != std::string::npos) {
        m_folder = scene.substr(0, lastSlash)+"/";
    }
    else {
        lastSlash = scene.find_last_of("\\");
        if (lastSlash != std::string::npos) {
            m_folder = scene.substr(0, lastSlash) + "\\";
        }
        else {
            // Handle the case where there is no directory
            m_folder = "";
        }
    }
	Assimp::Importer importer;
	const aiScene* currentScene = importer.ReadFile(scene, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (currentScene) {
        m_scene = std::make_shared<Scene>();
        std::shared_ptr<Node> root = std::make_shared<Node>();
        root->name(currentScene->mRootNode->mName.C_Str());
        glm::mat4 transform = getTransform(currentScene->mRootNode->mTransformation);
        root->matrix(transform,false);
        root->finalMatrix(transform,false);
        root->parent(nullptr);
        m_scene->root = root;
        nodeIteration(root, currentScene->mRootNode, currentScene);
        loadLights(currentScene, root);
        return m_scene;
    }
    else {
        std::cerr << "Could not find the directory" << std::endl;
        return nullptr;
    }
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
        return -1;  // No real roots, light never reaches threshold
    }

    // Calculate the roots using the quadratic formula
    float root1 = (-b + sqrt(discriminant)) / (2 * a);
    float root2 = (-b - sqrt(discriminant)) / (2 * a);

    // The radius should be positive, so we take the positive root if it exists
    float radius = fmax(root1, root2);

    return radius;
}

void Prisma::SceneLoader::nodeIteration(std::shared_ptr<Node> nodeRoot, aiNode* node, const aiScene* scene)
{

    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        auto currentNode = std::make_shared<Node>();
        glm::mat4 transform = getTransform(node->mChildren[i]->mTransformation);
        currentNode->matrix(transform, false);
        currentNode->finalMatrix(nodeRoot->finalMatrix() * transform, false);
        currentNode->name(node->mChildren[i]->mName.C_Str());
        currentNode->parent(nodeRoot);
        nodeRoot->addChild(currentNode, false);
        nodeIteration(nodeRoot->children()[i], node->mChildren[i], scene);
    }
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        auto currentMesh = getMesh(mesh, scene);
        auto isAnimate = std::dynamic_pointer_cast<AnimatedMesh>(currentMesh);
        currentMesh->matrix(glm::mat4(1.0f), false);
        currentMesh->finalMatrix(glm::mat4(1.0f), false);
        currentMesh->parent(nodeRoot);

        if (!isAnimate) {
            currentMesh->computeAABB();
        }

        nodeRoot->addChild(currentMesh,false);
        if (isAnimate) {
            m_scene->animateMeshes.push_back(isAnimate);
        }
        else {
            m_scene->meshes.push_back(currentMesh);
        }
        
    }
}

void Prisma::SceneLoader::setVertexBoneDataToDefault(Prisma::AnimatedMesh::AnimateVertex& vertex)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        vertex.m_BoneIDs[i] = -1;
        vertex.m_Weights[i] = 0.0f;
    }
}

std::shared_ptr<Prisma::Mesh> Prisma::SceneLoader::getMesh(aiMesh* mesh, const aiScene* scene)
{
    std::shared_ptr<Prisma::Mesh> currentMesh = nullptr;
    std::shared_ptr<Prisma::Mesh::VerticesData> data = std::make_shared<Prisma::Mesh::VerticesData>();
    std::shared_ptr<Prisma::AnimatedMesh::AnimateVerticesData> animeteData = std::make_shared<Prisma::AnimatedMesh::AnimateVerticesData>();


    if (mesh->mNumBones > 0) {
        currentMesh = std::make_shared<Prisma::AnimatedMesh>();
    }
    else {
        currentMesh = std::make_shared<Prisma::Mesh>();
    }

    currentMesh->name(mesh->mName.C_Str());


    if (mesh->mNumBones == 0) {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Prisma::Mesh::Vertex vertex{};
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }

            if (mesh->mTextureCoords[0])
            {
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
            }
            else
                vertex.texCoords = glm::vec2(0.0f, 0.0f);

            data->vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                data->indices.push_back(face.mIndices[j]);
        }
    }
    else {
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Prisma::AnimatedMesh::AnimateVertex vertex{};

            setVertexBoneDataToDefault(vertex);
            glm::vec3 vector;
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.position = vector;
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.normal = vector;
            }

            if (mesh->mTextureCoords[0])
            {
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
            }
            else
                vertex.texCoords = glm::vec2(0.0f, 0.0f);

            animeteData->vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                animeteData->indices.push_back(face.mIndices[j]);
        }
        extractBoneWeightForVertices(std::dynamic_pointer_cast<AnimatedMesh>(currentMesh), animeteData, mesh, scene);
    }

    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    std::shared_ptr<Prisma::MaterialComponent> currentMaterial = std::make_shared <MaterialComponent>();
    currentMaterial->diffuse(loadMaterialTextures(material, aiTextureType_DIFFUSE,m_sceneParameters.srgb));
    currentMaterial->normal(loadMaterialTextures(material, aiTextureType_NORMALS));
    currentMaterial->roughness_metalness(loadMaterialTextures(material, aiTextureType_DIFFUSE_ROUGHNESS));
    std::vector<Prisma::Texture> emptyVector;
    if (currentMaterial->diffuse().empty()) {
        emptyVector.push_back(black);
        currentMaterial->diffuse(emptyVector);
        std::cout << "No diffuse texture " + currentMesh->name()+" MaterialComponent name: "+ material->GetName().C_Str() << std::endl;
    }
    if (currentMaterial->normal().empty()) {
        emptyVector.push_back(normal);
        currentMaterial->normal(emptyVector);
        std::cout << "No normal texture " + currentMesh->name() + " MaterialComponent name: " + material->GetName().C_Str() << std::endl;
    }
    if (currentMaterial->roughness_metalness().empty()) {
        emptyVector.push_back(black);
        currentMaterial->roughness_metalness(emptyVector);
        std::cout << "No roughness or metalness texture " + currentMesh->name() + " MaterialComponent name: " + material->GetName().C_Str() << std::endl;
    }

    currentMesh->material(currentMaterial);

    if (mesh->mNumBones > 0) {
        std::dynamic_pointer_cast<AnimatedMesh>(currentMesh)->loadAnimateModel(animeteData);
    }
    else {
        currentMesh->loadModel(data);
    }
    return currentMesh;
}

std::vector<Prisma::Texture> Prisma::SceneLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type, bool srgb)
{
    std::vector<Prisma::Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for (auto & texture : textures_loaded)
        {
            if (texture.name() == m_folder+str.C_Str())
            {
                textures.push_back(texture);
                skip = true;
                break;
            }
        }
        if (!skip)
        {
            Prisma::Texture texture;
            std::string name = str.C_Str();
            name = m_folder + name;
            if (!texture.loadTexture(name,srgb)) {
                std::cerr << "Texture " + name + " not found" << std::endl;
            }
            else 
            {
                texture.name(name);
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
    }
    return textures;
}

void Prisma::SceneLoader::loadLights(const aiScene* currentScene, std::shared_ptr<Node> root)
{
    for (int i = 0; i < currentScene->mNumLights; i++) {
        auto assimpLight = currentScene->mLights[i];
        switch (assimpLight->mType)
        {
        case aiLightSource_DIRECTIONAL: {
            auto light = std::make_shared<Prisma::Light<Prisma::LightType::LightDir>>();
            Prisma::LightType::LightDir lightDir;

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
            auto lightNode = nodeFinder.find(root, light->name());
            lightNode->addChild(light, false);
            light->finalMatrix(lightNode->finalMatrix(), false);
            light->parent(lightNode);
            light->createShadow(4096, 4096);

            m_scene->dirLights.push_back(light);

            break;
        }
        case aiLightSource_POINT:
        {
            auto light = std::make_shared<Prisma::Light<Prisma::LightType::LightOmni>>();
            Prisma::LightType::LightOmni lightOmni;

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
            lightOmni.shadowMap = 0;
            lightOmni.radius = 1;
            light->type(lightOmni);
            light->name(assimpLight->mName.C_Str());
            auto lightNode = nodeFinder.find(root, light->name());
            light->parent(lightNode);
            lightNode->addChild(light, false);
            light->finalMatrix(lightNode->finalMatrix(), false);
            light->parent(lightNode);
            light->createShadow(1024, 1024);

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

void Prisma::SceneLoader::setVertexBoneData(Prisma::AnimatedMesh::AnimateVertex& vertex, int boneID, float weight)
{
    for (int i = 0; i < MAX_BONE_INFLUENCE; ++i)
    {
        if (vertex.m_BoneIDs[i] < 0)
        {
            vertex.m_Weights[i] = weight;
            vertex.m_BoneIDs[i] = boneID;
            break;
        }
    }
}


void Prisma::SceneLoader::extractBoneWeightForVertices(std::shared_ptr<Prisma::AnimatedMesh> animatedMesh, std::shared_ptr<AnimatedMesh::AnimateVerticesData> vertices, aiMesh* mesh, const aiScene* scene)
{
    auto& boneInfoMap = animatedMesh->boneInfoMap();
    int& boneCount = animatedMesh->boneInfoCounter();

    for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex)
    {
        int boneID = -1;
        std::string boneName = mesh->mBones[boneIndex]->mName.C_Str();
        if (boneInfoMap.find(boneName) == boneInfoMap.end())
        {
            Prisma::BoneInfo newBoneInfo;
            newBoneInfo.id = boneCount;
            newBoneInfo.offset = getTransform(mesh->mBones[boneIndex]->mOffsetMatrix);
            boneInfoMap[boneName] = newBoneInfo;
            boneID = boneCount;
            boneCount++;
        }
        else
        {
            boneID = boneInfoMap[boneName].id;
        }
        assert(boneID != -1);
        auto weights = mesh->mBones[boneIndex]->mWeights;
        int numWeights = mesh->mBones[boneIndex]->mNumWeights;

        for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex)
        {
            int vertexId = weights[weightIndex].mVertexId;
            float weight = weights[weightIndex].mWeight;
            assert(vertexId <= vertices->vertices.size());
            setVertexBoneData(vertices->vertices[vertexId], boneID, weight);
        }
    }
}
