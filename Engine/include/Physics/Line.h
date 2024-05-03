#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <gl/glew.h>
#include <string>
#include <iostream>
#include "../Helpers/Shader.h"

class Line {
    unsigned int m_VBO, m_VAO;
    std::vector<float> m_vertices;
    glm::vec3 m_startPoint;
    glm::vec3 m_endPoint;
    glm::mat4 m_MVP;
    glm::vec3 m_lineColor;
    std::shared_ptr<Prisma::Shader> m_shader;
    unsigned int m_colorPos;
    unsigned int m_MVPPos;
public:
    Line();

    int setMVP(glm::mat4 mvp);

    int setColor(glm::vec3 color);

    int draw(glm::vec3 start, glm::vec3 end);

    ~Line();
};