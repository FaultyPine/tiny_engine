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
- INPUT REFACTOR!!!
    - move game-specific code into game, generalize tiny_engine input api
    - create abstraction layers for an 'input device' and make an array of 4 of those
    - connected controllers will be the 'later' players.. 
        I.E. if 1 controller connected and player count is set to 2, keyboard is p1 and controller is p2
        if 2 controllers connected and player count is 4, keyboard is p1/p2, and controllers are p3/p4
        This is so keyboard can always have a max of two players since only two people can realistically use one keyboard at once
    - if implemented properly, game code shouldn't have to change. We still just check the button of a player idx
https://www.glfw.org/docs/latest/input_guide.html

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