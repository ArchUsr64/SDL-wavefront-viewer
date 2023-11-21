# SDL_Wavefront_Viewer
A basic program to display obj wireframe on the screen with no hardware acceleration<br>
![image](https://github.com/ArchUsr64/SDL_Wavefront_Viewer/assets/83179501/b2d7747e-5893-4344-bd69-9efb1e7aeff5)

**NOTE: Only for models with triangle faces**
### Controls are as follows:
Arrow keys or w, a, s, d for moving the model on the screen<br>
Page Up and Page Down for changing the size of the model<br>
Shift and Space to move the model close or farther from the camera<br>
Tab to toggle the rotation axis of the model<br>
### Directions to use:
Rename the wavefront as `untitled.obj` and move it to the same directory where you cloned this repository(the repository has a teapot shaped wavefront by default, make sure to delete that if you want to use a custom one).<br>
On a linux system with [SDL](https://www.libsdl.org) and [GCC](https://gcc.gnu.org/) installed, run the command `g++ -lSDL2 main.cpp -o executable && ./executable` inside the directory where you cloned the repository.
For other operating systems, compile and run the program with [SDL](https://www.libsdl.org/) installed.<br>
