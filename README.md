# POTP

## Credits:
Khronos Group for OpenGL
____ for GLAD
___ for GLFW
____ for miniaudio
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

### Current Task


### TODO
- statue activation/win condition
- clean up debug line drawing
- Implement Z indexing
    - have a list of entity pointers and iterate through them in our main draw function calling draw and using the entity's z index to draw in the correct order
    - clip ninja behind statue when it's base y is greater than statues base y

### Stretch Goals
- intro screen w/visuals for how to play/controls like in og game
- vfx for statues/ninjas
- Main Menu
- Settings
- Controls menu


## Notes

- scratch these for now, they aren't necessary for the pretty small game im building... don't overengineer
- Add event/signal system!
- Collision system should be rewritten to be based on events/signals