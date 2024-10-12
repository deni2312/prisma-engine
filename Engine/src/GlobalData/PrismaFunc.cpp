#include "../../include/GlobalData/PrismaFunc.h"
#include "../../include/GlobalData/GlobalData.h"
#include "../../include/GlobalData/Defines.h"
#include "../../include/Helpers/Settings.h"
#include "../../include/Helpers/SettingsLoader.h"

#include <iostream>

#include <fstream>

struct PrivatePrisma {
    std::shared_ptr<Prisma::CallbackHandler> callback;
    bool initCallback = false;;
};

std::shared_ptr<PrivatePrisma> privatePrisma;

namespace Prisma {

    void framebufferSizeCallback(GLFWwindow* window, int width, int height)
    {
        if (privatePrisma->callback->resize) {
            privatePrisma->callback->resize(width, height);
        }
        glViewport(0, 0, width, height);
    }

    void mouseCallback(GLFWwindow* window, double x, double y)
    {
        if (privatePrisma->callback->mouse) {
            privatePrisma->callback->mouse(x, y);
        }
    }
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        if (privatePrisma->callback->mouseClick) {
            privatePrisma->callback->mouseClick(button, action, xpos, ypos);
        }
    }

    void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        if (privatePrisma->callback->keyboard) {
            privatePrisma->callback->keyboard(key, scancode,action,mods);
        }
    }

    void rollCallback(GLFWwindow* window, double x, double y)
    {
        if (privatePrisma->callback->rollMouse) {
            privatePrisma->callback->rollMouse(x, y);
        }
    }
}

Prisma::PrismaFunc::PrismaFunc()
{
    std::ifstream f(DIR_DEFAULT_SETTINGS);
    Settings settings = Prisma::SettingsLoader::getInstance().getSettings();
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_window = glfwCreateWindow(settings.width, settings.height, settings.name.c_str(), NULL, NULL);
    if (m_window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
    }
    glfwMakeContextCurrent(m_window);
    glewInit();
    glewExperimental = GL_TRUE;
    glEnable(GL_DEPTH_TEST);
    glClearColor(CLEAR_COLOR.x, CLEAR_COLOR.y, CLEAR_COLOR.z, CLEAR_COLOR.w);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    privatePrisma = std::make_shared<PrivatePrisma>();

    defaultBlack.loadTexture({ DIR_DEFAULT_BLACK });
    defaultWhite.loadTexture({ DIR_DEFAULT_WHITE });
    defaultNormal.loadTexture({ DIR_DEFAULT_NORMAL });
}

void Prisma::PrismaFunc::swapBuffers()
{
    glfwSwapBuffers(m_window);
    glfwPollEvents();
}

void Prisma::PrismaFunc::clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Prisma::PrismaFunc::setCallback(std::shared_ptr<CallbackHandler> callbackHandler)
{
    privatePrisma->callback = callbackHandler;
    if (!privatePrisma->initCallback) {
        glfwSetFramebufferSizeCallback(m_window, Prisma::framebufferSizeCallback);
        glfwSetCursorPosCallback(m_window, Prisma::mouseCallback);
        glfwSetMouseButtonCallback(m_window, Prisma::mouseButtonCallback);
        glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GLFW_TRUE);
        glfwSetKeyCallback(m_window, Prisma::keyboardCallback);
        glfwSetScrollCallback(m_window, Prisma::rollCallback);
        privatePrisma->initCallback = true;
    }
}

void Prisma::PrismaFunc::closeWindow()
{
    glfwSetWindowShouldClose(m_window, true);
}

bool Prisma::PrismaFunc::shouldClose()
{
    return glfwWindowShouldClose(m_window);
}

void Prisma::PrismaFunc::hiddenMouse(bool hidden)
{
    glfwSetInputMode(m_window, GLFW_CURSOR, hidden?GLFW_CURSOR_NORMAL:GLFW_CURSOR_DISABLED);
}

void Prisma::PrismaFunc::msaa(bool isMsaa,int samples)
{
    if (isMsaa) {
        glfwWindowHint(GLFW_SAMPLES, samples);
        glEnable(GL_MULTISAMPLE);
    }
    else {
        glDisable(GL_MULTISAMPLE);
    }
}

GLFWwindow* Prisma::PrismaFunc::window()
{
	return m_window;
}

void Prisma::PrismaFunc::destroy()
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}