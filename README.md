# tiny_engine

## Info

Libraries Used:
- [OpenGL](https://www.opengl.org/) by Khronos Group
- [GLAD](https://glad.dav1d.de/) by David Herberth
- [GLFW](https://www.glfw.org/)
- [miniaudio](https://github.com/mackron/miniaudio) by David Reid
- [GLtext](https://github.com/vallentin/glText) by Christian Vallentin 
- [tiny_obj_loader](https://github.com/tinyobjloader/tinyobjloader)

Features/Included Implementations:
- Postprocessing
- Job System (multithreading)
- deterministic randomness
- sprite rendering
- spritesheet parsing/animating
- .obj file parsing/mesh rendering
- particle system
- 2d and 3d vector shape drawing (squares, circles, lines, spheres, cubes)
- .obj loading/rendering (w/material support)
- shadow mapping
- skybox
- postprocessing
- Profiling with chrome://tracing api
- Shader hot reloading

Written with this engine:  
[Part Of The Pack](https://faultypine.itch.io/part-of-the-pack)  
![image](https://user-images.githubusercontent.com/53064235/211900902-6c979f44-357a-4449-adf0-40598d9a7edb.png)  
Simple nature scene I used to implement & test 3D features  
![Capture](https://user-images.githubusercontent.com/53064235/211901373-47841a5b-5ea7-49e9-afac-28dd2b46d4aa.PNG)  



### TODO/Topics to Experiment With:
- Learn how to use Perforce
- ~~make sure pch works with msvc~~
- ~~instancing~~
    - ~~allow data to be passed through extra vertex attributes~~
- ~~Add sampler functionality to Shader class. Can add a sampler to a shader, and it'll keep track of it's texture units~~
- Try making the island/waterfall scene!
    - ~~Floating island~~
    - ~~Rock for waterfall~~
    - ~~pond in middle~~, area near pond is green, rest of island is white
    - ~~pond -> water (shader/mesh/tex/etc)~~
    - ~~grass on island~~
        - ~~grass spawning algorithm~~
    - waterfall
        - flowing into pond
        - Make rock bigger, waterfall starts high up - mostly out of view
        - waterfall shader/mesh/particles
            - 3D particles - extension of 2d particle system
- better grass sway w/distortion texture to emulate wind better
- outline postprocessing shader
    - generate normals texture for postprocessing
        - at this point i would be generating depth and
        normals as seperate g-buffers... shouldn't I just 
        commit and go full deferred at that point? ....
        I need to actually build a renderer....
        - maybe a full deferred renderer is a bit much.
        - I think it would be best to just have the 
            depth prepass also render normals (multiple render targets)
- ~~Skybox~~
- PBR lighting???
- Resource system
    - load from filesystem
    - store things like Shader, Mesh, Model, etc objects
    - allow access to these things through ids or shared pointers
- ~~Shader hot reload~~
- HDR/Tonemapping & Bloom
- Scene graph/frustum culling 
    - https://learnopengl.com/Guest-Articles/2021/Scene/Scene-Graph
    - https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
- Space partitioning (BSP/Quadtree/Octree)
- animation
    - .obj files don't support animation... might need to implement gltf or collada or fbx support
    - https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation
    - https://www.youtube.com/watch?v=f3Cr8Yx3GGA
- Portals! (And utilizing that to make "non-euclidean" spaces)
- 3D World Wrapping (I.E. Manifold garden)
- (Visualizing) L-Systems

#### Tetris

[Engine Changes]
- Z-index sprite ordering
- asset hot-reloading
- ^^^ (Renderer and Asset systems?)

### Possible future features:
- ~~debug line drawing~~
- ~~Postprocessing~~
- Event system
- ~~Job System (threading)~~
    - lock free queue: https://jbseg.medium.com/lock-free-queues-e48de693654b
- ~~Intrusive profiling API - using chrome://tracing as the visualizing backend~~
    - https://perfetto.dev/docs/instrumentation/tracing-sdk
    - https://gist.github.com/TheCherno/31f135eea6ee729ab5f26a6908eb3a5e




## Personal Notes:
- OSX Retina Screen 1/4 render bug:
    http://disq.us/p/1osvm46: "For OSX retina device users, you may have to double the screen height and width when you create a framebuffer. Otherwise when you want to draw the screen using a framebuffer, some weird thing may occur (like I found only 1/4 of the previous content is rendered on the screen, which is the left corner).
    The reason of this might be that OSX uses 4 pixels to represent one under retina display. And GLFW aware of this problem while some other functions do not. So when you create a 800 x 600 window, it might actually be a 1600 x 1200 window..."

- MTL File Format:
Ka: specifies ambient color     values between 0 and 1 for the RGB components.
Kd: specifies diffuse color
Ks: specifies specular color
Ke: emissive coefficient (rgb emission color)
Ns: focus of specular highlights. normally range from 0 to 1000, with a high value resulting in a tight, concentrated highlight.
Ni: defines the optical density (aka index of refraction) values can range from 0.001 to 10. A value of 1.0 means that light does not bend as it passes through an object.
d: (dissolve) specifies alpha (0 = fully transparent, 1 = fully opaque)
