# tiny_engine

## Info

Libraries Used:
- [OpenGL](https://www.opengl.org/) by Khronos Group
- [GLAD](https://glad.dav1d.de/) by David Herberth
- [GLFW](https://www.glfw.org/)
- [miniaudio](https://github.com/mackron/miniaudio) by David Reid
- [GLtext](https://github.com/vallentin/glText) by Christian Vallentin 
- [imgui](https://github.com/ocornut/imgui) ImGui by Omar
- [Assimp](https://github.com/assimp/assimp) 
- [metadesk](https://dion.systems/metadesk) Metadesk by Dion Systems
- [Bullet](https://github.com/bulletphysics/bullet3) Bullet by BulletPhysics


## Installing Dependencies

This project is currently built and maintained for windows. [vcpkg](https://vcpkg.io/en/) is recommended for installing dependencies

Run `vcpkg install` in the root directory of this project (same directory as the vcpkg.json file)

## Features/Included Implementations
- Postprocessing
- Job System (multithreading)
- deterministic randomness
- sprite rendering
- spritesheet parsing/animating
- .obj file parsing/mesh rendering
- particle system
- 2d and 3d vector shape drawing (squares, circles, lines, spheres, cubes)
- 3d model loading/rendering (w/material support)
- shadow mapping
- skybox
- Shader hot reloading & #include parser
- custom allocator(s)
- custom container types
- custom build system (backed by Ninja)
- compile-time reflection / type metadata generation system (backed by Metadesk)


Written with this engine:  
[Part Of The Pack](https://faultypine.itch.io/part-of-the-pack)  
![image](https://user-images.githubusercontent.com/53064235/211900902-6c979f44-357a-4449-adf0-40598d9a7edb.png)  
Simple nature scene I used to implement & test 3D features  
![Screenshot 2023-04-26 235502](https://user-images.githubusercontent.com/53064235/234755544-509599a3-1df1-4656-bcd0-6be7c841b4fb.png)


QuadTree Implementation:  

https://user-images.githubusercontent.com/53064235/216891116-b9521259-7010-4fa0-bd52-3d54e71d830e.mp4  

Outline/Sketch Effect  

https://user-images.githubusercontent.com/53064235/232272366-69ce9e2f-c13b-4db3-8781-e4ad967f1b2d.mp4  
  
Simple physics

https://github.com/FaultyPine/tiny_engine/assets/53064235/a390abcf-4f2c-4d48-a282-06d47cd22cef



### TODO:

Renderer refactor:

BOOKMARK: Put uniforms in the MODEL, not the shader.
Uniforms are tied to the ENTITY (or really, the entity's model) not to the shader
Uniforms are often shared between shaders for things like prepasses
we should be tracking uniforms and their values on the models so there's 1 place to set them
and the renderer sets the uniforms from there for each of it's passes

- indexing the ObjectData gpu buffer using entity ids right now. 
    Need some way of indexing it 


- "entities"/serialize objects... 
    - Big refactor - move types to the type reflection system
    - rel pointers to make all entities one big blob?
- Renderer
    - Centralized system that takes in draw commands. At end of frame draws all requests and can
        do pre-processing on those commands like batching
    - what I have in mind for this would likely touch nearly every single aspect of the current renderer.
        when I end up doing this - it would honestly be better to start completely from scratch than try to migrate my current
        rendering stuff to what i have in mind.
    - deferred renderer
- entity picking
- HDR/Tonemapping & Bloom
- PBR
- ImGuizmo
- Resource system
    - load from filesystem
    - store things like Shader, Mesh, Model, etc
    - allow access to these things through ids
    - would let me "unify" my current subsystems.. each of these subsystems rn has it's own map
        would be neat to have those all under the same umbrella like d4's SNOs
- Scene graph/frustum culling 
    - https://learnopengl.com/Guest-Articles/2021/Scene/Scene-Graph
    - https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
- Space partitioning
    - OctTree (?)
    - BVH
        - raycasting
- animation
    - basic skeletal
    - IK
- get linux build on feature parity
- Event system
- Portals! (And utilizing that to make "non-euclidean" spaces)
- 3D World Wrapping (I.E. Manifold garden)
- (Visualizing) L-Systems



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
