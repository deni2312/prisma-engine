<p>
<h1 align="center">
Prisma Engine
</p>
<center>
    <img src="https://github.com/deni2312/prisma-engine/blob/main/bin/images/screenshot.png" alt="DamagedHelmet">
</center>

## 
Prisma Engine is an OpenGL-based rendering engine designed for creating dynamic 3D applications. It is a personal project created to showcase the capabilities of modern OpenGL. The engine is constantly evolving, so there could be significant changes from one version to another.

## Features

- **OpenGL 4.6 Support**
- **Clustered rendering pipelines**
- **Directional and Omni Lights**
- **Omni shadows and Cascaded shadow maps**
- **Animations/Animations blending**
- **Bullet Physics Integration**
- **Indirect Drawing**
- **HDR**
- **Omni and Directional lights Shadows**
- **glTF Loader**
- **Physically Based Rendering (PBR)** 
- **Image Based Lighting (IBL)**
- **Screen space reflections**
- **Scene Graph**
- **Bloom**
- **Postprocessing effects**
- **Particle effects**
- **Volumetric Clouds**
- **Heightmap landscapes**
- **Procedural texture generation**
- **Grass optimized by frustum culling**
- **GPU sorted transparencies**
- **Thread safe animations**
- **GPU frustum culled meshes**
- **Editor:**
    - **Run/Debug modes**
    - **Transform Modifier**
    - **Physics Modifier**
    - **Show Scene Graph**
    - **Animation Viewer**
    - **Change Render Pipeline (Forward clustered/Deferred clustered)**
    - **Texture debugger**


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
3. Open the project with visual studio

4. Open Tools -> Options -> Vcpkg then change the vcpkg directory to the path of vcpkg executable

5. Click on main.cpp and run the default scene

6. (Optionally) In  `configuration\settings.json` file you can change some settings like width, height and name of the window.

## Demos

### Demo


An example of a demo scene with deferred clustered shading and forward to run smoothly many lights, with some updates like GPU sorted transparencies, terrain rendering and height based texture, frustum culled grass, that has also animations , it runs at 120 FPS 500+ different animated meshes that run updates in different threads. (Click on the image for the youtube video)

https://www.youtube.com/watch?v=ylrV2Ebg6FA

[![Sponza](https://img.youtube.com/vi/ylrV2Ebg6FA/0.jpg)](https://youtu.be/ylrV2Ebg6FA?si=CkqPa_8tHgQe89RT)

### Demo


An example of a demo scene with animation blending, particle effects and screen space reflections. (Click on the image for the youtube video)

https://youtu.be/4LInQ8es3wM

[![Sponza](https://img.youtube.com/vi/4LInQ8es3wM/0.jpg)](https://youtu.be/4LInQ8es3wM)


### Sponza


An example of a Sponza scene that utilizes 1000 point lights through clustered shading, with some post-processing, and includes animations taken from Mixamo. (Click on the image for the youtube video)

https://www.youtube.com/watch?v=T0_1JGvYyl0

[![Sponza](https://img.youtube.com/vi/T0_1JGvYyl0/0.jpg)](https://www.youtube.com/watch?v=T0_1JGvYyl0)

### Physics

An example of a scene with bullet physics, point shadows and the dynamic loading of a sponza scene (Click on the image for the youtube video)

[![Sponza](https://img.youtube.com/vi/W6aWU5asums/0.jpg)](https://www.youtube.com/watch?v=W6aWU5asums)


### Cascaded Shadow Mapping


An example of NVIDIA Orca scene that has Cascaded Shadow Maps, animations and 1000 lights rendered in realtime. (Click on the image for the youtube video)

https://www.youtube.com/watch?v=PT4RTldmVqc

[![Sponza](https://img.youtube.com/vi/PT4RTldmVqc/0.jpg)](https://www.youtube.com/watch?v=PT4RTldmVqc)



## Default scene

The scene models showcased can be found on the official repository for KhronosGroup's glTF Sample Models:

https://github.com/KhronosGroup/glTF-Sample-Models

# Contributing to Prisma Engine

We welcome contributions to Prisma Engine! Whether it's bug fixes, new features, or documentation improvements, your contributions help make Prisma better for everyone.

## Bug Reports

If you encounter a bug, please submit an issue with detailed information about how to reproduce it.

## Next updates

- **Code refactoring/Eventual bug fixes**

## LICENSE

All the software is under MIT (see the LICENSE file) and the icons are from <a target="_blank" href="https://icons8.com">Icons8</a>

The NVIDIA Orca scene has a  CC BY-NC-SA 4.0 ( https://creativecommons.org/licenses/by-nc-sa/4.0/ )

## Feature Requests

If you have a feature request, open an issue and describe the feature you would like to see.

Thank you for contributing to Prisma Engine!

# Bibliography/Sitography


- https://developer.nvidia.com/ue4-sun-temple
- https://learnopengl.com/
- https://ogldev.org/
- https://github.com/ocornut/imgui
- https://github.com/DaveH355/clustered-shading
- https://github.com/bulletphysics/bullet3
- https://github.com/KhronosGroup/glTF-Sample-Models
- https://www.mixamo.com/





