#pragma once
#include <string>

namespace Prisma::ShaderNames {
const std::string CONSTANT_OMNI_DATA = "omniData";
const std::string CONSTANT_ANIMATION = "animations";
const std::string MUTABLE_OMNI_DATA_SHADOW = "omniShadow";
const std::string STATIC_DIR_SHADOW = "csmShadow";
const std::string MUTABLE_INDEX_OPAQUE = "opaqueIndices";
const std::string MUTABLE_INDEX_TRANSPARENT = "transparentIndices";
const std::string CONSTANT_DIR_DATA_SHADOW = "LightSpaceMatrices";
const std::string CONSTANT_DIR_DATA = "dirData";
const std::string CONSTANT_VIEW_PROJECTION = "ViewProjection";
const std::string CONSTANT_LIGHT_SIZES = "LightSizes";
const std::string MUTABLE_DIFFUSE_TEXTURE = "diffuseTexture";
const std::string MUTABLE_DIFFUSE_TEXTURE_ANIMATION = "diffuseTextureAnimation";
const std::string MUTABLE_NORMAL_TEXTURE = "normalTexture";
const std::string MUTABLE_NORMAL_TEXTURE_ANIMATION = "normalTextureAnimation";
const std::string MUTABLE_ROUGHNESS_METALNESS_TEXTURE = "rmTexture";
const std::string MUTABLE_ROUGHNESS_METALNESS_TEXTURE_ANIMATION = "rmTextureAnimation";
const std::string MUTABLE_SPECULAR_TEXTURE = "specularTexture";
const std::string MUTABLE_SPECULAR_TEXTURE_ANIMATION = "specularTextureAnimation";
const std::string MUTABLE_AMBIENT_OCCLUSION_TEXTURE = "aoTexture";
const std::string MUTABLE_MODELS = "models";
const std::string CONSTANT_LUT = "lut";
const std::string MUTABLE_IRRADIANCE = "irradiance";
const std::string MUTABLE_PREFILTER = "prefilter";
const std::string CONSTANT_CLUSTERS = "clusters";
const std::string CONSTANT_CLUSTERS_DATA = "ConstantsClusters";
const std::string MUTABLE_STATUS = "statusData";
const std::string MUTABLE_STATUS_ANIMATION = "statusDataAnimation";
}