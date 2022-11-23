# tiny_engine

## Info

Libraries Used:
- [OpenGL](https://www.opengl.org/) by Khronos Group
- [GLAD](https://glad.dav1d.de/) by David Herberth
- [GLFW](https://www.glfw.org/)
- [miniaudio](https://github.com/mackron/miniaudio) by David Reid
- [GLtext](https://github.com/vallentin/glText) by Christian Vallentin 
- [tiny_obj_loader](https://github.com/tinyobjloader/tinyobjloader)

Features:
- Postprocessing
- Job System (multithreading)
- sprite rendering
- spritesheet parsing/animating
- .obj file parsing/mesh rendering
- 2d vector shape drawing (sqaures, circles, lines)
- deterministic randomness

### TODO:
- Learn how to use Perforce
- 3D lighting
    - ~~Basic blinn-phong~~
    - ~~Point & Directional light~~
    - Materials
- default mesh generation (plane, cube, sphere)
- shadow mapping
- HDR & Bloom
- instancing
- Scene graph/frustum culling 
    - https://learnopengl.com/Guest-Articles/2021/Scene/Scene-Graph
    - https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    - Space partitioning (BSP/Quadtree/Octree)
- animation
    - https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation
    - https://www.youtube.com/watch?v=f3Cr8Yx3GGA

### Possible future features:
- ~~debug line drawing~~
- ~~Postprocessing~~
- Event system
- ~~Job System (threading)~~
    - lock free queue: https://jbseg.medium.com/lock-free-queues-e48de693654b
- Intrusive profiling API - using chrome://tracing as the visualizing backend
    - https://perfetto.dev/docs/instrumentation/tracing-sdk