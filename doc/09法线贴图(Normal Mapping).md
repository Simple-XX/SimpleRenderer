## 法线贴图
引用文章:[法线贴图](https://learnopengl-cn.github.io/05%20Advanced%20Lighting/04%20Normal%20Mapping/)</br>
&emsp;&emsp;&emsp;&emsp;[bump mapping](https://blog.csdn.net/yjr3426619/article/details/81022781)</br>
***
&emsp;&emsp;我们的场景中已经充满了多边形物体，其中每个都可能由成百上千平坦的三角形组成。我们以向三角形上附加纹理的方式来增加额外细节，提升真实感，隐藏多边形几何体是由无数三角形组成的事实。纹理确有助益，然而当你近看它们时，这个事实便隐藏不住了。现实中的物体表面并非是平坦的，而是表现出无数（凹凸不平的）细节。<br/>

&emsp;&emsp;例如，砖块的表面。砖块的表面非常粗糙，显然不是完全平坦的：它包含着接缝处水泥凹痕，以及非常多的细小的空洞。如果我们在一个有光的场景中看这样一个砖块的表面，问题就出来了。下图中我们可以看到砖块纹理应用到了平坦的表面，并被一个点光源照亮。<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/bb6a66236493478a80f0028ec05fe0d8.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAZG91c2hhYWFhYQ==,size_20,color_FFFFFF,t_70,g_se,x_16)<br/><br/>
&emsp;&emsp;光照并没有呈现出任何裂痕和孔洞，完全忽略了砖块之间凹进去的线条；表面看起来完全就是平的。我们可以使用specular贴图根据深度或其他细节阻止部分表面被照的更亮，以此部分地解决问题，但这并不是一个好方案。我们需要的是某种可以告知光照系统给所有有关物体表面类似深度这样的细节的方式。<br/>

&emsp;&emsp;如果我们以光的视角来看这个问题：是什么使表面被视为完全平坦的表面来照亮？答案会是表面的法线向量。以光照算法的视角考虑的话，只有一件事决定物体的形状，这就是垂直于它的法线向量。砖块表面只有一个法线向量，表面完全根据这个法线向量被以一致的方式照亮。如果每个fragment都是用自己的不同的法线会怎样？这样我们就可以根据表面细微的细节对法线向量进行改变；这样就会获得一种表面看起来要复杂得多的幻觉：<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://learnopengl-cn.github.io/img/05/04/normal_mapping_surfaces.png)<br/><br/>

&emsp;&emsp;每个fragment使用了自己的法线，我们就可以让光照相信一个表面由很多微小的（垂直于法线向量的）平面所组成，物体表面的细节将会得到极大提升。这种每个fragment使用各自的法线，替代一个面上所有fragment使用同一个法线的技术叫做法线贴图（normal mapping）或凹凸贴图（bump mapping）。<br/>



### 法线贴图
***
为使法线贴图工作，我们需要为每个fragment提供一个法线。像diffuse贴图和specular贴图一样，我们可以使用一个2D纹理来储存法线数据。2D纹理不仅可以储存颜色和光照数据，还可以储存法线向量。这样我们可以从2D纹理中采样得到特定纹理的法线向量。
```bash
     device.material[1].have_normal = 1;

	init_texture_by_normal(&device, "path", 1);
```
由于法线向量是个几何工具，而纹理通常只用于储存颜色信息，用纹理储存法线向量不是非常直接。如果你想一想，就会知道纹理中的颜色向量用r、g、b元素代表一个3D向量。类似的我们也可以将法线向量的x、y、z元素储存到纹理中，代替颜色的r、g、b元素。法线向量的范围在-1到1之间，所以我们先要将其映射到0到1的范围：

```bash
   s_vector rgb_normal = normal * 0.5 + 0.5; // 从 [-1,1] 转换至 [0,1]
```

将法线向量变换为像这样的RGB颜色元素，我们就能把根据表面的形状的fragment的法线保存在2D纹理中。教程开头展示的那个砖块的例子的法线贴图如下所示：<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://learnopengl-cn.github.io/img/05/04/normal_mapping_normal_map.png)<br/><br/>

这会是一种偏蓝色调的纹理（你在网上找到的几乎所有法线贴图都是这样的）。这是因为所有法线的指向都偏向z轴（0, 0, 1）这是一种偏蓝的颜色。法线向量从z轴方向也向其他方向轻微偏移，颜色也就发生了轻微变化，这样看起来便有了一种深度。例如，你可以看到在每个砖块的顶部，颜色倾向于偏绿，这是因为砖块的顶部的法线偏向于指向正y轴方向（0, 1, 0），这样它就是绿色的了。<br/>

从法线贴图范围[0,1]获取法线，将法线向量转换为范围[-1,1]<br/>
```bash
if (device->material[count].have_normal == 1)
		{
			
			s_vector norm;
			read_the_texture(norm, &device->material[count].normal_texture, u, v);
			norm.float_dot(2.0f);
			s_vector tmp_1(1.0f, 1.0f, 1.0f, 1.0f);
			norm.minus_two(norm, tmp_1);
			norm.normalize();
             }
```

得到的结果如下图<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/ddbd91916edd436fa88687208421d427.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAZG91c2hhYWFhYQ==,size_20,color_FFFFFF,t_70,g_se,x_16)<br/><br/>

&emsp;&emsp;但是光照看起来完全不正确，所有法线都指向z方向，它们本该朝着表面法线指向y方向的。一个可行方案是为每个表面制作一个单独的法线贴图。如果是一个立方体的话我们就需要6个法线贴图，但是如果模型上有无数的朝向不同方向的表面，这就不可行了（译注：实际上对于复杂模型可以把朝向各个方向的法线储存在同一张贴图上，你可能看到过不只是蓝色的法线贴图，不过用那样的法线贴图有个问题是你必须记住模型的起始朝向，如果模型运动了还要记录模型的变换，这是非常不方便的；此外就像作者所说的，如果把一个diffuse纹理应用在同一个物体的不同表面上，就像立方体那样的，就需要做6个法线贴图，这也不可取）。<br/>

&emsp;&emsp;另一个稍微有点难的解决方案是，在一个不同的坐标空间中进行光照，这个坐标空间里，法线贴图向量总是指向这个坐标空间的正z方向；所有的光照向量都相对与这个正z方向进行变换。这样我们就能始终使用同样的法线贴图，不管朝向问题。这个坐标空间叫做切线空间（tangent space）。<br/>
### 切线空间
***
&emsp;&emsp;在法线贴图中，需要提供两张纹理——材质纹理和法线纹理，而在视差贴图和浮雕贴图中，还需要提供一张高度图纹理。法线纹理的作用是提供一个法线信息，纹理中对应的rgb的颜色值分别代表法线向量的分量，但是，这个向量是在什么空间下的向量？首先可以设想一下，这个向量如果是世界坐标系下的合不合理？很显然，在实际的渲染中，物体是有可能作变换的，如果一个物体进行了旋转变换，那么它的法线也要作相应的变换，法线纹理存的信息就不可以直接使用，要做相应的变换。似乎这种想法并不是最优的，但如果场景中有很多静态的物体也可以考虑使用。还有一种想法是存储在模型空间下的向量，这种想法似乎可行，并且比世界坐标系下的要好很多，但是这种方法太过于依赖模型本身的细节，如果模型发生了形变，这个向量依旧需要做变换，而且复用率不高，一个模型的法线纹理不好用在其他的模型上。<br/>
&emsp;&emsp;那么一种更好的方式，也是绝大多数凹凸贴图中的实现方式是让法线纹理存储在切线空间下的坐标，切线空间，肯定是跟切线有关的一个空间。对于一个顶点，它的法线方向是已知的，现在需要构造一个坐标系，让这个法线方向为Z轴，再选取两条过该点的切线，作为另外两个轴，其中一条称为Tangent，另一条称为Bit-Tangent，即切线空间的基为TBN，问题是，如何选这两条切线？与法线垂直的直线有无数条，构成了一个切平面，应该选取这个平面上哪两条互相垂直的直线？对于三角网络，可以想到利用纹理坐标（u,v）来构造，关于具体的构造方法我查阅的相关资料似乎都略有不同，具体探讨可见[为什么要有切线空间（Tangent Space），它的作用是什么？](https://www.zhihu.com/question/23706933)<br/>
使用切线空间的好处：<br/>
1.自由度高，独立于模型而存在，可以尝试用在不同的网格模型上<br/>
2.可以复用，例如一个正方体的六个面完全可以使用相同的切线坐标<br/>
3.纹理可压缩，切线空间下的Z坐标往往都是正值，因此可以在内存中只存储X、Y的坐标值，通过模长为1来计算Z的坐标值<br/>
&emsp;&emsp;事实上，无论采用哪种空间，只要最终能够让视线向量、光线向量、法线向量统一在一个线性空间下做与光照相关的运算就可以了，因此也没有必要拘泥于到底是哪个空间下好。但这并不意味着最终统一在哪个空间下不重要，注意这里所说的是法线信息存储在哪个空间下并不需要太钻牛角尖，而实际运算中，究竟是要将这三者统一在切线空间还是世界空间下，或者别的空间下是非常重要的。<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdn.net/20180712165941950)<br/><br/>

#### 法线贴图原理及实现
***
&emsp;&emsp;如前文所述，法线贴图的原理就是直接指定顶点在切线空间下的法线位置，用在材质纹理下的颜色信息和法线纹理下的法线信息来做光照的相关运算。实际编码最重要的一个环节就是将视线、法线、光线统一到一个空间下。有两种方式：一是将切线空间下的法线变换到世界空间下，然后直接利用世界空间下的光线、视线做运算，这种方法比较简明，但是性能略显不足，注意对于每一个像素的法线信息都是要靠插值求得的，也就是说，这样处理每个像素的法线都要做一次线性变换到世界空间下，它的计算是逐像素的；另一种方法是将世界空间下的视线、光线都变换到切线空间下，这种处理只需要逐顶点计算。<br/>
下面我们采用第一种方式来计算，比较好处理：<br/>
现在假设我们有了TBN的值，构造了如下矩阵![图片](https://img-blog.csdnimg.cn/15624bdfe4374e7d858224a634f5963e.png)这是从切线空间到世界空间的过渡矩阵，而由过渡矩阵和坐标变换的关系可知，<br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/bc4c827e0591498c9048b57542a3b5d0.png)<br/>
如果我们把向量以行向量的形式表示，就可以得到<br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/47553cd2006d4e32b8f70ffbafa131c0.png)<br/>
其中&emsp;![图片](https://img-blog.csdnimg.cn/77bd6159059646c3804b894cc854dbdd.png)是世界空间下的法向量<br/>

&emsp;![图片](https://img-blog.csdnimg.cn/d22e39f4e0244a23a93385c0d012ca4b.png)是切线空间下的法线方向。<br/>
代码如下
```bash
      //in v_shader
      s_vector tmp1(vv->tangent.x, vv->binormal.x, vv->normal.x, 1.0f);
	ff->storage0 = tmp1;

	tmp1.reset(vv->tangent.y, vv->binormal.y, vv->normal.y, 1.0f);
	ff->storage1 = tmp1;

	tmp1.reset(vv->tangent.z, vv->binormal.z, vv->normal.z, 1.0f);
	ff->storage2 = tmp1;

     //then in f_shader
    if (device->material[count].have_normal == 1)
		{
			
			s_vector norm;
			read_the_texture(norm, &device->material[count].normal_texture, u, v);
			norm.float_dot(2.0f);
			s_vector tmp_1(1.0f, 1.0f, 1.0f, 1.0f);
			norm.minus_two(norm, tmp_1);
			norm.normalize();
			norm.reset(ff->storage0.dotproduct(norm), ff->storage1.dotproduct(norm), ff->storage2.dotproduct(norm), 1.0f);
			norm.normalize();
			N = norm;
		}

```
<br/>看起来是正确的法线贴图<br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/5c579865737940c38d62c558b3b03071.png?x-oss-process=image/watermark,type_ZHJvaWRzYW5zZmFsbGJhY2s,shadow_50,text_Q1NETiBAZG91c2hhYWFhYQ==,size_16,color_FFFFFF,t_70,g_se,x_16)<br/>