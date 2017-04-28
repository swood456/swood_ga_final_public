# Stephen Wood Game Archetecture Final Project
## Cloth Simulation

Cloth component simulates a square piece of cloth

## Controls:
* __wasd__ - move camera (preexisting)
* __p__ pause simulation (preexisting)
* __z__ reset cloth positions
* __r__ increase cloth structural spring constant
* __f__ decrease cloth structural spring constant
* __t__ increase cloth sheer spring constant
* __g__ decrease cloth sheer spring constant
* __y__ increase cloth bend spring constant
* __h__ decrease cloth bend spring constant
* __u / o__ rotate the textured cube
* __ijkl__ translate the textured cube

## Files that were changed:
* __src/engine/main.cpp__: Updated main to have a bunch of different cloth components that can be commented in and out. Also have simple GUI elements to display framerate and spring constants
* __src/engine/physics/ga_cloth_component.h and .cpp__: Main cloth simulation code
* __src/engine/graphics/ga_material__: added in phong_color_material, which is the material used for the cloth
* __src/engine/entity/ga_lua_component.h__: added in simple ijkl movement and rotation using u and o
* __data/shaders/ga_phong_color shaders__: shaders used for phong lighting on a solid color.