#pragma once
#include <string>
#include "glm/glm.hpp"

const int MAX_DIR_LIGHTS = 1024;
const int MAX_OMNI_LIGHTS = 1024;
const int MAX_CLUSTER_SIZE = 100;
const int MAX_BONE_INFLUENCE = 4;
const int MAX_BONES = 128;
const int MAX_ANIMATION_MESHES = 1024;
const int MAX_SHADOW_OMNI = 1024;
const int MAX_SHADOW_DIR = 4096;
const std::string DIR_DEFAULT_BLACK = "../../../Resources/res/black.png";
const std::string DIR_DEFAULT_SCENE = "../../../Resources/BasicScene/BasicScene.gltf";
const std::string DIR_DEFAULT_NORMAL = "../../../Resources/res/normal.jpg";
const std::string DIR_DEFAULT_SETTINGS = "../../../Resources/configuration/settings.json";
const glm::vec4 CLEAR_COLOR = glm::vec4(0.0f);