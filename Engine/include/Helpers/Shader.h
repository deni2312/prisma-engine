#pragma once
#include "GL/glew.h"
#include "glm/glm.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


namespace Prisma {
    class Shader
    {
    public:
        struct ShaderHeaders {
            std::string vertex = "";
            std::string geometry = "";
            std::string fragment = "";
        };

        unsigned int ID;
        // constructor generates the shader on the fly
        // ------------------------------------------------------------------------
        Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr, ShaderHeaders headers = {"","",""})
        {
            // 1. retrieve the vertex/fragment source code from filePath
            std::string vertexCode;
            std::string fragmentCode;
            std::string geometryCode;
            std::ifstream vShaderFile;
            std::ifstream fShaderFile;
            std::ifstream gShaderFile;
            // ensure ifstream objects can throw exceptions:
            vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try
            {
                // open files
                vShaderFile.open(vertexPath);
                fShaderFile.open(fragmentPath);
                std::stringstream vShaderStream, fShaderStream;
                // read file's buffer contents into streams
                vShaderStream << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();
                // close file handlers
                vShaderFile.close();
                fShaderFile.close();
                // convert stream into string
                vertexCode = headers.vertex+vShaderStream.str();
                fragmentCode = headers.fragment + fShaderStream.str();
                // if geometry shader path is present, also load a geometry shader
                if (geometryPath != nullptr)
                {
                    gShaderFile.open(geometryPath);
                    std::stringstream gShaderStream;
                    gShaderStream << gShaderFile.rdbuf();
                    gShaderFile.close();
                    geometryCode = headers.geometry + gShaderStream.str();
                }
            }
            catch (std::ifstream::failure& e)
            {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            }

            std::string shaderName = std::string(vertexPath) + " " +std::string(fragmentPath)+"\n";

            const char* vShaderCode = vertexCode.c_str();
            const char* fShaderCode = fragmentCode.c_str();
            // 2. compile shaders
            unsigned int vertex, fragment;
            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX",shaderName);
            // fragment Shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT",shaderName);
            // if geometry shader is given, compile geometry shader
            unsigned int geometry;
            if (geometryPath != nullptr)
            {
                const char* gShaderCode = geometryCode.c_str();
                geometry = glCreateShader(GL_GEOMETRY_SHADER);
                glShaderSource(geometry, 1, &gShaderCode, NULL);
                glCompileShader(geometry);
                checkCompileErrors(geometry, "GEOMETRY");
            }
            // shader Program
            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            if (geometryPath != nullptr)
                glAttachShader(ID, geometry);
            glLinkProgram(ID);
            checkCompileErrors(ID, "PROGRAM");
            // delete the shaders as they're linked into our program now and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            if (geometryPath != nullptr)
                glDeleteShader(geometry);

        }

        Shader(const char* computePath,std::string header="") {
            std::string computeCode;
            std::ifstream cShaderFile;
            // ensure ifstream objects can throw exceptions:
            cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try
            {
                // open files
                cShaderFile.open(computePath);
                std::stringstream cShaderStream;
                // read file's buffer contents into streams
                cShaderStream << cShaderFile.rdbuf();
                // close file handlers
                cShaderFile.close();
                // convert stream into string
                computeCode = header+cShaderStream.str();
            }
            catch (std::ifstream::failure& e)
            {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            }

            unsigned int compute;
            const char* cShaderCode = computeCode.c_str();
            // compute shader
            compute = glCreateShader(GL_COMPUTE_SHADER);
            glShaderSource(compute, 1, &cShaderCode, NULL);
            glCompileShader(compute);
            checkCompileErrors(compute, "COMPUTE");

            // shader Program
            ID = glCreateProgram();
            glAttachShader(ID, compute);
            glLinkProgram(ID);
            checkCompileErrors(ID, "PROGRAM");
        }

        void dispatchCompute(glm::ivec3 size) {
            glDispatchCompute((unsigned int)size.x, (unsigned int)size.y, (unsigned int)size.z);
        }

        void wait(unsigned int type) {
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
            glUniform1i(position, (int)value);
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
            glUniform2ui(position, (unsigned int)value.x, (unsigned int)value.y);
        }

        void setUVec3(int position, const glm::ivec3& value) const
        {
            glUniform3ui(position, (unsigned int)value.x, (unsigned int)value.y, (unsigned int)value.z);
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

        int getUniformPosition(const std::string& name) {
            return glGetUniformLocation(ID, name.c_str());
        }

    private:
        // utility function for checking shader compilation/linking errors.
        // ------------------------------------------------------------------------
        void checkCompileErrors(GLuint shader, std::string type,std::string shaderName="")
        {
            GLint success;
            GLchar infoLog[1024];
            if (type != "PROGRAM")
            {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << shaderName<<"ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                }
            }
            else
            {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << shaderName<<"ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                }
            }
        }
    };
}