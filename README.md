# LearnMiniCAD 

## 项目概述

项目不是成为完整的 CAD 软件，而是用尽可能少的代码，把一个 CAD 系统该有的骨架——绘图、选择、Undo/Redo、图层、文件序列化，方便学习。

## vcpkg 安装依赖

- 获取 vcpkg 包管理器

```bash
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
bootstrap-vcpkg.bat
```

- 安装 DirectXTK 库

```bash
vcpkg install directxtk:x64-windows
```

## 工程目录

```
LearnMiniCAD/
│
├── source/
│   ├── Lesson01-Win32/            # 教程 01: Win32初始化
│   ├── Lesson02-InitD3D11/        # 教程 02: D3D11初始化
│   ├── Lesson03-Viewport/         # 教程 03: 视口与相机
│   ├── Lesson04-Renderer/         # 教程 04: Shader着色器
│   ├── Lesson05-Core/             # 教程 05: 几何内核
│   ├── Lesson06-Scene/            # 教程 06: 场景与图层
│   ├── Lesson07-Input/            # 教程 07: 消息系统与责任链
│   ├── Lesson08-CommandStack      # 教程 08: 命令堆栈
│   ├── Lesson09-Editor/           # 教程 09: 交互控制器
│   ├── Lesson10-Document/         # 教程 10: 文档
│   ├── Lesson11-Command/          # 教程 11: 实现添加、删除命令，支持撤销重做
│   ├── Lesson12-Preview/          # 教程 12: 实时绘制预览
│   ├── Lesson13-Picking/          # 教程 13: 实时选择实体
│   ├── Lesson14-ViewState/        # 教程 14: Viewport中的视图状态
│   ├── Lesson15-RenderPass/       # 教程 15: 分批渲染光标、轴网、实体类
│   ├── Lesson16-Grip/             # 教程 16: 实体夹点计算
│   ├── Lesson17-Snap/             # 教程 17: 实体最近点
│   ├── Lesson18-Drag/             # 教程 18: 拖拽编辑
│   ├── Lesson19-Constraint/       # 教程 19: 实现正交约束
│   ├── Lesson20-ImGui/            # 教程 20: ImGui 初始化
│   └── Lesson21-Widgets/          # 教程 21: ImGui 封装UI组件
│
├── thirdParty/                    # 第三方依赖
│   │
│   └──imgui/                      # imgui
│
└── CMakeLists.txt

```
