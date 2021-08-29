&emsp;&emsp;I have already drawn a mesh for you,you can change it in the main.cpp if you want.<br/>
&emsp;&emsp;
   first you set a device,the steps may go like<br/>
```bash
  device_t device;
  device_init(&device, 800, 600, screen_fb);
  //the first one is the device you are using 
  // and the second and third are the width and height of the screen
  //the last thing is the framebuffer of the screen
  
```
   <br/>the you go to set the camera.<br/>
```bash
    s_vector eye(2.19f, 0.475f, 11.15f, 1.0f), at(-1.0f, 0.0f, 0.0f, 1.0f), up(0.0f, 0.0f, 1.0f, 1.0f);
	camera_at_zero(&device, eye, at, up);
    //eye is where your eyes lay, at is where you are looking at and the up is just a vector which is 
    //set to be up. normally let the zaxis to be the up.

```
<br/>and get the texture in, and chose one render state<br/>
```bash
    	init_texture(&device);
	device.render_state = RENDER_STATE_TEXTURE;
```
to draw something such as a box.
at the end of the round <br/>"while (screen_exit == 0 && screen_keys[VK_ESCAPE] == 0)",<br/>you can add things such as<br/>
```bash
    draw_box(&device, alpha);
    //alpha is the angle you want it to rotate at original
```
<br/>As I already done the camera for you,you can push the up,down,left,right button to get moved in the scence,and press the space key to change render state. right now there are texture,color and wireframe for you to chose.<br/>

you can check the main.cpp where I already shown one.