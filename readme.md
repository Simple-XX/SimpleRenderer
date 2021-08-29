#SimpleRenderer
***
## Structure
the following are showing different things in the files
- s_math:
    - the simple interp function
    - the definition of vector(x,y,z,w) and some operations on it.
    - the definition of vector2f(u,v) and the operations on it.
    - the definition of color(r,g,b,a) and the operations on it.
    - the definition of matrix(4X4) and the operations on it.
- rasterize:
    - functions such as set the look at matrix,the perspective matrix and orthographic matrix.
    - the transform matrix which record and update the changes.
    - function to split the triangle.
    - functions to get the scanline.
- render:
    - the definition of device and some operations on it.   
    - the functions to compute Barycenter,tangent and binormal.
    - the rendering trap.
    - the vertex and fragment shaders.



## Dependencies

###[windows.h](https://en.wikipedia.org/wiki/Windows.h)

&emsp;&emsp;Right now the project is running on windows,and it's using api from windows to get the result.

## The simple way to get started
&emsp;&emsp;I have already drawn a mesh for you,you can change it in the main.cpp if you want.
&emsp;&emsp;
   first you set a device,the steps may go like
```bash
  device_t device;
  device_init(&device, 800, 600, screen_fb);
  //the first one is the device you are using 
  // and the second and third are the width and height of the screen
  //the last thing is the framebuffer of the screen
  
```
   the you go to set the camera.
```bash
    s_vector eye(2.19f, 0.475f, 11.15f, 1.0f), at(-1.0f, 0.0f, 0.0f, 1.0f), up(0.0f, 0.0f, 1.0f, 1.0f);
	camera_at_zero(&device, eye, at, up);
    //eye is where your eyes lay, at is where you are looking at and the up is just a vector which is 
    //set to be up. normally let the zaxis to be the up.

```
and get the texture in, and chose one render state
```bash
    	init_texture(&device);
	device.render_state = RENDER_STATE_TEXTURE;
```
to draw something such as a box.
at the end of the round "while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0)",you can add things such as
```bash
    draw_box(&device, alpha);
    //alpha is the angle you want it to rotate at original
```
As I already done the camera for you,you can push the up,down,left,right button to get moved in the scence,and press the space key to change render state. right now there are texture,color and wireframe for you to chose.
## How to build

&emsp;&emsp;You can download the src file and easily build this project through cmake tools with the cmakelists inside.