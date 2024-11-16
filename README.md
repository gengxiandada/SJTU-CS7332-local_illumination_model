# README

## 项目概述
本课程项目依据如下公式:

$I_\lambda = I_{a \lambda}k_a + \sum_{1 \leq i \leq m} S_i[k_d L_{d\lambda}(l \cdot n) + k_s L_{s \lambda}(r \cdot v)^n], S_i = \frac{1}{a + b d_i + c {d_i}^2}$

实现了一个局部光照模型。

本光照模型可包含自定义数量的点光源，实现了镜面反射、漫反射、环境光、阴影、半透明物体等功能。相关系数可自定义，物体在场景中的布局可由scene.txt文件自定义，光线信息可在lightPos.pos文件中自定义。场景视角可由键盘控制前后左右(W,S,A,D)移动，以及上下左右(up,down,left,right)的转动。

## local_illumination_model目录结构
```sh
.
├── bin
├── build
├── include
│   ├── GL      // OpenGL扩展
│   ├── GLFW    // 用于创建和管理窗口、上下文及处理输入
│   ├── glm     // 提供了常用的数学运算功能，如向量和矩阵的运算、变换（旋转、缩放、平移等），以及投影矩阵和视图矩阵的计算
│   ├── KHR     // 定义了平台无关的数据类型和宏，以确保代码的可移植性
│   ├── FrameBuffer.h
│   ├── IndexBuffer.h
│   ├── Lights.h
│   ├── Renderer.h
│   ├── Shader.h
│   ├── Texture.h
│   ├── tiny_obj_loader.h     // 用于加载3D模型文件的C++头文件库
│   ├── VertexArray
│   ├── VertexBuffer.h
│   └── VertexBufferLayout.h
├── lib
│   ├── libGLEW.2.2.dylib
│   ├── libglfw.3.dylib
│   └── libglfw3.a
├── res
│   ├── objects       // 3D模型文件
│   ├── shaders       // 着色器
│   ├── textures      // 纹理贴图
│   └── lightPos.pos  // 光源位置信息
├── src
│   └── vendor
│       └── stb_image      // 用于图像加载和处理
│           ├── stb_image.cpp
│           └── stb_image.h
│   ├──FrameBuffer.cpp           // 帧缓冲区类
│   ├──IndexBuffer.cpp           // 索引缓冲区类
│   ├──Lights.cpp                // 光源类
│   ├──Demo.cpp                  // "https://www.bilibili.com/video/BV1MJ411u7Bc/?spm_id_from=333.337.search-card.all.click&vd_source=5f44bbaeca42514008ef3db14ea107cf"中的示例程序
│   ├──Renderer.cpp              // 渲染器类
│   ├──Texture.cpp               // 纹理（深度贴图）类
│   ├──utils.cpp                 // 辅助函数
│   ├──VertexArray.cpp           // 顶点数组类
│   ├──VertexBuffer.cpp          // 顶点缓冲区类
│   └──VertexBufferLayout.cpp    // 顶点缓冲布局类
├── Application.cpp
├── CMakeLists.txt
└── README.md
```

## 开发环境
本项目使用 C++ 基于 macOS-arm64 平台开发，使用 OpenGL4.1 及相关扩展库，使用 cmake 进行项目管理。

### clang 版本 
```sh
Apple clang version 15.0.0 (clang-1500.3.9.4) 
```

### cmake 版本
```sh
cmake version 3.28.2
```

## 直接运行
### macOS
```sh
$ mkdir build
$ cd build
$ cmake ..
$ make App
$ ../bin/App
```

## 调整视角

### 相机位置移动
W: 上，S: 下，A: 左，D:右

### 相机视角转动
up: 下，down: 上，left: 左，right:右

## 场景布局修改可通过自定义scene.txt文件实现

## 参考
github项目：https://github.com/lym01803/toy-local-illumination-model

视频教程：https://www.bilibili.com/video/BV1MJ411u7Bc/?spm_id_from=333.337.search-card.all.click&vd_source=5f44bbaeca42514008ef3db14ea107cf