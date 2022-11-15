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
- Postprocessing
- Job System (multithreading)
- sprite rendering
- spritesheet parsing/animating
- .obj file parsing/mesh rendering
- vector shape drawing (sqaures, circles, lines)
- deterministic randomness

## Part Of The Pack

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
- ~~Postprocessing/particle effects~~
#### Project submission
- ~~intro screen w/visuals for how to play/controls like in og game~~
- ~~Ensure builds work! Test on other machines~~
    - ~~maybe also implement a github action for automatic builds/also mac builds?~~

#### Extra
- ~~Clean up scene code with function pointers~~
- Statue activation effect (~~particles~~ & postprocessing?)
- looping music
    - would require a bit of refactoring with my use of miniaudio
- better indicate when players die... 
    - during playtesting, I died in a smoke, and couldn't figure out if I was
        dead or not
    - have score text indicate death, and put player scores in 4 corners of screen - this way
        players don't have to necessarily memorize their "player number" they can just remember
        the corner of the screen their score is in

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