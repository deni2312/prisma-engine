#include "../include/PlayerController.h"
#include "Components/PhysicsMeshComponent.h"
#include "Components/InstancingComponent.h"
#include <glm/gtx/string_cast.hpp>

#include "engine.h"
#include "../Components/include/InstancingGrassComponent.h"
#include <Components/CloudComponent.h>
#include <Components/WaterComponent.h>

PlayerController::PlayerController(std::shared_ptr<Prisma::Scene> scene) : m_scene{scene} {
    Prisma::NodeHelper nodeHelper;

    m_animatedMesh = std::dynamic_pointer_cast<Prisma::AnimatedMesh>(
        nodeHelper.find(m_scene->root, "MutantMesh")->children()[0]);

    if (m_animatedMesh) {
        m_walkAnimation = std::make_shared<Prisma::Animation>(
            "../../../Resources/DefaultScene/animations/animation.gltf", m_animatedMesh);
        m_jumpAnimation = std::make_shared<Prisma::Animation>(
            "../../../Resources/DefaultScene/animations/jump.gltf",
            m_animatedMesh);
        m_idleAnimation = m_animatedMesh->animator()->animation();
            
    }

    m_bboxMesh = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(m_scene->root, "BBoxMesh"));

    m_sphereMesh = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(m_scene->root, "SphereMesh"));

    m_gunPosition = nodeHelper.find(m_scene->root, "GunPosition");

    m_basePosition = m_sphereMesh->parent()->matrix();
    m_basePosition[3] = glm::vec4(0, 0, 0, 1);

    m_physics = std::dynamic_pointer_cast<Prisma::PhysicsMeshComponent>(m_bboxMesh->components()["Physics"]);
    m_physics->collisionData({Prisma::Physics::Collider::BOX_COLLIDER, 1.0, true});

    m_baseData = m_animatedMesh->parent()->parent()->matrix();
    m_animations = IDLE;
    m_previousAnimations = IDLE;
    auto id = m_physics->physicsId();
    Prisma::Physics::getInstance().bodyInterface().SetFriction(id, 10);
    auto contact = [&](const Body& body) {
        m_isColliding = true;
    };

    auto noContact = [&](const BodyID& body) {
        m_isColliding = false;
    };

    m_physics->onCollisionStay(contact);
    m_physics->onCollisionExit(noContact);

    glm::mat4 rotation = rotate(glm::mat4(1.0), glm::radians(90.0f), glm::vec3(1, 0, 0)) * translate(
                             glm::mat4(1.0), glm::vec3(0, 0, -1));

    std::vector<Prisma::Interpolator::Timeframe> timeframes;
    float amplitude = 1.0f;
    float frequency = 2.0f;
    for (int i = 0; i <= 10; ++i) {
        float t = static_cast<float>(i) / 10.0f * glm::pi<float>() * 2.0f;
        float x = std::cos(t) * amplitude;
        float y = std::sin(t) * amplitude;
        timeframes.push_back({rotation * translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f)), 1.0f});
    }

    m_interpolator.timeframe(timeframes);


    if (m_numScene == 0) {
        m_particleController.init(m_scene->root);


        auto grass1Renderer = std::make_shared<Prisma::Node>();

        grass1Renderer->name("Grass1Renderer");

        auto grass1Instance = std::make_shared<Prisma::InstancingGrassComponent>();

        auto grass2Renderer = std::make_shared<Prisma::Node>();

        grass2Renderer->name("Grass2Renderer");

        auto grass2Instance = std::make_shared<Prisma::InstancingGrassComponent>();

        std::vector<Prisma::Mesh::MeshData> models;
        std::vector<Prisma::Mesh::MeshData> models1;
        auto grass2 = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(m_scene->root, "Grass_2.250")->children()[0]);
        auto grass1 = std::dynamic_pointer_cast<Prisma::Mesh>(nodeHelper.find(m_scene->root, "Grass_1.250")->children()[0]);

        int range = 2000;

        for (int i = 0; i < 10000; ++i) {
            // Random translation
            float x = (std::rand() % range - range / 2);
            float z = (std::rand() % range - range / 2);

            float x1 = (std::rand() % range - range / 2);
            float z1 = (std::rand() % range - range / 2);

            glm::mat4 translation = glm::translate(grass2->parent()->matrix(), glm::vec3(x, 0, z)) * glm::rotate(glm::mat4(1.0), glm::radians(static_cast<float>(std::rand() % 360)), glm::vec3(0, 1, 0));
            glm::mat4 translation1 = glm::translate(grass1->parent()->matrix(), glm::vec3(x1, 0, z1)) * glm::rotate(glm::mat4(1.0), glm::radians(static_cast<float>(std::rand() % 360)), glm::vec3(0, 1, 0));

            glm::mat4 normalMatrix = glm::transpose(glm::inverse(translation));
            glm::mat4 normalMatrix1 = glm::transpose(glm::inverse(translation1));

            models.push_back({translation, normalMatrix});
            models1.push_back({translation1, normalMatrix1});
        }

        grass1Instance->mesh(grass1);

        grass1Instance->models(models);

        grass1Renderer->addComponent(grass1Instance);

        m_scene->root->addChild(grass1Renderer);

        grass2Instance->mesh(grass2);

        grass2Instance->models(models1);

        grass2Renderer->addComponent(grass2Instance);

        m_scene->root->addChild(grass2Renderer);

        auto water = std::make_shared<Prisma::Node>();

        water->name("WaterNode");

        auto waterComponent = std::make_shared<Prisma::WaterComponent>();

        waterComponent->nodePosition(m_bboxMesh);

        waterComponent->radius(5);

        water->addComponent(waterComponent);

        m_scene->root->addChild(water);

    }

    m_sphereMesh->visible(false);

    for (int i = 0; i < m_maxBalls; i++) {
        auto ball = Prisma::Mesh::instantiate(m_sphereMesh);
        auto lightParent = std::make_shared<Prisma::Node>();
        ball->name("Ball");
        auto light = std::make_shared<Prisma::Light<Prisma::LightType::LightOmni>>();
        Prisma::LightType::LightOmni lightType;
        lightType.diffuse = glm::vec4(8);
        light->type(lightType);
        light->name("LightBall");
        lightParent->name("LightParent");
        lightParent->addChild(light);
        ball->parent()->addChild(lightParent);

        ball->parent()->matrix(glm::translate(glm::mat4(1), glm::vec3(0, -1000, 0)));

        auto physicsComponent = std::make_shared<Prisma::PhysicsMeshComponent>();
        ball->addComponent(physicsComponent);
        m_balls.push_back(ball);

        physicsComponent->collisionData({Prisma::Physics::Collider::SPHERE_COLLIDER, 1.0, false});
        /* physicsComponent->onCollisionEnter([ballCopy, this](const auto& body) {
            if (auto ballShared = ballCopy.lock()) {
                this->m_balls.push_back(ballShared);
            }
        });*/
    }

    m_areaLight = nodeHelper.find(m_scene->root, "ParentArea_74");
    createCamera();
    createKeyboard();
}

void PlayerController::updateCamera() {
    m_velocity = m_baseVelocity;
    //m_velocity = m_baseVelocity * 1.0f / (float)Prisma::Engine::getInstance().fps();

    // Calculate the new position based on yaw, pitch, and distance from target
    glm::vec3 offset;
    offset.x = m_distance * cos(glm::radians(m_pitch)) * cos(glm::radians(m_yaw));
    offset.y = m_distance * sin(glm::radians(m_pitch));
    offset.z = m_distance * cos(glm::radians(m_pitch)) * sin(glm::radians(m_yaw));

    m_position = m_target + offset;

    // Update the camera view matrix
    m_scene->camera->position(m_position);
    m_scene->camera->center(m_target);
    m_scene->camera->up(m_up);
}


void PlayerController::updateKeyboard() {
    updateAnimations();
    auto playerData = m_animatedMesh->parent()->parent()->matrix();
    auto id = m_physics->physicsId();
    glm::vec3 frontClamp = m_front;
    frontClamp.y = 0;
    glm::mat4 offsetRotation;
    auto isJumping = m_animatedMesh->animator()->animation()->id() == m_jumpAnimation->id() && m_animatedMesh->
                                                                                               animator()->currentTime() + m_jumpAnimation->ticksPerSecond() * 1.0f /
                     Prisma::Engine::getInstance().fps() >=
                     m_jumpAnimation->duration();
    auto velocity = Prisma::Physics::getInstance().bodyInterface().GetLinearVelocity(id);
    if (isJumping && m_previousAnimations == JUMP) {
        m_previousAnimations = IDLE;
    }
    if ((m_animations == IDLE || m_animations == WALK) && m_isColliding && !isJumping) {
        if (glfwGetKey(m_window, Prisma::KEY_W) == GLFW_PRESS) {
            auto currentDirection = Prisma::JtoVec3(-normalize(glm::vec3(frontClamp * m_velocity)));
            currentDirection.SetY(velocity.GetY());
            m_currentDirection = currentDirection;
            Prisma::Physics::getInstance().bodyInterface().SetLinearVelocity(id, currentDirection);
            offsetRotation = rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1));
            playerData = m_baseData * rotate(glm::mat4(offsetRotation), glm::radians(m_yaw),
                                             glm::vec3(0, 0, 1));
            m_animatedMesh->parent()->parent()->matrix(playerData);
            m_previousClick = Prisma::KEY_W;
            m_previousAnimations = WALK;
        }
        if (glfwGetKey(m_window, Prisma::KEY_A) == GLFW_PRESS) {
            auto currentDirection = Prisma::JtoVec3(normalize(cross(frontClamp, m_up)) * m_velocity);
            currentDirection.SetY(velocity.GetY());
            m_currentDirection = currentDirection;
            Prisma::Physics::getInstance().bodyInterface().SetLinearVelocity(id, currentDirection);
            playerData = m_baseData * rotate(glm::mat4(1.0f), glm::radians(m_yaw), glm::vec3(0, 0, 1));
            m_animatedMesh->parent()->parent()->matrix(playerData);
            m_previousClick = Prisma::KEY_A;
            m_previousAnimations = WALK;
        }
        if (glfwGetKey(m_window, Prisma::KEY_S) == GLFW_PRESS) {
            auto currentDirection = Prisma::JtoVec3(normalize(glm::vec3(frontClamp * m_velocity)));
            currentDirection.SetY(velocity.GetY());
            m_currentDirection = currentDirection;
            Prisma::Physics::getInstance().bodyInterface().SetLinearVelocity(id, currentDirection);
            offsetRotation = rotate(glm::mat4(1.0f), glm::radians(270.0f), glm::vec3(0, 0, 1));
            playerData = m_baseData * rotate(glm::mat4(offsetRotation), glm::radians(m_yaw),
                                             glm::vec3(0, 0, 1));
            m_animatedMesh->parent()->parent()->matrix(playerData);
            m_previousClick = Prisma::KEY_S;
            m_previousAnimations = WALK;
        }
        if (glfwGetKey(m_window, Prisma::KEY_D) == GLFW_PRESS) {
            auto currentDirection = Prisma::JtoVec3(-normalize(cross(frontClamp, m_up)) * m_velocity);
            currentDirection.SetY(velocity.GetY());
            m_currentDirection = currentDirection;
            Prisma::Physics::getInstance().bodyInterface().SetLinearVelocity(id, currentDirection);
            offsetRotation = rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0, 0, 1));
            playerData = m_baseData * rotate(glm::mat4(offsetRotation), glm::radians(m_yaw),
                                             glm::vec3(0, 0, 1));
            m_animatedMesh->parent()->parent()->matrix(playerData);
            m_previousClick = Prisma::KEY_D;
            m_previousAnimations = WALK;
        }
        if (glfwGetKey(m_window, Prisma::KEY_G) == GLFW_PRESS) {
            m_hidden = !m_hidden;
            Prisma::PrismaFunc::getInstance().hiddenMouse(m_hidden);
        }
        clearVelocity();
    }
    Prisma::Physics::getInstance().bodyInterface().ActivateBody(id);
    Prisma::Physics::getInstance().bodyInterface().SetRotation(id, Quat::sIdentity(), EActivation::Activate);
    Prisma::Physics::getInstance().bodyInterface().SetAngularVelocity(id, Vec3::sZero());
    m_animatedMesh->animator()->updateAnimation(1.0f / Prisma::Engine::getInstance().fps());
}

void PlayerController::scene(std::shared_ptr<Prisma::Scene> scene) {
    m_scene = scene;
}

void PlayerController::update() {
    
    if (m_numScene == 0) {
        m_particleController.update();        
    }
    target(m_animatedMesh->parent()->finalMatrix()[3]);
    updateCamera();
    updateKeyboard();
}

std::shared_ptr<Prisma::CallbackHandler> PlayerController::callback() {
    return m_handler;
}

void PlayerController::target(glm::vec3 target) {
    m_target = target;
}

void PlayerController::createCamera() {
    m_window = Prisma::PrismaFunc::getInstance().window();
    m_handler = std::make_shared<Prisma::CallbackHandler>();
    m_distance = 5;
    m_handler->mouse = [this](float x, float y) {
        float xpos = x;
        float ypos = y;

        if (m_firstMouse) {
            m_lastX = xpos;
            m_lastY = ypos;
            m_firstMouse = false;
        }

        float xoffset = xpos - m_lastX;
        float yoffset = m_lastY - ypos; // reversed since y-coordinates go from bottom to top
        m_lastX = xpos;
        m_lastY = ypos;

        float sensitivity = 0.1f; // change this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

        // make sure that when pitch is out of bounds, screen doesn't get flipped
        if (m_pitch > 89.0f)
            m_pitch = 89.0f;
        if (m_pitch < -89.0f)
            m_pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = normalize(front);
    };
    m_handler->mouseClick = [&](int button, int action, double x, double y) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            std::shared_ptr<Prisma::Mesh> ball = m_balls[m_ballIndex].lock();

            auto physicsComponent = std::dynamic_pointer_cast<Prisma::PhysicsMeshComponent>(ball->components()["Physics"]);
            auto position = m_gunPosition->finalMatrix()[3] + glm::vec4(
                                Prisma::JfromVec3(m_currentDirection), 0.0f);

            m_basePosition[3] = position;

            ball->parent()->matrix(m_basePosition);
            std::weak_ptr<Prisma::Mesh> ballCopy = ball;
            physicsComponent->collisionData({Prisma::Physics::Collider::SPHERE_COLLIDER, 1.0, true});
            /* physicsComponent->onCollisionEnter([ballCopy, this](const auto& body) {
                if (auto ballShared = ballCopy.lock()) {
                    this->m_balls.push_back(ballShared);
                }
            });*/
            Prisma::Physics::getInstance().bodyInterface().AddImpulse(physicsComponent->physicsId(),
                                                                      m_currentDirection * 5);
            m_ballIndex=(m_ballIndex+1)%m_maxBalls;
        }
    };
}

void PlayerController::createKeyboard() {
    m_handler->keyboard = [this](int key, int scancode, int action, int mods) {
        if (key == Prisma::KEY_G && action == GLFW_RELEASE) {
            m_hide = !m_hide;
            if (m_hide) {
                Prisma::PrismaFunc::getInstance().hiddenMouse(m_hide);
            } else {
                Prisma::PrismaFunc::getInstance().hiddenMouse(m_hide);
            }
            m_previousClick = Prisma::KEY_G;
        }
        if (key == Prisma::KEY_SPACE && action == GLFW_PRESS) {
            auto id = m_physics->physicsId();

            if (m_animatedMesh->animator()->animation()->id() != m_jumpAnimation->id()) {
                Prisma::Physics::getInstance().bodyInterface().AddImpulse(id, Vec3(0, 5.0f, 0));
            }
            m_previousClick = Prisma::KEY_SPACE;
            m_previousAnimations = JUMP;
        }
    };
}

void PlayerController::clearVelocity() {
    auto id = m_physics->physicsId();
    auto velocity = Prisma::Physics::getInstance().bodyInterface().GetLinearVelocity(id);

    if (glfwGetKey(m_window, m_previousClick) == GLFW_RELEASE && m_animations == WALK) {
        Prisma::Physics::getInstance().bodyInterface().SetLinearVelocity(id, Vec3(0.0f, velocity.GetY(), 0.0f));
        m_animatedMesh->animator()->playAnimation(m_idleAnimation, m_blending);
        m_previousAnimations = IDLE;
    }
}

void PlayerController::updateAnimations() {
    if (m_previousAnimations != m_animations) {
        m_animations = m_previousAnimations;
        switch (m_animations) {
            case IDLE: {
                m_animatedMesh->animator()->playAnimation(m_idleAnimation, m_blending);
                break;
            }
            case WALK: {
                m_animatedMesh->animator()->playAnimation(m_walkAnimation, m_blending);
                break;
            }
            case JUMP: {
                m_animatedMesh->animator()->playAnimation(m_jumpAnimation, m_blending);
                break;
            }
            case DEFAULT: {
                break;
            }
        }
    }
}