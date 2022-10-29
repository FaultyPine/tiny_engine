# POTP

## Credits:
Khronos Group for OpenGL  
Dav1dde for GLAD  
GLFW contributors for GLFW  
mackron for miniaudio  
Christian Vallentin for GLText  
Sergey Kosarevsky for PoissonGenerator  
TDingle for statue sprites and music  
Sebastian Belalcazar for keyboard icon  
Good Ware for controller icon  
joalfa for X icon  

# tiny_engine

## Info

Libraries Used:
- [OpenGL](https://www.opengl.org/) by Khronos Group
- [GLAD](https://glad.dav1d.de/) by David Herberth
- [GLFW](https://www.glfw.org/)
- [miniaudio](https://github.com/mackron/miniaudio) by David Reid
- [GLtext](https://github.com/vallentin/glText) by Christian Vallentin 
- [PoissonGenerator](https://github.com/corporateshark/poisson-disk-generator) by Sergey Kosarevsky
- [tiny_obj_loader](https://github.com/tinyobjloader/tinyobjloader)

Features:
- deterministic randomness
- custom spritesheet parsing
- vector shape drawing
- .obj file parsing

## Part Of The Pack

1. Of the goals for milestone 2 you set in your main project milestone 1, what were you able to accomplish? Do a brief tour of what you accomplished in the past week. It's a good idea to have your goals list open, switching back and forth in the screen recording.
2. What snags are you running into? Describe any problems you are having, or expect to be having in the near future. 
Goal-setting for milestone 
3. Goal-setting for milestone 3. What are the goals that you feel must get done for milestone 2, and what goals do you hope to get also get done?

### MVP

#### Milestone 1
- ~~Display graphics onscreen~~
- ~~(moving) Sprites~~
#### Milestone 2
- ~~Spritesheet animation~~
- ~~Audio~~
- ~~Text~~
- ~~Ninja Punching/dying~~
- ~~Two (or more) players can play~~
- ~~Win condition: Player ninjas killed~~
- ~~Smoke grenades~~
- ~~Restart after game is won~~
- ~~Win condition: Statue activation~~
#### Milestone 3
- ~~Controller Support~~
- ~~Controller "setup" scene~~
- ~~Ninja punch & statue activation SFX~~
- ~~Handle window resizing~~
#### Milestone 4
- ~~Title Screen~~
- ~~Postprocessing~~/particle effects
#### Project submission
- ~~intro screen w/visuals for how to play/controls like in og game~~

#### Extra
- ~~Clean up scene code with function pointers~~


### Stretch Goals
- Implement Z indexing
    - have a list of entity pointers and iterate through them in our main draw function calling draw and using the entity's z index to draw in the correct order
    - clip ninja behind statue when it's base y is greater than statues base y
- vfx for statues/ninjas
- Main Menu
- Settings menu
- Controls menu


### Possible Future Engine Features:
- ~~debug line drawing~~
- ~~Postprocessing~~
- Event system
- ~~Job System (threading)~~
    - lock free queue: https://jbseg.medium.com/lock-free-queues-e48de693654b
- Intrusive profiling API - using chrome://tracing as the visualizing backend
    - https://perfetto.dev/docs/instrumentation/tracing-sdk