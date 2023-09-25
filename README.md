# tiny_engine

## Info

Libraries Used:
- [OpenGL](https://www.opengl.org/) by Khronos Group
- [GLAD](https://glad.dav1d.de/) by David Herberth
- [GLFW](https://www.glfw.org/)
- [miniaudio](https://github.com/mackron/miniaudio) by David Reid
- [GLtext](https://github.com/vallentin/glText) by Christian Vallentin 
- [imgui](https://github.com/ocornut/imgui) ImGui by Omar
- [metadesk](https://dion.systems/metadesk) Metadesk by Dion Systems

Features/Included Implementations:
- type reflection/metadata system
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
- Shader hot reloading & #include parser

Written with this engine:  
[Part Of The Pack](https://faultypine.itch.io/part-of-the-pack)  
![image](https://user-images.githubusercontent.com/53064235/211900902-6c979f44-357a-4449-adf0-40598d9a7edb.png)  
Simple nature scene I used to implement & test 3D features  
![Screenshot 2023-04-26 235502](https://user-images.githubusercontent.com/53064235/234755544-509599a3-1df1-4656-bcd0-6be7c841b4fb.png)


QuadTree Implementation:  

https://user-images.githubusercontent.com/53064235/216891116-b9521259-7010-4fa0-bd52-3d54e71d830e.mp4  

Outline/Sketch Effect  

https://user-images.githubusercontent.com/53064235/232272366-69ce9e2f-c13b-4db3-8781-e4ad967f1b2d.mp4  
  



### TODO/Topics to Experiment With:
- Reflection with Metadesk
- refactor shaders to use includes
- Resource system
    - load from filesystem
    - store things like Shader, Mesh, Model, etc objects
    - allow access to these things through ids or shared pointers
- serialize objects... I.E. store positions and whatnot so im not just hardcoding them
    - will also allow me to change them while in-engine and save those changes
    - use rel pointers/arrays to make everything one big blob
- Custom allocators (arenas)
- better grass sway w/distortion texture to emulate wind better
- PBR
- HDR/Tonemapping & Bloom
- Scene graph/frustum culling 
    - https://learnopengl.com/Guest-Articles/2021/Scene/Scene-Graph
    - https://learnopengl.com/Guest-Articles/2021/Scene/Frustum-Culling
- Space partitioning (BSP/Quadtree/Octree)
- animation
    - .obj files don't support animation... might need to implement gltf or collada or fbx support
    - https://learnopengl.com/Guest-Articles/2020/Skeletal-Animation
    - https://www.youtube.com/watch?v=f3Cr8Yx3GGA
- Perforce?
- Portals! (And utilizing that to make "non-euclidean" spaces)
- 3D World Wrapping (I.E. Manifold garden)
- (Visualizing) L-Systems
- get linux build on feature parity
- in general, research good debugging tools...
    - https://werat.dev/blog/what-a-good-debugger-can-do/
    - https://rr-project.org/  (get this compiling under linux? Did, but weird mouse issues...)
- Event system



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
