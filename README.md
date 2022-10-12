# POTP

## Credits:
Khronos Group for OpenGL
____ for GLAD
___ for GLFW
____ for miniaudio
Christian Vallentin for GLText
Sergey Kosarevsky for PoissonGenerator
TDingle for statue sprites and music


# tiny_engine

## Info

Libraries Used:
- OpenGL
- GLAD
- GLFW
- miniaudio
- [GLtext](https://github.com/vallentin/glText) by Christian Vallentin 
- [PoissonGenerator](https://github.com/corporateshark/poisson-disk-generator) by Sergey Kosarevsky

Features:
- deterministic randomness
- custom spritesheet parsing
- vector shape drawing

## TODO

# CURRENT TASK:
- ensure multiple player input is clean and working to get MVP

### MVP
- clip ninja behind statue when it's base y is greater than statues base y

### More...
- Controller support
- statue activation/win conditions

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