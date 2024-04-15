<p>
<h1 align="center">
Prisma Engine
</p>
<center>
    <img src="https://github.com/deni2312/prisma-engine/bin/images/screenshot.png" alt="DamagedHelmet">
</center>

## 
Prisma Engine is an OpenGL-based rendering engine designed for creating dynamic 3D applications. It is a personal project created to showcase the capabilities of modern OpenGL. The engine is constantly evolving, so there could be significant changes from one version to another.

## Features

- **OpenGL 4.6 Support**
- **Clustered rendering pipelines**
- **Directional and Omni Lights**
- **Bullet Physics Integration**
- **Indirect Drawing**
- **Omni and Directional lights Shadows**
- **glTF Loader**
- **Physically Based Rendering (PBR)** 
- **Image Based Lighting (IBL)**
- **Scene Graph**
- **Editor:**
    - **Transform Modifier**
    - **Physics Modifier**
    - **Show Scene Graph**
    - **Change Render Pipeline (Forward clustered/Deferred clustered)**

## Getting Started

To start using Prisma Engine in your projects, follow these steps:

1. Clone the repository:
```bash
git clone --recursive https://github.com/deni2312/prisma-engine.git
```
2. Install dependencies with vcpkg:
```bash
cd prisma-engine
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
cd ..
.\vcpkg\vcpkg install
  ```
3. Open Visual studio and go to Project --> CMake Settings --> CMake Command arguments and set it:
```bash
vcpkg/vcpkg/scripts/buildsystems/vcpkg.cmake
  ```
4. Then delete and reload cache (by right clicking on the project)

5. Click on main.cpp and run the default scene

6. (Optionally) In  `configuration\settings.json` file you can change some settings like width, height and camera options.

## Demos

### Sponza


An example of sponza scene that runs 1000 point lights through clustered shading with some post-processing (Click on the image for the youtube video)

[![Sponza](https://img.youtube.com/vi/QSJAYbaVJJw/0.jpg)](https://www.youtube.com/watch?v=QSJAYbaVJJw)

### Physics

An example of a scene with bullet physics, point shadows and the dynamic loading of a sponza scene (Click on the image for the youtube video)

[![Sponza](https://img.youtube.com/vi/W6aWU5asums/0.jpg)](https://www.youtube.com/watch?v=W6aWU5asums)



## Default scene

The scene models showcased can be found on the official repository for KhronosGroup's glTF Sample Models:

https://github.com/KhronosGroup/glTF-Sample-Models

# Contributing to Prisma Engine

We welcome contributions to Prisma Engine! Whether it's bug fixes, new features, or documentation improvements, your contributions help make Prisma better for everyone.

## Bug Reports

If you encounter a bug, please submit an issue with detailed information about how to reproduce it.

## Next updates

- **Skeletal Meshes**
- **Code refactoring**
- **Cache physic states**
- **Transparency**
- **Cascade shadow maps for directional lights**
- **Frustum culling(compute shader)**
- **Run/Debug buttons**


## LICENSE

All the software is under MIT (see the LICENSE file) and the icons are from <a target="_blank" href="https://icons8.com">Icons8</a>


## Feature Requests

If you have a feature request, open an issue and describe the feature you would like to see.

Thank you for contributing to Prisma Engine!

# Bibliography/Sitography

- https://learnopengl.com/
- https://ogldev.org/
- https://github.com/ocornut/imgui
- https://github.com/DaveH355/clustered-shading
- https://github.com/bulletphysics/bullet3
- https://github.com/KhronosGroup/glTF-Sample-Models





