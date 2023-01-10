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
- 2d vector shape drawing (squares, circles, lines)
- deterministic randomness

### TODO:
- Learn how to use Perforce
- make sure pch works with msvc
- ~~instancing~~
    - ~~allow data to be passed through extra vertex attributes~~
- 3D particles
- Add sampler functionality to Shader class. Can add a sampler to a shader, and it'll keep track of it's texture units
- Try making the island/waterfall scene!
    - ~~Floating island~~
    - ~~Rock for waterfall~~
    - Make rock bigger, waterfall starts high up - mostly out of view
    - ~~pond in middle~~, area near pond is green, rest of island is white
    - ~~pond -> water (shader/mesh/tex/etc)~~
    - ~~grass on island~~
        - ~~grass spawning algorithm~~
    - waterfall flowing into pond
        - waterfall shader/mesh/particles
- PBR shading
- Resource system
    - load from filesystem
    - store things like Shader, Mesh, Model, etc objects
    - allow access to these things through shared pointers

- HDR/Tonemapping & Bloom
- Scene graph/frustum culling 
    - https://learnopengl.com/Guest-Articles/2021/Scene/Scene-Graph
    - https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
    - Space partitioning (BSP/Quadtree/Octree)
- animation
    - .obj files don't support animation... might need to implement gltf or collada or fbx support
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




## Personal Notes:
- MTL:
Ka: specifies ambient color     values between 0 and 1 for the RGB components.
Kd: specifies diffuse color
Ks: specifies specular color
Ke: emissive coefficient (rgb emission color)
Ns: focus of specular highlights. normally range from 0 to 1000, with a high value resulting in a tight, concentrated highlight.
Ni: defines the optical density (aka index of refraction) values can range from 0.001 to 10. A value of 1.0 means that light does not bend as it passes through an object.
d: (dissolve) specifies alpha (0 = fully transparent, 1 = fully opaque)