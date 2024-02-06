# Minecraft2D

Minecraft2D is a basic isometric version of Minecraft made in C!
It uses the Windows.h library to help manage the window and to let me manipulate the pixels in the window.
No other graphic libraries are used, except my own :)
All "block textures" are simply 3 rgb values for each side, mixed up by a custom noise function.

Blocks such as water and sand have physics! Try what you can do with them :)

## Controls:

**WASD:** Move the cursor on a 2D plain

**Left Click** Place the block you're currently holding at the position of the cursor.

**Right Click** Remove the block the cursor is on.

**Shift:** Moves the cursor up

**Control:** Moves the cursor down

**Scroll Wheel:** Cycles between blocks

## Worlds:

I use random seed-based world generation to create the world. The entire bottom layer (z=0) and some of the second layer (z=1) are made out of bedrock, this block cannot be broken. 
After this, a part of the world is filled up with stone, The placement of the stone varies for every seed. After this, some sand and grass is added onto the stone, the thickness of this layer is once again map dependent.
Trees, are also randomly scattered about, although I'd like to add some leafs in the future.

## Blocks:

**Bedrock:** This block makes up the lowest layer. It cannot be broken or replaced by other blocks.

**Stone:** This block makes up most of the middle layer of the ground.

**Grass:** Grass and sand make up the top layer. Grass' top texture is completely different from the sides.

**Sand:** Can occasionally be found at the top layer. It'll slowly fall if there's no block underneath it.

**Water:** Upon placing a water source block, Smaller blocks will spil out in all directions, the further away from the block, the lower the water block will become. Water can also flow down. Examples can be found in the screenshots below.

**Wood:** Dead trees made out of wood can spawn naturally. 

## Screenshots:

![image](https://github.com/Aliijjn/Minecraft2D/assets/114729493/f94c1d19-84da-4956-be02-9a37bc0c0cf3)

![image](https://github.com/Aliijjn/Minecraft2D/assets/114729493/59f849f5-6aa3-40f3-9143-cf2660c8ad33)

![image](https://github.com/Aliijjn/Minecraft2D/assets/114729493/539a5abf-9e83-4ebf-a079-f4fbdab41109)

###  Made by Alijn Kuijer, 2023
