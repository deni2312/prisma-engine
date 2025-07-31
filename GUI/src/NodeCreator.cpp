#include "../include/NodeCreator.h"

std::shared_ptr<Prisma::Node> Prisma::GUI::NodeCreator::createCube() {
    auto verticesData = std::make_shared<Mesh::VerticesData>();

    // Define the vertices for each face of the cube
    std::vector<Mesh::Vertex> vertices = {
        // Front face
        {{-1, -1, 1}, {0, 0, 1}, {0, 0}}, // Bottom-left
        {{1, -1, 1}, {0, 0, 1}, {1, 0}}, // Bottom-right
        {{1, 1, 1}, {0, 0, 1}, {1, 1}}, // Top-right
        {{-1, 1, 1}, {0, 0, 1}, {0, 1}}, // Top-left

        // Back face
        {{1, -1, -1}, {0, 0, -1}, {0, 0}}, // Bottom-left
        {{-1, -1, -1}, {0, 0, -1}, {1, 0}}, // Bottom-right
        {{-1, 1, -1}, {0, 0, -1}, {1, 1}}, // Top-right
        {{1, 1, -1}, {0, 0, -1}, {0, 1}}, // Top-left

        // Left face
        {{-1, -1, -1}, {-1, 0, 0}, {0, 0}}, // Bottom-left
        {{-1, -1, 1}, {-1, 0, 0}, {1, 0}}, // Bottom-right
        {{-1, 1, 1}, {-1, 0, 0}, {1, 1}}, // Top-right
        {{-1, 1, -1}, {-1, 0, 0}, {0, 1}}, // Top-left

        // Right face
        {{1, -1, 1}, {1, 0, 0}, {0, 0}}, // Bottom-left
        {{1, -1, -1}, {1, 0, 0}, {1, 0}}, // Bottom-right
        {{1, 1, -1}, {1, 0, 0}, {1, 1}}, // Top-right
        {{1, 1, 1}, {1, 0, 0}, {0, 1}}, // Top-left

        // Top face
        {{-1, 1, 1}, {0, 1, 0}, {0, 0}}, // Bottom-left
        {{1, 1, 1}, {0, 1, 0}, {1, 0}}, // Bottom-right
        {{1, 1, -1}, {0, 1, 0}, {1, 1}}, // Top-right
        {{-1, 1, -1}, {0, 1, 0}, {0, 1}}, // Top-left

        // Bottom face
        {{-1, -1, -1}, {0, -1, 0}, {0, 0}}, // Bottom-left
        {{1, -1, -1}, {0, -1, 0}, {1, 0}}, // Bottom-right
        {{1, -1, 1}, {0, -1, 0}, {1, 1}}, // Top-right
        {{-1, -1, 1}, {0, -1, 0}, {0, 1}} // Top-left
    };

    // Define indices for each face (two triangles per face)
    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0, // Front
        4, 5, 6, 6, 7, 4, // Back
        8, 9, 10, 10, 11, 8, // Left
        12, 13, 14, 14, 15, 12, // Right
        16, 17, 18, 18, 19, 16, // Top
        20, 21, 22, 22, 23, 20 // Bottom
    };

    // Calculate tangent and bitangent for each face
    for (int i = 0; i < indices.size(); i += 3) {
        Mesh::Vertex& v0 = vertices[indices[i]];
        Mesh::Vertex& v1 = vertices[indices[i + 1]];
        Mesh::Vertex& v2 = vertices[indices[i + 2]];

        glm::vec3 deltaPos1 = v1.position - v0.position;
        glm::vec3 deltaPos2 = v2.position - v0.position;

        glm::vec2 deltaUV1 = v1.texCoords - v0.texCoords;
        glm::vec2 deltaUV2 = v2.texCoords - v0.texCoords;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent = f * (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y);
        glm::vec3 bitangent = f * (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x);

        v0.tangent = tangent;
        v1.tangent = tangent;
        v2.tangent = tangent;

        v0.bitangent = bitangent;
        v1.bitangent = bitangent;
        v2.bitangent = bitangent;
    }

    verticesData->vertices = vertices;
    verticesData->indices = indices;
    return createMesh(verticesData, "Cube");
}

std::shared_ptr<Prisma::Node> Prisma::GUI::NodeCreator::createSphere(int subDivisions) {
    auto verticesData = std::make_shared<Mesh::VerticesData>();
    float PI = 3.14159265358979323846;
    float radius = 1.0f;

    std::vector<Mesh::Vertex> vertices;
    std::vector<unsigned int> indices;

for (int lat = 0; lat <= subDivisions; ++lat) {
        float theta = lat * PI / subDivisions;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int lon = 0; lon <= subDivisions; ++lon) {
            float phi = lon * 2.0f * PI / subDivisions;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            Mesh::Vertex vertex;

            // Position
            float x = radius * sinTheta * cosPhi;
            float y = radius * cosTheta;
            float z = radius * sinTheta * sinPhi;
            vertex.position = glm::vec3(x, y, z);

            // Normal
            vertex.normal = glm::normalize(vertex.position);

            // Texture Coordinates
            if (lat == 0) {
                vertex.texCoords = glm::vec2(0.5f, 1.0f);  // top pole
            } else if (lat == subDivisions) {
                vertex.texCoords = glm::vec2(0.5f, 0.0f);  // bottom pole
            } else {
                vertex.texCoords = glm::vec2(static_cast<float>(lon) / subDivisions, 1.0f - static_cast<float>(lat) / subDivisions);
            }

            vertices.push_back(vertex);
        }
    }

    // Index generation
    for (int lat = 0; lat < subDivisions; ++lat) {
        for (int lon = 0; lon < subDivisions; ++lon) {
            int current = lat * (subDivisions + 1) + lon;
            int next = current + subDivisions + 1;

            // Triangle 1
            indices.push_back(current);
            indices.push_back(current + 1);
            indices.push_back(next);

            // Triangle 2
            indices.push_back(next);
            indices.push_back(current + 1);
            indices.push_back(next + 1);
        }
    }

    // Tangent and Bitangent Calculation
    for (size_t i = 0; i < indices.size(); i += 3) {
        Mesh::Vertex& v0 = vertices[indices[i]];
        Mesh::Vertex& v1 = vertices[indices[i + 1]];
        Mesh::Vertex& v2 = vertices[indices[i + 2]];

        glm::vec3 deltaPos1 = v1.position - v0.position;
        glm::vec3 deltaPos2 = v2.position - v0.position;

        glm::vec2 deltaUV1 = v1.texCoords - v0.texCoords;
        glm::vec2 deltaUV2 = v2.texCoords - v0.texCoords;

        float denom = deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y;
        if (fabs(denom) < 1e-6f) continue;  // skip degenerate UVs

        float f = 1.0f / denom;

        glm::vec3 tangent = f * (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y);
        glm::vec3 bitangent = f * (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x);

        tangent = glm::normalize(tangent);
        bitangent = glm::normalize(bitangent);

        v0.tangent = tangent;
        v1.tangent = tangent;
        v2.tangent = tangent;

        v0.bitangent = bitangent;
        v1.bitangent = bitangent;
        v2.bitangent = bitangent;
    }

    verticesData->vertices = vertices;
    verticesData->indices = indices;

    return createMesh(verticesData, "Sphere");
}

std::shared_ptr<Prisma::Node> Prisma::GUI::NodeCreator::createOmnidirectional() {
    auto parent = std::make_shared<Node>();
    parent->name("ParentOmnidirectional_" + std::to_string(parent->uuid()));
    auto light = std::make_shared<Light<LightType::LightOmni>>();
    light->name("Omnidirectional_" + std::to_string(light->uuid()));
    light->createShadow(Define::MAX_SHADOW_OMNI_TEXTURE_SIZE, Define::MAX_SHADOW_OMNI_TEXTURE_SIZE);
    parent->addChild(light);
    GlobalData::getInstance().currentGlobalScene()->root->addChild(parent);
    return light;
}

std::shared_ptr<Prisma::Node> Prisma::GUI::NodeCreator::createDirectional() {
    auto parent = std::make_shared<Node>();
    parent->name("ParentDirectional_" + std::to_string(parent->uuid()));
    auto light = std::make_shared<Light<LightType::LightDir>>();
    light->name("Directional" + std::to_string(light->uuid()));
    light->createShadow(Define::MAX_SHADOW_DIR_TEXTURE_SIZE, Define::MAX_SHADOW_DIR_TEXTURE_SIZE);
    parent->addChild(light);
    GlobalData::getInstance().currentGlobalScene()->root->addChild(parent);
    return light;
}

std::shared_ptr<Prisma::Node> Prisma::GUI::NodeCreator::createArea() {
    auto verticesData = std::make_shared<Mesh::VerticesData>();

    std::vector<Mesh::Vertex> vertices = {
        // Front face
        {{-1, -1, 0.0001}, {0, 0, 1}, {0, 0}}, // Bottom-left
        {{1, -1, 0.0001}, {0, 0, 1}, {1, 0}}, // Bottom-right
        {{1, 1, 0.0001}, {0, 0, 1}, {1, 1}}, // Top-right
        {{-1, 1, 0.0001}, {0, 0, 1}, {0, 1}}, // Top-left

        // Back face
        {{1, -1, -0.0001}, {0, 0, -1}, {0, 0}}, // Bottom-left
        {{-1, -1, -0.0001}, {0, 0, -1}, {1, 0}}, // Bottom-right
        {{-1, 1, -0.0001}, {0, 0, -1}, {1, 1}}, // Top-right
        {{1, 1, -0.0001}, {0, 0, -1}, {0, 1}}, // Top-left

        // Left face
        {{-1, -1, -0.0001}, {-1, 0, 0}, {0, 0}}, // Bottom-left
        {{-1, -1, 0.0001}, {-1, 0, 0}, {1, 0}}, // Bottom-right
        {{-1, 1, 0.0001}, {-1, 0, 0}, {1, 1}}, // Top-right
        {{-1, 1, -0.0001}, {-1, 0, 0}, {0, 1}}, // Top-left

        // Right face
        {{1, -1, 0.0001}, {1, 0, 0}, {0, 0}}, // Bottom-left
        {{1, -1, -0.0001}, {1, 0, 0}, {1, 0}}, // Bottom-right
        {{1, 1, -0.0001}, {1, 0, 0}, {1, 1}}, // Top-right
        {{1, 1, 0.0001}, {1, 0, 0}, {0, 1}}, // Top-left

        // Top face
        {{-1, 1, 0.0001}, {0, 1, 0}, {0, 0}}, // Bottom-left
        {{1, 1, 0.0001}, {0, 1, 0}, {1, 0}}, // Bottom-right
        {{1, 1, -0.0001}, {0, 1, 0}, {1, 1}}, // Top-right
        {{-1, 1, -0.0001}, {0, 1, 0}, {0, 1}}, // Top-left

        // Bottom face
        {{-1, -1, -0.0001}, {0, -1, 0}, {0, 0}}, // Bottom-left
        {{1, -1, -0.0001}, {0, -1, 0}, {1, 0}}, // Bottom-right
        {{1, -1, 0.0001}, {0, -1, 0}, {1, 1}}, // Top-right
        {{-1, -1, 0.0001}, {0, -1, 0}, {0, 1}} // Top-left
    };

    // Define indices for each face (two triangles per face)
    std::vector<unsigned int> indices = {
        0, 1, 2, 2, 3, 0, // Front
        4, 5, 6, 6, 7, 4, // Back
        8, 9, 10, 10, 11, 8, // Left
        12, 13, 14, 14, 15, 12, // Right
        16, 17, 18, 18, 19, 16, // Top
        20, 21, 22, 22, 23, 20 // Bottom
    };

    // Calculate tangent and bitangent for each face
    for (int i = 0; i < indices.size(); i += 3) {
        Mesh::Vertex& v0 = vertices[indices[i]];
        Mesh::Vertex& v1 = vertices[indices[i + 1]];
        Mesh::Vertex& v2 = vertices[indices[i + 2]];

        glm::vec3 deltaPos1 = v1.position - v0.position;
        glm::vec3 deltaPos2 = v2.position - v0.position;

        glm::vec2 deltaUV1 = v1.texCoords - v0.texCoords;
        glm::vec2 deltaUV2 = v2.texCoords - v0.texCoords;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        glm::vec3 tangent = f * (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y);
        glm::vec3 bitangent = f * (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x);

        v0.tangent = tangent;
        v1.tangent = tangent;
        v2.tangent = tangent;

        v0.bitangent = bitangent;
        v1.bitangent = bitangent;
        v2.bitangent = bitangent;
    }

    verticesData->vertices = vertices;
    verticesData->indices = indices;

    auto newInstance = std::make_shared<Mesh>();
    newInstance->loadModel(verticesData);
    newInstance->matrix(glm::mat4(1.0));
    newInstance->name("PlaneMesh" + std::to_string(newInstance->uuid()));
    auto whiteMaterial = getEmptyMaterial();
    whiteMaterial->color(glm::vec4(1.0));
    whiteMaterial->plain(true);
    newInstance->material(whiteMaterial);
    auto parent = std::make_shared<Node>();
    parent->name("ParentArea_" + std::to_string(parent->uuid()));
    auto light = std::make_shared<Light<LightType::LightArea>>();
    light->name("Area" + std::to_string(light->uuid()));
    parent->addChild(light);
    parent->addChild(newInstance);
    GlobalData::getInstance().currentGlobalScene()->root->addChild(parent);
    return light;
}

std::shared_ptr<Prisma::Node> Prisma::GUI::NodeCreator::createNode() {
    auto newInstance = std::make_shared<Node>();
    newInstance->matrix(glm::mat4(1.0));
    newInstance->name("Node" + std::to_string(newInstance->uuid()));
    GlobalData::getInstance().currentGlobalScene()->root->addChild(newInstance);
    return newInstance;
}

std::shared_ptr<Prisma::Node> Prisma::GUI::NodeCreator::createMesh(std::shared_ptr<Mesh::VerticesData> verticesData,
                                                                   const std::string& name) {
    auto newInstance = std::make_shared<Mesh>();
    newInstance->loadModel(verticesData);
    auto currentMaterial = getEmptyMaterial();
    newInstance->material(currentMaterial);
    newInstance->matrix(glm::mat4(1.0));
    newInstance->name(name + "Mesh" + std::to_string(newInstance->uuid()));
    auto parent = std::make_shared<Node>();
    parent->name(name + "Parent" + std::to_string(parent->uuid()));
    parent->matrix(glm::mat4(1.0));
    GlobalData::getInstance().currentGlobalScene()->root->addChild(parent);
    parent->addChild(newInstance);
    return parent;
}

std::shared_ptr<Prisma::MaterialComponent> Prisma::GUI::NodeCreator::getEmptyMaterial() {
    auto currentMaterial = std::make_shared<MaterialComponent>();
    std::vector<Texture> emptyVector;
    emptyVector.clear();
    emptyVector.push_back(GlobalData::getInstance().defaultBlack());
    currentMaterial->diffuse(emptyVector);
    emptyVector.clear();
    emptyVector.push_back(GlobalData::getInstance().defaultNormal());
    currentMaterial->normal(emptyVector);
    emptyVector.clear();
    emptyVector.push_back(GlobalData::getInstance().defaultRoughness());
    currentMaterial->roughnessMetalness(emptyVector);
    emptyVector.clear();
    emptyVector.push_back(GlobalData::getInstance().defaultWhite());
    currentMaterial->specular(emptyVector);
    emptyVector.clear();
    emptyVector.push_back(GlobalData::getInstance().defaultWhite());
    currentMaterial->ambientOcclusion(emptyVector);
    return currentMaterial;
}