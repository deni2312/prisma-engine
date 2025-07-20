#include "Components/MaterialComponent.h"
#include "GlobalData/GlobalData.h"
#include "GlobalData/CacheScene.h"
#include "GlobalData/GlobalShaderNames.h"
#include "Helpers/WindowsHelper.h"
#include "Pipelines/PipelineHandler.h"
#include "ThirdParty/imgui/imgui.h"


static unsigned int materialId = 0;

void Prisma::MaterialComponent::diffuse(std::vector<Texture> diffuse) {
    m_diffuse = diffuse;
    CacheScene::getInstance().updateTextures(true);
}

void Prisma::MaterialComponent::normal(std::vector<Texture> normal) {
    m_normal = normal;
    CacheScene::getInstance().updateTextures(true);
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::diffuse() {
    return m_diffuse;
}


void Prisma::MaterialComponent::ui() {
    Component::ui();

    auto getLast = [](std::string s) {
        size_t found = s.find_last_of('/');
        return found != std::string::npos ? s.substr(found + 1) : s;
    };

    m_diffuseImage.handler = [&]() {
        std::vector<Texture> diffuseTextures;
        Texture texture;
        auto openFolder = WindowsHelper::getInstance().openFolder("All Files");
        if (!openFolder.empty()) {
            texture.loadTexture({openFolder, m_loadSrgb, Define::DEFAULT_MIPS, m_loadCompress});
            texture.name(openFolder);
            diffuseTextures.push_back(texture);
            diffuse(diffuseTextures);
            auto data = static_cast<ImageButton*>(std::get<2>(m_globalVars[0].type));
            data->texture = m_diffuse[0];
        }
    };

    m_normalImage.handler = [&]() {
        std::vector<Texture> normalTextures;
        Texture texture;
        auto openFolder = WindowsHelper::getInstance().openFolder("All Files");
        if (!openFolder.empty()) {
            texture.loadTexture({openFolder, m_loadSrgb, Define::DEFAULT_MIPS, m_loadCompress});
            texture.name(openFolder);
            normalTextures.push_back(texture);
            normal(normalTextures);
            auto data = static_cast<ImageButton*>(std::get<2>(m_globalVars[1].type));
            data->texture = m_normal[0];
        }
    };
     m_metalnessRoughnessImage.handler = [&]() {
        std::vector<Texture> metalnessRoughnessTextures;
        Texture texture;
        auto openFolder = WindowsHelper::getInstance().openFolder("All Files");
        if (!openFolder.empty()) {
            texture.loadTexture({openFolder, m_loadSrgb, Define::DEFAULT_MIPS, m_loadCompress});
            texture.name(openFolder);
            metalnessRoughnessTextures.push_back(texture);
            roughnessMetalness(metalnessRoughnessTextures);
            auto data = static_cast<ImageButton*>(std::get<2>(m_globalVars[2].type));
            data->texture = m_roughnessMetalness[0];
        }
    };
    m_specularImage.handler = [&]() {
        std::vector<Texture> specularTextures;
        Texture texture;
        auto openFolder = WindowsHelper::getInstance().openFolder("All Files");
        if (!openFolder.empty()) {
            texture.loadTexture({openFolder, m_loadSrgb, Define::DEFAULT_MIPS, m_loadCompress});
            texture.name(openFolder);
            specularTextures.push_back(texture);
            specular(specularTextures);
            auto data = static_cast<ImageButton*>(std::get<2>(m_globalVars[3].type));
            data->texture = m_specular[0];
        }
    };
     m_ambientOcclusionImage.handler = [&]() {
        std::vector<Texture> ambientOcclusionTextures;
        Texture texture;
        auto openFolder = WindowsHelper::getInstance().openFolder("All Files");
        if (!openFolder.empty()) {
            texture.loadTexture({openFolder, m_loadSrgb, Define::DEFAULT_MIPS, m_loadCompress});
            texture.name(openFolder);
            ambientOcclusionTextures.push_back(texture);
            ambientOcclusion(ambientOcclusionTextures);
            auto data = static_cast<ImageButton*>(std::get<2>(m_globalVars[4].type));
            data->texture = m_ambientOcclusion[0];
        }
    };

     
    auto hover = [&](const std::string& type) {
         return [=]() {
             std::vector<Prisma::Texture> textures;
             if (type == "diffuse") {
                 textures = diffuse();
             }
             if (type == "normal") {
                 textures = normal();
             }
             if (type == "metalness roughness") {
                 textures = roughnessMetalness();
             }
             if (type == "specular") {
                 textures = specular();
             }
             if (type == "ambient occlusion") {
                 textures = ambientOcclusion();
             }


             std::string typeText = type;
             ImGui::BeginTooltip();
             ImGui::Text(("Type: " + typeText).c_str());
             ImGui::Text(("Name: " + textures[0].name()).c_str());
             ImGui::Text(("Size: " + std::to_string(textures[0].data().width) + "x" + std::to_string(textures[0].data().height)).c_str());
             ImGui::Text("Srgb: %s", textures[0].parameters().srgb ? "Yes" : "No");
             ImGui::Text("Compress: %s", textures[0].parameters().compress ? "Yes" : "No");

             ImGui::EndTooltip();
         };
     };

    m_diffuseImage.hover = hover("diffuse");
    m_normalImage.hover = hover("normal");
    m_metalnessRoughnessImage.hover = hover("metalness roughness");
    m_specularImage.hover = hover("specular");
    m_ambientOcclusionImage.hover = hover("ambient occlusion");


    if (m_diffuse.size() > 0) {
        m_diffuseName = std::make_shared<std::string>(getLast(m_diffuse[0].name()));
        m_componentTypeDiffuse = std::make_tuple(TYPES::TEXTURE_BUTTON, "Diffuse", &m_diffuseImage);
        m_diffuseImage.texture = m_diffuse[0];

        addGlobal({m_componentTypeDiffuse, true, m_size});
    }

    if (m_normal.size() > 0) {
        m_normalName = std::make_shared<std::string>(getLast(m_normal[0].name()));

        m_componentTypeNormal = std::make_tuple(TYPES::TEXTURE_BUTTON, "Normal", &m_normalImage);
        m_normalImage.texture = m_normal[0];

        addGlobal({m_componentTypeNormal, false, m_size});
    }

    if (m_roughnessMetalness.size() > 0) {
        m_metalness_roughnessName = std::make_shared<std::string>(getLast(m_roughnessMetalness[0].name()));

        m_componentTypeMetalnessRoughness = std::make_tuple(TYPES::TEXTURE_BUTTON,
                                                            std::to_string(m_roughnessMetalness[0].rawId()),
                                                            &m_metalnessRoughnessImage);

        m_metalnessRoughnessImage.texture = m_roughnessMetalness[0];

        addGlobal({m_componentTypeMetalnessRoughness, true, m_size});
    }

    if (m_specular.size() > 0) {
        m_specularName = std::make_shared<std::string>(getLast(m_specular[0].name()));
        m_componentTypeSpecular = std::make_tuple(TYPES::TEXTURE_BUTTON, std::to_string(m_specular[0].rawId()),
                                                  &m_specularImage);
        m_specularImage.texture = m_specular[0];
        addGlobal({m_componentTypeSpecular, false, m_size});
    }

    if (m_ambientOcclusion.size() > 0) {
        m_ambientOcclusionName = std::make_shared<std::string>(getLast(m_ambientOcclusion[0].name()));
        m_componentTypeAmbientOcclusion = std::make_tuple(TYPES::TEXTURE_BUTTON,
                                                          std::to_string(m_ambientOcclusion[0].rawId()),
                                                          &m_ambientOcclusionImage);
        m_ambientOcclusionImage.texture = m_ambientOcclusion[0];
        addGlobal({m_componentTypeAmbientOcclusion, false, m_size});
    }

    ComponentType componentPlain;
    componentPlain = std::make_tuple(TYPES::BOOL, "Plain", &m_plain);

    ComponentType componentTransparent;
    componentTransparent = std::make_tuple(TYPES::BOOL, "Transparent", &m_transparent);

    ComponentType componentColor;
    componentColor = std::make_tuple(TYPES::COLOR, "Color", &m_color);

    // Refraction cube properties
    auto GlassReflectionColorMask = glm::vec3(0.22f, 0.83f, 0.93f);
    float GlassAbsorption = 0.5;
    auto GlassMaterialColor = glm::vec4(1);
    auto GlassIndexOfRefraction = glm::vec2(1.5f, 1.02f); // min and max IOR
    int GlassEnableDispersion = 0;
    unsigned int DispersionSampleCount = 4; // 1..16

    ComponentType componentGlassReflectionColorMask;
    componentGlassReflectionColorMask = std::make_tuple(TYPES::VEC3, "GlassReflectionColorMask",
                                                        &m_rtMaterial.GlassReflectionColorMask);

    ComponentType componentSrgb;
    componentSrgb = std::make_tuple(TYPES::BOOL, "Srgb", &m_loadSrgb);

    ComponentType componentCompress;
    componentCompress = std::make_tuple(TYPES::BOOL, "Compress", &m_loadCompress);

    ComponentType componentGlassAbsorption;
    componentGlassAbsorption = std::make_tuple(TYPES::FLOAT, "GlassAbsorption", &m_rtMaterial.GlassAbsorption);

    ComponentType componentGlassMaterialColor;
    componentGlassMaterialColor = std::make_tuple(TYPES::VEC3, "GlassMaterialColor",
                                                  &m_rtMaterial.GlassMaterialColor);

    ComponentType componentGlassIndexOfRefraction;
    componentGlassIndexOfRefraction = std::make_tuple(TYPES::VEC2, "GlassIndexOfRefraction",
                                                      &m_rtMaterial.GlassIndexOfRefraction);

    ComponentType componentGlassEnableDispersion;
    componentGlassEnableDispersion = std::make_tuple(TYPES::BOOL, "GlassEnableDispersion",
                                                     &m_rtMaterial.GlassEnableDispersion);

    ComponentType componentDispersionSampleCount;
    componentDispersionSampleCount = std::make_tuple(TYPES::INT, "DispersionSampleCount",
                                                     &m_rtMaterial.DispersionSampleCount);

    ComponentType componentMetalness;
    componentMetalness = std::make_tuple(TYPES::FLOAT, "MetalnessData", &m_metalness);

    ComponentType componentRoughness;
    componentRoughness = std::make_tuple(TYPES::FLOAT, "RoughnessData", &m_roughness);

    m_apply = []() {
        CacheScene::getInstance().updateTextures(true);
        CacheScene::getInstance().updateLights(true);
        CacheScene::getInstance().updateStatus(true);
    };

    ComponentType componentButton;
    componentButton = std::make_tuple(TYPES::BUTTON, "Apply", &m_apply);

    addGlobal({componentPlain, false});
    addGlobal({componentSrgb, false});
    addGlobal({componentCompress, false});
    addGlobal({componentTransparent, false});
    addGlobal({componentColor, false});
    addGlobal({componentGlassReflectionColorMask, false});
    addGlobal({componentGlassAbsorption, false});
    addGlobal({componentGlassMaterialColor, false});
    addGlobal({componentGlassIndexOfRefraction, false});
    addGlobal({componentGlassEnableDispersion, false});
    addGlobal({componentDispersionSampleCount, false});
    addGlobal({componentMetalness, false});
    addGlobal({componentRoughness, false});
    addGlobal({componentButton, false});

    uiRemovable(false);
}

void Prisma::MaterialComponent::update() {
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::normal() {
    return m_normal;
}

void Prisma::MaterialComponent::roughnessMetalness(std::vector<Texture> roughnessMetalness) {
    m_roughnessMetalness = roughnessMetalness;
    CacheScene::getInstance().updateTextures(true);
}

void Prisma::MaterialComponent::specular(std::vector<Texture> specular) {
    m_specular = specular;
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::specular() {
    return m_specular;
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::roughnessMetalness() {
    return m_roughnessMetalness;
}

void Prisma::MaterialComponent::ambientOcclusion(std::vector<Texture> ambientOcclusion) {
    m_ambientOcclusion = ambientOcclusion;
}

std::vector<Prisma::Texture>& Prisma::MaterialComponent::ambientOcclusion() {
    return m_ambientOcclusion;
}

unsigned int Prisma::MaterialComponent::material_id() {
    return m_id;
}

void Prisma::MaterialComponent::material_name(std::string name) {
    m_materialName = name;
}

Prisma::MaterialComponent::MaterialComponent() {
    name("Material Component");
    m_id = materialId;
    materialId++;
}

std::string Prisma::MaterialComponent::material_name() {
    return m_materialName;
}

void Prisma::MaterialComponent::transparent(bool transparent) {
    m_transparent = transparent;
    CacheScene::getInstance().updateStatus(true);
}

bool Prisma::MaterialComponent::transparent() const {
    return m_transparent;
}

void Prisma::MaterialComponent::color(glm::vec4 color) {
    m_color = color;
    CacheScene::getInstance().updateTextures(true);
}

glm::vec4 Prisma::MaterialComponent::color() const {
    return m_color;
}

void Prisma::MaterialComponent::plain(bool plain) {
    m_plain = plain;
    CacheScene::getInstance().updateStatus(true);
}

bool Prisma::MaterialComponent::plain() {
    return m_plain;
}

void Prisma::MaterialComponent::rtMaterial(RayTracingMaterial rtMaterial) {
    m_rtMaterial = rtMaterial;
}

Prisma::MaterialComponent::RayTracingMaterial Prisma::MaterialComponent::rtMaterial() {
    return m_rtMaterial;
}

void Prisma::MaterialComponent::isSpecular(bool specular) {
    m_isSpecular = specular;
}

bool Prisma::MaterialComponent::isSpecular() {
    return m_isSpecular;
}

void Prisma::MaterialComponent::roughness(float roughness) {
    m_roughness = roughness;
}

float Prisma::MaterialComponent::roughness() const {
    return m_roughness;
}

void Prisma::MaterialComponent::metalness(float metalness) {
    m_metalness = metalness;
}

float Prisma::MaterialComponent::metalness() const {
    return m_metalness;
}