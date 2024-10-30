#include "../../include/Physics/Line.h"

Prisma::Line::Line()
{
	m_lineColor = glm::vec3(1, 1, 1);
	m_MVP = glm::mat4(1.0f);

	m_shader = std::make_shared<Shader>("../../../Engine/Shaders/LineDebugPipeline/vertex.glsl",
	                                    "../../../Engine/Shaders/LineDebugPipeline/fragment.glsl");
	m_shader->use();
	m_colorPos = m_shader->getUniformPosition("color");
	m_MVPPos = m_shader->getUniformPosition("MVP");
}

int Prisma::Line::setMVP(glm::mat4 mvp)
{
	m_MVP = mvp;
	return 1;
}

int Prisma::Line::setColor(glm::vec3 color)
{
	m_lineColor = color;
	return 1;
}

int Prisma::Line::draw(glm::vec3 start, glm::vec3 end)
{
	m_shader->use();
	m_shader->setMat4(m_MVPPos, m_MVP);
	m_shader->setVec3(m_colorPos, m_lineColor);
	m_vertices = {
		start.x, start.y, start.z,
		end.x, end.y, end.z,
	};
	glDeleteBuffers(1, &m_VBO);
	glDeleteVertexArrays(1, &m_VAO);
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices), m_vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), static_cast<void*>(nullptr));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindVertexArray(m_VAO);
	glDrawArrays(GL_LINES, 0, 2);
	return 1;
}

Prisma::Line::~Line()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
}
