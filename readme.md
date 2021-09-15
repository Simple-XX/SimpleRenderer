# SimpleRenderer

Right now, it includes

1. camera  by Euler angles
2. Phong Lighting Model
3. use the stb_image.h to read the images.
4. depth test.
5. support Lighting maps such as Diffuse maps,Specular maps and normal maps.
6. get the obj file by tiny_obj_loader.h.
7. Face culling.
8. Blending.
9. vertex shader and fragment shader.
10. bump
11. basic PBR

## Structure

The following are showing different things in the files

- s_math:
    - the simple interp function
    - the definition of `vector(x, y, z, w)` and some operations on it.
    - the definition of `vector2f(u, v)` and the operations on it.
    - the definition of `color(r, g, b, a)` and the operations on it.
    - the definition of `matrix(4 x 4)` and the operations on it.
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
    - to read the texture and the obj.

## Dependencies

### [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h)

Image loader by Sean Barrett,which I use in this project to get the texture.

### [tiny_obj_loader.h](https://github.com/tinyobjloader/tinyobjloader)

Tiny but powerful single file wavefront obj loader.

## How to build

You can download the src file and easily build this project through cmake tools with the cmakelists inside. But remember to get the images out of the file called images, as I use these images and obj as "model/xxx.obj"，so just put the files called "model" and "photo" in the build file you create for cmake....

## A few tips to get started

The render itself doesn't need to depend on any kinds of platforms. As it's developed by C/C++.  You can use it as how you use the OpenGL, however,it also needs something to output the result. In the src file, I showed how to use the `window.h` to get the result, and change the camera. You can read the `main.cpp` and see how to work with this renderer, and then drop the main.cpp, to  use it through other tools to get the output. And here am I writing to explain something for you:

### 1. Get the device

The steps may go like:

```cpp
device_t device;
device_init(&device, 800, 600, screen_fb);
// the first one is the device you are using 
// and the second and third are the width and height of the screen
// the last thing is the framebuffer of the screen
```

Then you go to set the camera.

```cpp
s_vector eye(2.19f, 0.475f, 11.15f, 1.0f), at(-1.0f, 0.0f, 0.0f, 1.0f), up(0.0f, 0.0f, 1.0f, 1.0f);
camera_at_zero(&device, eye, at, up);
// eye is where your eyes lay, at is where you are looking at and the up is just a vector which is 
// set to be up. normally let the zaxis to be the up.
```

And chose one render state.

```cpp
device.render_state = RENDER_STATE_TEXTURE;
```

### 2. Get the mesh

Set the mesh by yourself.

```cpp
vertex_t mesh[36] = {
// Positions                  // Texture Coords  //color           //rhw // Normals    // material number
{{-0.5f, -0.5f, -0.5f, 1.0f},{ 0.0f,  0.0f},{ 1.0f, 0.5f, 0.31f, 1.0f },1, { 0.0f,  0.0f,-1.0f,0.0f},1},
{{-0.5f,  0.5f, -0.5f, 1.0f},{ 0.0f,  1.0f},{ 1.0f, 0.5f, 0.31f ,1.0f}, 1,{ 0.0f,  0.0f,-1.0f,0.0f},1},
.....
```

Set the mesh from obj

```cpp
std::vector<vertex_t> tot_vertex;//set a vector to record the <vertex_t>
load_obj(tot_vertex, &device, "model/nanosuit.obj", "model", 3,1);
// the first one is the vector to record mesh.
// the second one is the device you just created.
// the 3rd one is the path of the obj,and the path of mtf file and other images.
// the 4th one is to show what the order to get outputed of the obj is.
// the 5th one is to show if you wanna silp the y-axis or not.
```

### 3. Get the texture

If you already get the obj,then you don't have to set the texture for it.
- if you are using the mesh you created yourself.

```cpp
device.material[1].have_diffuse = 1; // if it has the diffuse texture.
init_texture_by_diffuse(&device, "photo/container2.png",1);
// the device and the path and the number of the object.
device.material[1].have_specular = 1;
init_texture_by_specular(&device, "photo/container2_specular.png", 1);
```

### 4. Draw them!!

Draw the mesh you created by youself.

```cpp
int cnt = 0;
float theta = 1.0f; // the angle to rotate.
cnt++; // cnt is related to the number you put in for the object.
s_matrix m;
s_vector axis(-1.0f, -0.5f, 1.0f, 1.0f); // the axis you want to rotate on
s_vector pos(2.0f, 1.0f, 0.0f, 1.0f); // to change the pos of it
s_vector scale(1.0f, 1.0f, 1.0f, 1.0f); // to scanle the object.
m.set_rotate_translate_scale(axis, theta, pos, scale);

device->transform.world = m;
device->transform.update(); // update

device->material[cnt].shininess = 64.0f; // set the shininess.
draw_plane(device, 36, mesh, cnt);
// the number of mesh,and the mesh you are using, the number of the object.
```

Draw the obj.

```cpp
cnt++;
m.set_rotate_translate_scale(axis, theta, pos, scale);
device->transform.world = m;
device->transform.update();
draw_plane(device, tot_vertex.size(), tot_vertex, cnt);
```

Check the framebuffer to get the result.

```cpp
typedef unsigned int IUINT32;
IUINT32 cc = device.framebuffer[y][x];
int RGB_a = (cc >> 24) & 0xff;
int RGB_r = (cc >> 16) & 0xff;
int RGB_g = (cc >> 8) & 0xff;
int RGB_b = cc & 0xff;
```

Then you can get the RGB of the pos(y,x).
