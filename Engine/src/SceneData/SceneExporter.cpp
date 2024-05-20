#include "../../include/SceneData/SceneExporter.h"
#include <iostream>
#include "../../include/GlobalData/GlobalData.h"
#include <assimp/Exporter.hpp>
#include "../../../vcpkg_installed/x64-windows/include/assimp/DefaultLogger.hpp"

std::shared_ptr<Prisma::Exporter> Prisma::Exporter::instance = nullptr;
Assimp::Importer importer;
Assimp::Exporter exporter;


Prisma::Exporter::Exporter()
{

}


void Prisma::Exporter::exportScene()
{
    if (!currentGlobalScene || !currentGlobalScene->root) {
        std::cerr << "Error: No scene data available to export." << std::endl;
        return;
    }
    m_scene = importer.ReadFile(DIR_DEFAULT_SCENE, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    unsigned int sizeMesh = currentGlobalScene->meshes.size() + currentGlobalScene->animateMeshes.size();

    const_cast<aiScene*>(m_scene)->mMeshes = new aiMesh*[sizeMesh];
    const_cast<aiScene*>(m_scene)->mNumMeshes = sizeMesh;
    const_cast<aiScene*>(m_scene)->mFlags = 0;

    const_cast<aiScene*>(m_scene)->mNumMaterials = 1;
    const_cast<aiScene*>(m_scene)->mMaterials = new aiMaterial * [m_scene->mNumMaterials];
    const_cast<aiScene*>(m_scene)->mMaterials[0] = new aiMaterial();

    for (int i = 0; i < currentGlobalScene->meshes.size();i++) {
        const_cast<aiScene*>(m_scene)->mMeshes[i] = getMesh(currentGlobalScene->meshes[i]);
    }

    for (int i = 0; i < currentGlobalScene->animateMeshes.size(); i++) {
        const_cast<aiScene*>(m_scene)->mMeshes[currentGlobalScene->meshes.size()+i] = getMesh(currentGlobalScene->animateMeshes[i]);
    }

    addNodesRecursively(currentGlobalScene->root, m_scene->mRootNode);

    std::string outputFilePath = "C:\\Users\\denis\\Downloads\\prisma-engine\\Resources\\Landscape\\landscape1.gltf";
    aiReturn result = exporter.Export(m_scene, "gltf2", outputFilePath);
    Assimp::DefaultLogger::create(ASSIMP_DEFAULT_LOG_NAME, Assimp::Logger::VERBOSE);
    Assimp::DefaultLogger::get()->attachStream(Assimp::LogStream::createDefaultStream(aiDefaultLogStream_STDOUT), Assimp::Logger::Debugging);

    if (result != aiReturn_SUCCESS) {
        std::cerr << "Error exporting: " << exporter.GetErrorString() << std::endl;
    }
    else {
        std::cout << "Scene exported successfully to " << outputFilePath << std::endl;
    }
}

Prisma::Exporter& Prisma::Exporter::getInstance()
{
    if (!instance) {
        instance = std::make_shared<Prisma::Exporter>();
    }
    return *instance;
}

void Prisma::Exporter::addNodesRecursively(const std::shared_ptr<Prisma::Node>& sceneNode, aiNode* next) {
    if (!sceneNode || !next) {
        return;
    }

    auto childrenSize = sceneNode->children().size();
    if (childrenSize > 0) {
        next->mNumChildren = static_cast<unsigned int>(childrenSize);
        next->mChildren = new aiNode*[childrenSize]();
        for (unsigned int i = 0; i < childrenSize; i++) {
            aiNode* childNode = new aiNode();
            childNode->mName = sceneNode->children()[i]->name();
            next->mChildren[i] = childNode;
            if (std::dynamic_pointer_cast<Mesh>(sceneNode->children()[i])) {
                next->mChildren[i]->mMeshes = new unsigned int[1];
                int index = 0;
                for (auto mesh : currentGlobalScene->meshes) {
                    if (mesh->uuid() == sceneNode->children()[i]->uuid()) {
                        next->mChildren[i]->mMeshes[0] = index;
                        next->mChildren[i]->mNumMeshes = 1;
                        break;
                    }
                    index++;
                }

                for (auto mesh : currentGlobalScene->animateMeshes) {
                    if (mesh->uuid() == sceneNode->children()[i]->uuid()) {
                        next->mChildren[i]->mMeshes[0] = index;
                        next->mChildren[i]->mNumMeshes = 1;
                        break;
                    }
                    index++;
                }


            }
            else {
                next->mChildren[i]->mNumMeshes = 0;
            }
            next->mChildren[i]->mParent = next;
            next->mChildren[i]->mTransformation = glmToAiMatrix4x4(sceneNode->children()[i]->matrix());
            addNodesRecursively(sceneNode->children()[i], childNode);
        }
    }



}

aiMesh* Prisma::Exporter::getMesh(std::shared_ptr<Prisma::Mesh> mesh) {
    auto anim_mesh = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(mesh);
    aiMesh* ai_mesh = new aiMesh();
    if (!anim_mesh) {
        ai_mesh->mMaterialIndex = 0;
        // Set the number of vertices and allocate space
        ai_mesh->mNumVertices = mesh->verticesData().vertices.size();
        ai_mesh->mVertices = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->mNormals = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->mTextureCoords[0] = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->mTangents = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->mBitangents = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->HasTextureCoordsName(false);
        ai_mesh->HasVertexColors(false);
        ai_mesh->mNumUVComponents[0] = 2;
        ai_mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
        // Fill vertex data
        for (unsigned int i = 0; i < ai_mesh->mNumVertices; ++i) {
            ai_mesh->mVertices[i] = aiVector3D(mesh->verticesData().vertices[i].position.x, mesh->verticesData().vertices[i].position.y, mesh->verticesData().vertices[i].position.z);
            ai_mesh->mNormals[i] = aiVector3D(mesh->verticesData().vertices[i].normal.x, mesh->verticesData().vertices[i].normal.y, mesh->verticesData().vertices[i].normal.z);
            ai_mesh->mTextureCoords[0][i] = aiVector3D(mesh->verticesData().vertices[i].texCoords.x, mesh->verticesData().vertices[i].texCoords.y, 0);
            ai_mesh->mTangents[i] = aiVector3D(mesh->verticesData().vertices[i].tangent.x, mesh->verticesData().vertices[i].tangent.y, mesh->verticesData().vertices[i].tangent.z);
            ai_mesh->mBitangents[i] = aiVector3D(mesh->verticesData().vertices[i].bitangent.x, mesh->verticesData().vertices[i].bitangent.y, mesh->verticesData().vertices[i].bitangent.z);
        }

        // Set the number of faces and allocate space
        ai_mesh->mNumFaces = mesh->verticesData().indices.size() / 3;
        ai_mesh->mFaces = new aiFace[ai_mesh->mNumFaces];

        // Fill face data
        for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
            aiFace& face = ai_mesh->mFaces[i];
            face.mNumIndices = 3;
            face.mIndices = new unsigned int[3];
            face.mIndices[0] = mesh->verticesData().indices[3 * i];
            face.mIndices[1] = mesh->verticesData().indices[3 * i + 1];
            face.mIndices[2] = mesh->verticesData().indices[3 * i + 2];
        }

        // Set mesh name
        ai_mesh->mName = aiString(mesh->name());
    }
    else {
        ai_mesh->mMaterialIndex = 0;
        // Set the number of vertices and allocate space
        ai_mesh->mNumVertices = anim_mesh->animateVerticesData()->vertices.size();
        ai_mesh->mVertices = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->mNormals = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->mTextureCoords[0] = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->mTangents = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->mBitangents = new aiVector3D[ai_mesh->mNumVertices];
        ai_mesh->HasTextureCoordsName(false);
        ai_mesh->HasVertexColors(false);
        ai_mesh->mNumUVComponents[0] = 2;
        ai_mesh->mPrimitiveTypes = aiPrimitiveType_TRIANGLE;
        // Fill vertex data
        for (unsigned int i = 0; i < ai_mesh->mNumVertices; ++i) {
            ai_mesh->mVertices[i] = aiVector3D(anim_mesh->animateVerticesData()->vertices[i].position.x, anim_mesh->animateVerticesData()->vertices[i].position.y, anim_mesh->animateVerticesData()->vertices[i].position.z);
            ai_mesh->mNormals[i] = aiVector3D(anim_mesh->animateVerticesData()->vertices[i].normal.x, anim_mesh->animateVerticesData()->vertices[i].normal.y, anim_mesh->animateVerticesData()->vertices[i].normal.z);
            ai_mesh->mTextureCoords[0][i] = aiVector3D(anim_mesh->animateVerticesData()->vertices[i].texCoords.x, anim_mesh->animateVerticesData()->vertices[i].texCoords.y, 0);
            ai_mesh->mTangents[i] = aiVector3D(anim_mesh->animateVerticesData()->vertices[i].tangent.x, anim_mesh->animateVerticesData()->vertices[i].tangent.y, anim_mesh->animateVerticesData()->vertices[i].tangent.z);
            ai_mesh->mBitangents[i] = aiVector3D(anim_mesh->animateVerticesData()->vertices[i].bitangent.x, anim_mesh->animateVerticesData()->vertices[i].bitangent.y, anim_mesh->animateVerticesData()->vertices[i].bitangent.z);
        }

        // Set the number of faces and allocate space
        ai_mesh->mNumFaces = anim_mesh->animateVerticesData()->indices.size() / 3;
        ai_mesh->mFaces = new aiFace[ai_mesh->mNumFaces];

        // Fill face data
        for (unsigned int i = 0; i < ai_mesh->mNumFaces; ++i) {
            aiFace& face = ai_mesh->mFaces[i];
            face.mNumIndices = 3;
            face.mIndices = new unsigned int[3];
            face.mIndices[0] = anim_mesh->animateVerticesData()->indices[3 * i];
            face.mIndices[1] = anim_mesh->animateVerticesData()->indices[3 * i + 1];
            face.mIndices[2] = anim_mesh->animateVerticesData()->indices[3 * i + 2];
        }

        // If mesh has bones, set bone data
        /*ai_mesh->mNumBones = anim_mesh->boneInfoMap().size();
        ai_mesh->mBones = new aiBone * [ai_mesh->mNumBones];
        int i = 0;
        for (auto& boneInfo : anim_mesh->boneInfoMap()) {
            const auto& bone = boneInfo;
            aiBone* ai_bone = new aiBone();
            ai_bone->mName = aiString(bone.first);
            ai_bone->mOffsetMatrix = glmToAiMatrix4x4(bone.second.offset); // Assuming bone.offsetMatrix is of type aiMatrix4x4
            ai_bone->mNumWeights = 4;
            ai_bone->mWeights = new aiVertexWeight[ai_bone->mNumWeights];
            for (unsigned int j = 0; j < ai_bone->mNumWeights; ++j) {
                auto weights = anim_mesh->animateVerticesData()->vertices[bone.second.id];

                ai_bone->mWeights[j].mVertexId = weights.m_BoneIDs[j];
                ai_bone->mWeights[j].mWeight = weights.m_Weights[j];
            }

            ai_mesh->mBones[i] = ai_bone;
            i++;
        }*/

        // Set mesh name
        ai_mesh->mName = aiString(mesh->name());
    }

    // Return the constructed aiMesh
    return ai_mesh;
}

aiMatrix4x4 Prisma::Exporter::glmToAiMatrix4x4(const glm::mat4& from) {
    aiMatrix4x4 to;
    to.a1 = from[0][0]; to.a2 = from[1][0]; to.a3 = from[2][0]; to.a4 = from[3][0];
    to.b1 = from[0][1]; to.b2 = from[1][1]; to.b3 = from[2][1]; to.b4 = from[3][1];
    to.c1 = from[0][2]; to.c2 = from[1][2]; to.c3 = from[2][2]; to.c4 = from[3][2];
    to.d1 = from[0][3]; to.d2 = from[1][3]; to.d3 = from[2][3]; to.d4 = from[3][3];
    return to;
}
