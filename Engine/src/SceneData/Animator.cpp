#include "SceneData/Animator.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/CacheScene.h"
#include <glm/gtx/string_cast.hpp>


Prisma::Animator::Animator(std::shared_ptr<Animation> animation) {
        m_CurrentTime = 0.0;
        m_CurrentAnimation = animation;
        CacheScene::getInstance().updateShadows(true);
}

void Prisma::Animator::updateAnimation(float dt) {
        if (m_CurrentAnimation) {
                CacheScene::getInstance().updateShadows(true);
                m_CurrentTime += m_CurrentAnimation->ticksPerSecond() * dt;
                m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->duration());

                // Perform the blend if in blending mode
                if (m_IsBlending && m_PreviousAnimation) {
                        // Increase blend factor over time
                        m_BlendFactor += dt / m_BlendDuration;

                        if (m_BlendFactor >= 1.0f) {
                                // Stop blending when blend factor reaches 1.0
                                m_BlendFactor = 1.0f;
                                m_IsBlending = false;
                                m_PreviousAnimation = nullptr;
                        }

                        // Calculate and blend bone transforms
                        blendAnimations(m_BlendFactor);
                } else {
                        // No blending, use current animation directly
                        calculateBoneTransform(&m_CurrentAnimation->rootNode(), glm::mat4(1.0f));
                }
                if (m_automatic) {
                        updateSSBO();
                }
        }
}

void Prisma::Animator::playAnimation(std::shared_ptr<Animation> pAnimation, float blendDuration) {
        CacheScene::getInstance().updateShadows(true);

        m_CurrentTime = 0.0f;
        m_PreviousAnimation = m_CurrentAnimation;
        m_CurrentAnimation = pAnimation;
        m_CurrentTime = 0.0f;
        m_BlendFactor = 0.0f;
        calculateCurrentTransform(&m_CurrentAnimation->rootNode(), glm::mat4(1.0f));
        auto transform = m_ssboAnimation;
        for (int i = 0; i < Define::MAX_BONES; i++) {
                m_currentTransform[i] = transform.animations[i];
        }
        m_BlendDuration = blendDuration;
        m_IsBlending = true;
        if (m_automatic) {
                updateSSBO();
        }
}

void Prisma::Animator::calculateBoneTransform(const AssimpNodeData* node, const glm::mat4& parentTransform) {
        const std::string& nodeName = node->name;
        glm::mat4 nodeTransform;

        auto Bone = m_CurrentAnimation->FindBone(nodeName);

        if (Bone) {
                Bone->Update(m_CurrentTime);
                nodeTransform = Bone->GetLocalTransform();
        } else {
                nodeTransform = node->transformation;
        }
        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        const auto& boneInfoMap = m_CurrentAnimation->boneIdMap();
        auto it = boneInfoMap->find(nodeName);
        if (it != boneInfoMap->end()) {
                const auto& boneInfo = it->second;
                m_ssboAnimation.animations[boneInfo.id] = globalTransformation * boneInfo.offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
                calculateBoneTransform(&node->children[i], globalTransformation);
}

void Prisma::Animator::frame(float frame) {
        if (m_CurrentAnimation) {
                CacheScene::getInstance().updateShadows(true);
                m_CurrentTime = frame;
                m_CurrentTime = fmod(m_CurrentTime, m_CurrentAnimation->duration());
                calculateBoneTransform(&m_CurrentAnimation->rootNode(), glm::mat4(1.0f));
                if (m_automatic) {
                        updateSSBO();
                }
        }
}

void Prisma::Animator::mesh(Node* mesh) {
        m_mesh = mesh;
        updateAnimation(0);
}

std::shared_ptr<Prisma::Animation> Prisma::Animator::animation() {
        return m_CurrentAnimation;
}

float Prisma::Animator::currentTime() const {
        return m_CurrentTime;
}

void Prisma::Animator::updateSSBO() {
        //auto ssbo = AnimationHandler::getInstance().ssbo();
        //auto uuid = findUUID();
        //ssbo->modifyData(sizeof(Prisma::AnimationHandler::SSBOAnimation) * uuid,
        //                 sizeof(Prisma::AnimationHandler::SSBOAnimation), &m_ssboAnimation);
        auto& contextData = PrismaFunc::getInstance().contextData();
        auto uuid = findUUID();
        contextData.immediateContext->UpdateBuffer(AnimationHandler::getInstance().animation(),
                                                   sizeof(AnimationHandler::SSBOAnimation) * uuid,
                                                   sizeof(AnimationHandler::SSBOAnimation), &m_ssboAnimation,
                                                   Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void Prisma::Animator::automaticUpdate(bool automatic) {
        m_automatic = automatic;
}

bool Prisma::Animator::automaticUpdate() {
        return m_automatic;
}

int Prisma::Animator::findUUID() {
        auto meshes = GlobalData::getInstance().currentGlobalScene()->animateMeshes;
        for (int i = 0; i < meshes.size(); i++) {
                if (meshes[i]->uuid() == m_mesh->uuid()) {
                        return i;
                }
        }
        return 0;
}

void Prisma::Animator::blendAnimations(float blendFactor) {
        for (int i = 0; i < Define::MAX_BONES; i++) {
                m_ssboAnimation.animations[i] = decomposeAndInterpolateMat4(
                        m_currentTransform[i], m_previousTransform[i],
                        blendFactor);
        }
}

void Prisma::Animator::calculateCurrentTransform(const AssimpNodeData* node, const glm::mat4& parentTransform) {
        const std::string& nodeName = node->name;
        glm::mat4 nodeTransform;

        auto Bone = m_CurrentAnimation->FindBone(nodeName);

        if (Bone) {
                Bone->Update(0);
                nodeTransform = Bone->GetLocalTransform();
        } else {
                nodeTransform = node->transformation;
        }
        glm::mat4 globalTransformation = parentTransform * nodeTransform;

        const auto& boneInfoMap = m_CurrentAnimation->boneIdMap();
        auto it = boneInfoMap->find(nodeName);
        if (it != boneInfoMap->end()) {
                const auto& boneInfo = it->second;
                m_previousTransform[boneInfo.id] = globalTransformation * boneInfo.offset;
        }

        for (int i = 0; i < node->childrenCount; i++)
                calculateCurrentTransform(&node->children[i], globalTransformation);
}

glm::mat4 Prisma::Animator::decomposeAndInterpolateMat4(const glm::mat4& matA, const glm::mat4& matB, float t) {
        glm::vec3 scaleA, scaleB, translationA, translationB, skewA, skewB;
        glm::vec4 perspectiveA, perspectiveB;
        glm::quat rotationA, rotationB;

        // Decompose both matrices
        decompose(matA, scaleA, rotationA, translationA, skewA, perspectiveA);
        decompose(matB, scaleB, rotationB, translationB, skewB, perspectiveB);

        // Interpolate each component
        glm::vec3 scaleInterp = mix(scaleA, scaleB, t);
        glm::vec3 translationInterp = mix(translationA, translationB, t);
        glm::quat rotationInterp = slerp(rotationA, rotationB, t);

        // Recompose the matrix
        glm::mat4 scaleMatrix = scale(glm::mat4(1.0f), scaleInterp);
        glm::mat4 rotationMatrix = mat4_cast(rotationInterp);
        glm::mat4 translationMatrix = translate(glm::mat4(1.0f), translationInterp);

        return translationMatrix * rotationMatrix * scaleMatrix;
}