#include "../include/ImGuiSelector.h"

Prisma::ImGuiSelector::ImGuiSelector() {
    m_screenData=std::make_shared<ScreenData>();
}

std::shared_ptr<Prisma::ImGuiSelector::RayCastResult> Prisma::ImGuiSelector::raycastWorld(const glm::vec3 start, glm::vec3 end) {
    auto meshes=currentGlobalScene->meshes;
    Mesh* correctMesh=nullptr;
    std::vector<std::pair<Mesh*,glm::vec3>> meshFound;

    for(auto mesh : meshes){
        auto model=mesh->finalMatrix();
        auto hitMesh=rayAABBIntersect(start,end,glm::vec3(model*glm::vec4(mesh->aabbData().min,1)),glm::vec3(model*glm::vec4(mesh->aabbData().max,1)));
        if(hitMesh.hit){
            meshFound.push_back({mesh.get(),hitMesh.point});
        }
    }

    if(meshFound.size()>0) {
        float minDistance = glm::length(meshFound[0].second-start);
        correctMesh=meshFound[0].first;
        for (auto mesh: meshFound) {
            auto currentLength=glm::length(mesh.second-start);
            if(minDistance>=currentLength){
                correctMesh=mesh.first;
                minDistance=currentLength;
            }
        }
    }

    if (correctMesh) {
        auto result=std::make_shared<RayCastResult>();
        result->other=(Prisma::Mesh*)correctMesh;
        return result;
    }
    return nullptr;
}

std::pair<glm::vec3, glm::vec3> Prisma::ImGuiSelector::castRayFromMouse(double mouseX, double mouseY) {
    // Normalize mouse coordinates
    float x = (2.0f * mouseX) / m_screenData->width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / m_screenData->height;
    float z = 1.0f;

    // Clip space to view space
    glm::vec4 rayClip(x, y, -1.0, 1.0);
    glm::vec4 rayEye = glm::inverse(m_screenData->projectionMatrix) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0, 0.0);

    // View space to world space
    glm::vec4 rayWorld = glm::inverse(m_screenData->viewMatrix) * rayEye;
    glm::vec3 rayDir = glm::normalize(glm::vec3(rayWorld));

    // Origin of the ray is the camera position
    glm::vec3 rayOrigin = glm::vec3(m_screenData->viewMatrix[3]);

    return std::make_pair(rayOrigin, rayDir);
}
