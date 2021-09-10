## PBR 基础理论
引用文章:[Theory](https://learnopengl-cn.github.io/07%20PBR/01%20Theory/)</br>

***
由于内容繁冗，这里只对使用的公式进行总结。 较为详细的理论可以见引用文章：</br>

**PBR**，或者用更通俗一些的称呼是指基于**物理的渲染(Physically Based Rendering)**，它指的是一些在不同程度上都基于与现实世界的物理原理更相符的基本理论所构成的渲染技术的集合。</br>

判断一种PBR光照模型是否是基于物理的，必须满足以下三个条件：</br>
* 基于微平面(Microfacet)的表面模型。</br>
* 能量守恒。</br>
* 应用基于物理的BRDF</br>

在这次的PBR系列教程之中，我们将会把重点放在最先由迪士尼(Disney)提出探讨并被Epic Games首先应用于实时渲染的PBR方案。</br>

### 反射率方程
***
在这里我们引入了一种被称为渲染方程(Render Equation)的东西。它是某些聪明绝顶人所构想出来的一个精妙的方程式，是如今我们所拥有的用来模拟光的视觉效果最好的模型。基于物理的渲染所坚定的遵循的是一种被称为反射率方程(The Reflectance Equation)的渲染方程的特化版本。要正确的理解PBR 很重要的一点就是要首先透彻的理解反射率方程：<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/54d67a6f693747dab5a7adf34515f198.png)<br/><br/>
**辐射率(Radiance)**，在这里用L来表示。辐射率被用来量化单一方向上发射来的光线的大小或者强度。由于辐射率是由许多物理变量集合而成的，一开始理解起来可能有些困难，因此我们首先关注一下这些物理量：<br/>
* 辐射通量<br/>
辐射通量Φ表示的是一个光源所输出的能量，以瓦特为单位。光是由多种不同波长的能量所集合而成的，而每种波长则与一种特定的（可见的）颜色相关。因此一个光源所放射出来的能量可以被视作这个光源包含的所有各种波长的一个函数。波长介于390nm到700nm（纳米）的光被认为是处于可见光光谱中，也就是说它们是人眼可见的波长。在下面你可以看到一幅图片，里面展示了日光中不同波长的光所具有的能量：<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://learnopengl-cn.github.io/img/07/01/daylight_spectral_distribution.png)<br/><br/>
辐射通量将会计算这个由不同波长构成的函数的总面积。直接将这种对不同波长的计量作为参数输入计算机图形有一些不切实际，因此我们通常不直接使用波长的强度而是使用三原色编码，也就是RGB（或者按通常的称呼：光色）来作为辐射通量表示的简化。这套编码确实会带来一些信息上的损失，但是这对于视觉效果上的影响基本可以忽略。
* 立体角<br/>
&emsp;&emsp;立体角用ω表示，它可以为我们描述投射到单位球体上的一个截面的大小或者面积。投射到这个单位球体上的截面的面积就被称为立体角(Solid Angle)，你可以把立体角想象成为一个带有体积的方向：<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://learnopengl-cn.github.io/img/07/01/solid_angle.png)<br/><br/>
可以把自己想象成为一个站在单位球面的中心的观察者，向着投影的方向看。这个投影轮廓的大小就是立体角。<br/><br/>
* 辐射强度<br/>
&emsp;&emsp;辐射强度(Radiant Intensity)表示的是在单位球面上，一个光源向每单位立体角所投送的辐射通量。举例来说，假设一个全向光源向所有方向均匀的辐射能量，辐射强度就能帮我们计算出它在一个单位面积（立体角）内的能量大小：<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://learnopengl-cn.github.io/img/07/01/radiant_intensity.png)<br/><br/>
计算辐射强度的公式如下所示：<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/ebc3e612947d488a8c06135bc3628b79.png)<br/><br/>
其中I表示辐射通量Φ除以立体角ω。<br/><br/>

在理解了辐射通量，辐射强度与立体角的概念之后，我们终于可以开始讨论辐射率的方程式了。这个方程表示的是，一个拥有辐射强度Φ的光源在单位面积A，单位立体角ω上的辐射出的总能量：<br/><br/>
&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/55b0268e8855437e8df5d07d64baf40d.png)<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://learnopengl-cn.github.io/img/07/01/radiance.png)<br/><br/>
辐射率是辐射度量学上表示一个区域平面上光线总量的物理量，它受到入射(Incident)（或者来射）光线与平面法线间的夹角θ的余弦值cosθ的影响：当直接辐射到平面上的程度越低时，光线就越弱，而当光线完全垂直于平面时强度最高。这和我们在前面的基础光照教程中对于漫反射光照的概念相似，其中cosθ就直接对应于光线的方向向量和平面法向量的点积<br/><br/>
现在唯一剩下的未知符号就是fr了，它被称为BRDF，或者双向反射分布函数(Bidirectional Reflective Distribution Function) ，它的作用是基于表面材质属性来对入射辐射率进行缩放或者加权。

#### BRDF
***







