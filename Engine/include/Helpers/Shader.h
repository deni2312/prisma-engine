#pragma once
#include "GL/glew.h"
#include "glm/glm.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Shadinclude.h"


namespace Prisma
{
	class Shader
	{
	public:
		struct ShaderHeaders
		{
			std::string vertex = "";
			std::string geometry = "";
			std::string fragment = "";
		};

		unsigned int ID;
		// constructor generates the shader on the fly
		// ------------------------------------------------------------------------
		Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr,
		       ShaderHeaders headers = {"", "", ""}, const char* tessControlPath = nullptr,
		       const char* tessEvalPath = nullptr)
		{
			// 1. retrieve the vertex/fragment source code from filePath
			std::string vertexCode;
			std::string fragmentCode;
			std::string geometryCode;
			std::string tessControlCode;
			std::string tessEvalCode;
			// ensure ifstream objects can throw exceptions:
			try
			{
				// convert stream into string
				vertexCode = headers.vertex + Shadinclude::load(vertexPath);
				fragmentCode = headers.fragment + Shadinclude::load(fragmentPath);
				// if geometry shader path is present, also load a geometry shader
				if (geometryPath != nullptr)
				{
					geometryCode = headers.geometry + Shadinclude::load(geometryPath);
				}
				if (tessControlPath != nullptr)
				{
					tessControlCode = Shadinclude::load(tessControlPath);
				}
				if (tessEvalPath != nullptr)
				{
					tessEvalCode = Shadinclude::load(tessEvalPath);
				}
			}
			catch (std::ifstream::failure& e)
			{
				std::cout << vertexPath << std::endl;
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
			}

			std::string shaderName = std::string(vertexPath) + " " + std::string(fragmentPath) + "\n";

			const char* vShaderCode = vertexCode.c_str();
			const char* fShaderCode = fragmentCode.c_str();
			// 2. compile shaders
			unsigned int vertex, fragment;
			// vertex shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, nullptr);
			glCompileShader(vertex);
			checkCompileErrors(vertex, "VERTEX", shaderName);
			// fragment Shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, nullptr);
			glCompileShader(fragment);
			checkCompileErrors(fragment, "FRAGMENT", shaderName);
			// if geometry shader is given, compile geometry shader
			unsigned int geometry;
			if (geometryPath != nullptr)
			{
				const char* gShaderCode = geometryCode.c_str();
				geometry = glCreateShader(GL_GEOMETRY_SHADER);
				glShaderSource(geometry, 1, &gShaderCode, nullptr);
				glCompileShader(geometry);
				checkCompileErrors(geometry, "GEOMETRY");
			}
			// if tessellation shader is given, compile tessellation shader
			unsigned int tessControl;
			if (tessControlPath != nullptr)
			{
				const char* tcShaderCode = tessControlCode.c_str();
				tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
				glShaderSource(tessControl, 1, &tcShaderCode, nullptr);
				glCompileShader(tessControl);
				checkCompileErrors(tessControl, "TESS_CONTROL");
			}
			unsigned int tessEval;
			if (tessEvalPath != nullptr)
			{
				const char* teShaderCode = tessEvalCode.c_str();
				tessEval = glCreateShader(GL_TESS_EVALUATION_SHADER);
				glShaderSource(tessEval, 1, &teShaderCode, nullptr);
				glCompileShader(tessEval);
				checkCompileErrors(tessEval, "TESS_EVALUATION");
			}
			// shader Program
			ID = glCreateProgram();
			glAttachShader(ID, vertex);
			glAttachShader(ID, fragment);
			if (geometryPath != nullptr)
				glAttachShader(ID, geometry);
			if (tessControlPath != nullptr)
				glAttachShader(ID, tessControl);
			if (tessEvalPath != nullptr)
				glAttachShader(ID, tessEval);
			glLinkProgram(ID);
			checkCompileErrors(ID, "PROGRAM");
			// delete the shaders as they're linked into our program now and no longer necessary
			glDeleteShader(vertex);
			glDeleteShader(fragment);
			if (geometryPath != nullptr)
				glDeleteShader(geometry);
		}

		Shader(const char* computePath, std::string header = "")
		{
			std::string computeCode;
			try
			{
				computeCode = header + Shadinclude::load(computePath);
			}
			catch (std::ifstream::failure& e)
			{
				std::cout << computePath << std::endl;
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
			}

			unsigned int compute;
			const char* cShaderCode = computeCode.c_str();
			// compute shader
			compute = glCreateShader(GL_COMPUTE_SHADER);
			glShaderSource(compute, 1, &cShaderCode, nullptr);
			glCompileShader(compute);
			checkCompileErrors(compute, "COMPUTE");

			// shader Program
			ID = glCreateProgram();
			glAttachShader(ID, compute);
			glLinkProgram(ID);
			checkCompileErrors(ID, "PROGRAM");
		}

		void dispatchCompute(glm::ivec3 size)
		{
			glDispatchCompute(static_cast<unsigned int>(size.x), static_cast<unsigned int>(size.y),
			                  static_cast<unsigned int>(size.z));
		}

		void wait(unsigned int type)
		{
			glMemoryBarrier(type);
		}

		// activate the shader
		// ------------------------------------------------------------------------
		void use() const
		{
			glUseProgram(ID);
		}

		// utility uniform functions
		// ------------------------------------------------------------------------
		void setBool(int position, bool value) const
		{
			glUniform1i(position, static_cast<int>(value));
		}

		// ------------------------------------------------------------------------
		void setInt(int position, int value) const
		{
			glUniform1i(position, value);
		}

		void setInt64(int position, uint64_t value) const
		{
			glUniformHandleui64ARB(position, value);
		}

		// ------------------------------------------------------------------------
		void setFloat(int position, float value) const
		{
			glUniform1f(position, value);
		}

		// ------------------------------------------------------------------------
		void setVec2(int position, const glm::vec2& value) const
		{
			glUniform2fv(position, 1, &value[0]);
		}

		void setUVec2(int position, const glm::ivec2& value) const
		{
			glUniform2ui(position, static_cast<unsigned int>(value.x), static_cast<unsigned int>(value.y));
		}

		void setUVec3(int position, const glm::ivec3& value) const
		{
			glUniform3ui(position, static_cast<unsigned int>(value.x), static_cast<unsigned int>(value.y),
			             static_cast<unsigned int>(value.z));
		}

		void setVec2(int position, float x, float y) const
		{
			glUniform2f(position, x, y);
		}

		// ------------------------------------------------------------------------
		void setVec3(int position, const glm::vec3& value) const
		{
			glUniform3fv(position, 1, &value[0]);
		}

		void setVec3(int position, float x, float y, float z) const
		{
			glUniform3f(position, x, y, z);
		}

		// ------------------------------------------------------------------------
		void setVec4(int position, const glm::vec4& value) const
		{
			glUniform4fv(position, 1, &value[0]);
		}

		void setVec4(int position, float x, float y, float z, float w) const
		{
			glUniform4f(position, x, y, z, w);
		}

		// ------------------------------------------------------------------------
		void setMat2(int position, const glm::mat2& mat) const
		{
			glUniformMatrix2fv(position, 1, GL_FALSE, &mat[0][0]);
		}

		// ------------------------------------------------------------------------
		void setMat3(int position, const glm::mat3& mat) const
		{
			glUniformMatrix3fv(position, 1, GL_FALSE, &mat[0][0]);
		}

		// ------------------------------------------------------------------------
		void setMat4(int position, const glm::mat4& mat) const
		{
			glUniformMatrix4fv(position, 1, GL_FALSE, &mat[0][0]);
		}

		int getUniformPosition(const std::string& name)
		{
			return glGetUniformLocation(ID, name.c_str());
		}

	private:
		// utility function for checking shader compilation/linking errors.
		// ------------------------------------------------------------------------
		void checkCompileErrors(GLuint shader, std::string type, std::string shaderName = "")
		{
			GLint success;
			GLchar infoLog[1024];
			if (type != "PROGRAM")
			{
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if (!success)
				{
					glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
					std::cout << shaderName << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog <<
						"\n -- --------------------------------------------------- -- " << std::endl;
				}
			}
			else
			{
				glGetProgramiv(shader, GL_LINK_STATUS, &success);
				if (!success)
				{
					glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
					std::cout << shaderName << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog <<
						"\n -- --------------------------------------------------- -- " << std::endl;
				}
			}
		}
	};
}
