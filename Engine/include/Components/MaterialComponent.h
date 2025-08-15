#pragma once
#include <vector>
#include "../Containers/Texture.h"
#include "Component.h"
#include "../GlobalData/Platform.h"
#include "Common/interface/RefCntAutoPtr.hpp"


namespace Diligent {
struct IPipelineState;
}

namespace Prisma {

class MaterialComponent : public Component {
public:
    struct RayTracingMaterial {
        // Refraction cube properties
        glm::vec3 GlassReflectionColorMask = glm::vec3(0.22f, 0.83f, 0.93f);
        float GlassAbsorption = 0.5;
        glm::vec4 GlassMaterialColor = glm::vec4(1);
        glm::vec2 GlassIndexOfRefraction = glm::vec2(1.5f, 1.02f); // min and max IOR
        int GlassEnableDispersion = 0;
        unsigned int DispersionSampleCount = 4; // 1..16
    };

    void ui() override;

    void update() override;

    void diffuse(std::vector<Texture> diffuse);

    std::vector<Texture>& diffuse();

    void normal(std::vector<Texture> normal);

    std::vector<Texture>& normal();

    void roughnessMetalness(std::vector<Texture> roughnessMetalness);

    std::vector<Texture>& roughnessMetalness();

    void specular(std::vector<Texture> specular);

    std::vector<Texture>& specular();

    void ambientOcclusion(std::vector<Texture> ambientOcclusion);

    std::vector<Texture>& ambientOcclusion();

    unsigned int material_id();

    void material_name(std::string name);

    MaterialComponent();

    std::string material_name();

    void transparent(bool transparent);

    bool transparent() const;

    void color(glm::vec4 color);

    glm::vec4 color() const;

    void plain(bool plain);

    bool plain();

    void rtMaterial(RayTracingMaterial rtMaterial);

    RayTracingMaterial rtMaterial();

    void isSpecular(bool specular);

    bool isSpecular();

    void roughness(float roughness);

    float roughness() const;

    void metalness(float metalness);

    float metalness() const;

    void emission(float emission);

    float emission() const;

private:
    std::vector<Texture> m_diffuse;
    std::vector<Texture> m_normal;
    std::vector<Texture> m_roughnessMetalness;
    std::vector<Texture> m_specular;
    std::vector<Texture> m_ambientOcclusion;

    std::shared_ptr<std::string> m_diffuseName;
    std::shared_ptr<std::string> m_normalName;
    std::shared_ptr<std::string> m_metalness_roughnessName;
    std::shared_ptr<std::string> m_specularName;
    std::shared_ptr<std::string> m_ambientOcclusionName;

    glm::vec4 m_color = glm::vec4(0);

    bool m_plain = false;

    bool m_transparent = false;

    unsigned int m_id;

    std::string m_materialName;

    ComponentType m_componentTypeDiffuse;
    ComponentType m_componentTypeNormal;
    ComponentType m_componentTypeMetalnessRoughness;
    ComponentType m_componentTypeSpecular;
    ComponentType m_componentTypeAmbientOcclusion;
    std::function<void()> m_apply;

    std::function<void(const std::string&, std::vector<Prisma::Texture>&)> m_hover;

    ImageButton m_diffuseImage;
    ImageButton m_normalImage;
    ImageButton m_metalnessRoughnessImage;
    ImageButton m_specularImage;
    ImageButton m_ambientOcclusionImage;

    RayTracingMaterial m_rtMaterial;

    bool m_isSpecular = false;

    bool m_loadSrgb = false;
    bool m_loadCompress = false;

    glm::vec2 m_size = glm::vec2(90, 90);

    float m_roughness = 0;
    float m_metalness = 0;
    float m_emission = 1;
};
}