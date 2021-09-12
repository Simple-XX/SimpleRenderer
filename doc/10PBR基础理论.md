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
&emsp;&emsp;BRDF，或者说**双向反射分布函数**，它接受入射（光）方向ωi，出射（观察）方向ωo，平面法线n以及一个用来表示微平面粗糙程度的参数a作为函数的输入参数。BRDF可以近似的求出每束光线对一个给定了材质属性的平面上最终反射出来的光线所作出的贡献程度。举例来说，如果一个平面拥有完全光滑的表面（比如镜面），那么对于所有的入射光线ωi（除了一束以外）而言BRDF函数都会返回0.0 ，只有一束与出射光线ωo拥有相同（被反射）角度的光线会得到1.0这个返回值。<br/>

&emsp;&emsp;BRDF基于我们之前所探讨过的微平面理论来近似的求得材质的反射与折射属性。对于一个BRDF，为了实现物理学上的可信度，它必须遵守能量守恒定律，也就是说反射光线的总和永远不能超过入射光线的总量。严格上来说，同样采用ωi和ωo作为输入参数的 Blinn-Phong光照模型也被认为是一个BRDF。然而由于Blinn-Phong模型并没有遵循能量守恒定律，因此它不被认为是基于物理的渲染。现在已经有很好几种BRDF都能近似的得出物体表面对于光的反应，但是几乎所有实时渲染管线使用的都是一种被称为Cook-Torrance BRDF模型。<br/>

Cook-Torrance BRDF兼有漫反射和镜面反射两个部分：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/fb64dfd5f93f41639d0ffbc10dfec7b8.png)<br/><br/>
&emsp;&emsp;这里的kd是早先提到过的入射光线中被折射部分的能量所占的比率，而ks是被反射部分的比率。BRDF的左侧表示的是漫反射部分，这里用flambert来表示。它被称为**Lambertian漫反射**，这和我们之前在漫反射着色中使用的常数因子类似，用如下的公式来表示：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/0c0414e27e7249c5af12f7ebcad13ba1.png)<br/><br/>
c 表示表面颜色（回想一下漫反射表面纹理）。除以π是为了对漫反射光进行标准化，因为前面含有BRDF的积分方程是受π影响的.<br/>
> 你也许会感到好奇，这个Lambertian漫反射和我们之前经常使用的漫反射到底有什么关系：之前我们是用表面法向量与光照方向向量进行点乘，然后再将结果与平面颜色相乘得到漫反射参数。点乘依然还在，但是却不在BRDF之内，而是转变成为了Lo积分末公式末尾处的n⋅ωi 。

目前存在着许多不同类型的模型来实现BRDF的漫反射部分，大多看上去都相当真实，但是相应的运算开销也非常的昂贵。不过按照Epic公司给出的结论，Lambertian漫反射模型已经足够应付大多数实时渲染的用途了。<br/>

&emsp;&emsp;BRDF的镜面反射部分要稍微更高级一些，它的形式如下所示：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/bcadb870dbfc4bd996cdf6ceddc77a7c.png)<br/><br/>
Cook-Torrance BRDF的镜面反射部分包含三个函数，此外分母部分还有一个标准化因子 。字母D，F与G分别代表着一种类型的函数，各个函数分别用来近似的计算出表面反射特性的一个特定部分。三个函数分别为正态分布函数(Normal **D**istribution Function)，菲涅尔方程(**F**resnel Rquation)和几何函数(**G**eometry Function)：<br/>
* **正态分布函数**：估算在受到表面粗糙度的影响下，取向方向与中间向量一致的微平面的数量。这是用来估算微平面的主要函数。 <br/>
正态分布函数D，或者说镜面分布，从统计学上近似的表示了与某些（中间）向量h取向一致的微平面的比率。举例来说，假设给定向量h，如果我们的微平面中有35%与向量h取向一致，则正态分布函数或者说NDF将会返回0.35。目前有很多种NDF都可以从统计学上来估算微平面的总体取向度，只要给定一些粗糙度的参数以及一个我们马上将会要用到的参数Trowbridge-Reitz GGX：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/e23cac3164de4280853e47ae8dd38a25.png)<br/><br/>
在这里h表示用来与平面上微平面做比较用的中间向量，而a表示表面粗糙度。<br/>

* **几何函数**：描述了微平面自成阴影的属性。当一个平面相对比较粗糙的时候，平面表面上的微平面有可能挡住其他的微平面从而减少表面所反射的光线。
<br/> 与NDF类似，几何函数采用一个材料的粗糙度参数作为输入参数，粗糙度较高的表面其微平面间相互遮蔽的概率就越高。我们将要使用的几何函数是GGX与Schlick-Beckmann近似的结合体，因此又称为Schlick-GGX：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/ac411ee7165c48428a3ed74208baba9b.png)<br/><br/>
这里的k是α基于几何函数是针对直接光照还是针对IBL光照的重映射(Remapping) :<br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/ae43eb6cc1834996b94578c49bc8fbe3.png)<br/><br/>
为了有效的估算几何部分，需要将观察方向（几何遮蔽(Geometry Obstruction)）和光线方向向量（几何阴影(Geometry Shadowing)）都考虑进去。我们可以使用史密斯法(Smith’s method)来把两者都纳入其中：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/5fe5d58ed6e1416a968a91c4e8e8bb27.png)<br/><br/>
* **菲涅尔方程** &emsp;菲涅尔（发音为Freh-nel）方程描述的是被反射的光线对比光线被折射的部分所占的比率，这个比率会随着我们观察的角度不同而不同。当光线碰撞到一个表面的时候，菲涅尔方程会根据观察角度告诉我们被反射的光线所占的百分比。利用这个反射比率和能量守恒原则，我们可以直接得出光线被折射的部分以及光线剩余的能量。<br/>

当垂直观察的时候，任何物体或者材质表面都有一个基础反射率(Base Reflectivity)，但是如果以一定的角度往平面上看的时候所有反光都会变得明显起来。你可以自己尝试一下，用垂直的视角观察你自己的木制/金属桌面，此时一定只有最基本的反射性。但是如果你从近乎90度（译注：应该是指和法线的夹角）的角度观察的话反光就会变得明显的多。如果从理想的90度视角观察，所有的平面理论上来说都能完全的反射光线。这种现象因菲涅尔而闻名，并体现在了菲涅尔方程之中。<br/>

&emsp;&emsp;菲涅尔方程是一个相当复杂的方程式，不过幸运的是菲涅尔方程可以用Fresnel-Schlick近似法求得近似解：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/428f0e29c23740ada8ab096a69279837.png)<br/><br/>
F0 表示平面的基础反射率，它是利用所谓折射指数(Indices of Refraction)或者说IOR计算得出的。然后正如你可以从球体表面看到的那样，我们越是朝球面掠角的方向上看（此时视线和表面法线的夹角接近90度）菲涅尔现象就越明显，反光就越强：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://learnopengl-cn.github.io/img/07/01/fresnel.png)<br/><br/>
#### Cook-Torrance反射率方程
***
随着Cook-Torrance BRDF中所有元素都介绍完毕，我们现在可以将基于物理的BRDF纳入到最终的反射率方程当中去了：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://img-blog.csdnimg.cn/18b463e15b454a2185ddbf03a1f02b52.png)<br/><br/>

#### PBR材质
***
在了解了PBR后面的数学模型之后，最后我们将通过说明美术师一般是如何编写一个我们可以直接输入PBR的平面物理属性的来结束这部分的讨论。PBR渲染管线所需要的每一个表面参数都可以用纹理来定义或者建模。使用纹理可以让我们逐个片段的来控制每个表面上特定的点对于光线是如何响应的：不论那个点是金属的，粗糙或者平滑，也不论表面对于不同波长的光会有如何的反应。<br/>

在下面你可以看到在一个PBR渲染管线当中经常会碰到的纹理列表，还有将它们输入PBR渲染器所能得到的相应的视觉输出：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://learnopengl-cn.github.io/img/07/01/textures.png)<br/><br/>
* **反照率**：反照率(Albedo)纹理为每一个金属的纹素(Texel)（纹理像素）指定表面颜色或者基础反射率。这和我们之前使用过的漫反射纹理相当类似，不同的是所有光照信息都是由一个纹理中提取的。漫反射纹理的图像当中常常包含一些细小的阴影或者深色的裂纹，而反照率纹理中是不会有这些东西的。它应该只包含表面的颜色（或者折射吸收系数）。<br/>

* **法线**：法线贴图纹理和我们之前在法线贴图教程中所使用的贴图是完全一样的。法线贴图使我们可以逐片段的指定独特的法线，来为表面制造出起伏不平的假象。<br/>

* **金属度**：金属(Metallic)贴图逐个纹素的指定该纹素是不是金属质地的。根据PBR引擎设置的不同，美术师们既可以将金属度编写为灰度值又可以编写为1或0这样的二元值。<br/>

* **粗糙度**：粗糙度(Roughness)贴图可以以纹素为单位指定某个表面有多粗糙。采样得来的粗糙度数值会影响一个表面的微平面统计学上的取向度。一个比较粗糙的表面会得到更宽阔更模糊的镜面反射（高光），而一个比较光滑的表面则会得到集中而清晰的镜面反射。某些PBR引擎预设采用的是对某些美术师来说更加直观的光滑度(Smoothness)贴图而非粗糙度贴图，不过这些数值在采样之时就马上用（1.0 – 光滑度）转换成了粗糙度。<br/>

* **AO**：环境光遮蔽(Ambient Occlusion)贴图或者说AO贴图为表面和周围潜在的几何图形指定了一个额外的阴影因子。比如如果我们有一个砖块表面，反照率纹理上的砖块裂缝部分应该没有任何阴影信息。然而AO贴图则会把那些光线较难逃逸出来的暗色边缘指定出来。在光照的结尾阶段引入环境遮蔽可以明显的提升你场景的视觉效果。网格/表面的环境遮蔽贴图要么通过手动生成，要么由3D建模软件自动生成。<br/>

美术师们可以在纹素级别设置或调整这些基于物理的输入值，还可以以现实世界材料的表面物理性质来建立他们的材质数据。这是PBR渲染管线最大的优势之一，因为不论环境或者光照的设置如何改变这些表面的性质是不会改变的，这使得美术师们可以更便捷的获取物理可信的结果。在PBR渲染管线中编写的表面可以非常方便的在不同的PBR渲染引擎间共享使用，不论处于何种环境中它们看上去都会是正确的，因此看上去也会更自然。<br/><br/>

在这个渲染器中，添加PBR材质的方式如下:<br/>
```bash
   device.material[1].have_diffuse = 0;
	//init_texture_by_diffuse(&device, "photo/brickwall.jpg",1);
	device.material[1].have_specular = 0;
	//init_texture_by_specular(&device, "photo/container2_specular.png", 1);
	device.material[1].have_normal = 1;

	init_texture_by_normal(&device, "photo/rustediron2_normal.png", 1);
	init_texture_by_albedo(&device, "photo/rustediron2_basecolor.png", 1);
	init_texture_by_metallic(&device, "photo/rustediron2_metallic.png", 1);
	init_texture_by_roughness(&device, "photo/rustediron2_roughness.png", 1);
```
计算DFG的函数如下<br/>
```bash
   //正态分布函数D
float DistributionGGX(s_vector& N, s_vector& H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(N.dotproduct(H), 0.0f);
	float NdotH2 = NdotH * NdotH;

	float nom = a2;
	float denom= (NdotH2 * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;

	return nom / denom;
}
//几何函数G
float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r * r) / 8.0;

	float nom = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(s_vector& N, s_vector& V, s_vector& L, float roughness)
{
	float NdotV = max(N.dotproduct(V), 0.0f);
	float NdotL = max(N.dotproduct(L), 0.0f);
	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

void fresnelSchlick(s_vector& result,float cosTheta,s_vector& F0)
{
	s_vector the_1(1.0f, 1.0f, 1.0f, 1.0f);
	s_vector tmp1; tmp1 = F0; tmp1.inverse(); tmp1.add(the_1);
	float clamp = CMID(1.0 - cosTheta, 0.0f, 1.0f);
	float tmp2 = pow(clamp, 5.0f);
	tmp1.float_dot(tmp2);
	
	result.add_two(tmp1, F0);
}

```


&emsp;&emsp;我们究竟将怎样表示场景上的辐照度(Irradiance), 辐射率(Radiance) L? 我们知道辐射率L（在计算机图形领域中）表示在给定立体角ω的情况下光源的辐射通量(Radiant flux)ϕ或光源在角度ω下发送出来的光能。 在我们的情况下，不妨假设立体角ω无限小，这样辐射度就表示光源在一条光线或单个方向向量上的辐射通量。<br/>

&emsp;&emsp;基于以上的知识，我们如何将其转化为以前的教程中积累的一些光照知识呢？ 那么想象一下，我们有一个点光源（一个光源在所有方向具有相同的亮度），它的辐射通量为用RBG表示为（23.47,21.31,20.79）。该光源的辐射强度(Radiant Intensity)等于其在所有出射光线的辐射通量。 然而，当我们为一个表面上的特定的点p着色时，在其半球领域Ω的所有可能的入射方向上，只有一个入射方向向量ωi直接来自于该点光源。 假设我们在场景中只有一个光源，位于空间中的某一个点，因而对于p点的其他可能的入射光线方向上的辐射率为0：<br/><br/>&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;&emsp;![图片](https://learnopengl-cn.github.io/img/07/02/lighting_radiance_direct.png)<br/><br/>
&emsp;&emsp;如果从一开始，我们就假设点光源不受光线衰减（光照强度会随着距离变暗）的影响，那么无论我们把光源放在哪，入射光线的辐射率总是一样的（除去入射角cosθ对辐射率的影响之外）。 正正是因为无论我们从哪个角度观察它，点光源总具有相同的辐射强度，我们可以有效地将其辐射强度建模为其辐射通量: 一个常量向量（23.47,21.31,20.79）。<br/>

&emsp;&emsp;然而，辐射率也需要将位置p作为输入，正如所有现实的点光源都会受光线衰减影响一样，点光源的辐射强度应该根据点p所在的位置和光源的位置以及他们之间的距离而做一些缩放。 因此，根据原始的辐射方程，我们会根据表面法向量n和入射角度wi来缩放光源的辐射强度。<br/>

&emsp;&emsp;在实现上来说：对于直接点光源的情况，辐射率函数L先获取光源的颜色值， 然后光源和某点p的距离衰减，接着按照n⋅wi缩放，但是仅仅有一条入射角为wi的光线打在点p上， 这个wi同时也等于在p点光源的方向向量。<br/>

代码表示如下:<br/>
```bash
   //do the light
		s_vector lightcolor; s_vector lightpos;
		lightcolor = device->pointlight[0].lightcolor;
		lightpos = device->pointlight[0].lightpos;

		s_vector L; L.minus_two(lightpos, WorldPos); L.normalize();
		s_vector H; H.add_two(V, L); H.normalize();

		float distance = L.length();
		float attenuation = 1.0f / (distance * distance);
		s_vector radiance; radiance = lightcolor; radiance.float_dot(attenuation);
```

