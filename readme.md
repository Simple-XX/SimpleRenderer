# SimpleRenderer
***
right now it includes</br>
1.camera  by Euler angles</br>
2.Phong Lighting Model</br>
3.use the stb_image.h to read the images.</br>
4.depth test.</br>
5.support Lighting maps such as Diffuse maps and Specular maps.</br>
6.get the obj file by tiny_obj_loader.h.</br>
7.Face culling.</br>
8.Blending.</br>
9.vertex shader and fragment shader.</br>
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

### [stb_image.h](https://github.com/nothings/stb/blob/master/stb_image.h)
&emsp;&emsp;image loader by Sean Barrett,which I use in this project to get the texture.</br></br>

### [tiny_obj_loader.h](https://github.com/tinyobjloader/tinyobjloader)
&emsp;&emsp;Tiny but powerful single file wavefront obj loader.

## How to build

&emsp;&emsp;You can download the src file and easily build this project through cmake tools with the cmakelists inside. but remember to get the images out of the file called images, as I use these images and obj as "model/xxx.obj"，so just put the files called "model" and "photo" in the build file you create for cmake....