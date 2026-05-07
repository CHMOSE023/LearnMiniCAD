# MiniCAD 

## 项目概述
一个基于 Direct3D 11 的轻量级 CAD 原型系统，使用 C++ 和 Win32 构建。 

项目不是成为完整的 CAD 软件，而是用尽可能少的代码，把一个 CAD 系统该有的骨架——绘图、选择、Undo/Redo、图层、文件序列化，方便学习。
  
## 功能

- **直线绘制**：按 `L` 激活工具，左键连续点击绘制折线段，右键结束当前段，空格从上一个端点继续，ESC 退出
- **选择与高亮**：鼠标悬停高亮，左键点选，Ctrl+左键多选，Delete 删除选中实体
- **Undo / Redo**：Ctrl+Z / Ctrl+Y，基于命令模式，支持添加和删除的完整撤销
- **视口导航**：鼠标中键拖拽平移，滚轮以鼠标为中心缩放，自适应动态网格
- **文件序列化**：Ctrl+S 保存为 `.mcad`（二进制格式），Ctrl+O 打开文件对话框加载，支持几何数据、颜色、图层属性的完整读写
- **图层管理**：图层数据结构完整（名称、颜色、可见性、锁定），序列化已接入
   
## 构建

```bash
git clone https://github.com/CHMOSE023/LearnMiniCAD
 ```

**用 Visual Studio 2022 及以上版本打开**  `CMakeLists.txt`。
  

## 工程目录
 
```
src/
├── App/ 
│   ├── Command/            操作命令（AddEntityCommand、DeleteEntityCommand、LayerCommands）
│   ├── CommandStack/       命令管理（CommandStack）
│   ├── Document/           文档管理（Scene、Layer、EntityContainer）
│   ├── Editor/             场景编辑（选择、工具调度、快捷键、框选）
│   ├── Grip/               拖动编辑（Grip、GripManager） 
│   ├── Input/              消息构建 (InputManager、MessageTranslator)
│   ├── Overlay/            实时预览 (Overlay.h)
│   ├── Picking/            选择图元（Picking.h） 
│   ├── Scene/              场景容器 (Scene.h Layer.h LayerManager.h)
│   ├── Snap/               捕捉夹点 (SnapEngine.h SnapResult.h)
│   ├── Tools/              绘图工具 (LineTool.h PointTool.h)
│   ├── UI/                 窗口组件（UIManager.h/cpp）  
│   ├── Main.cpp            程序入口 (main方法入口)
│   └── MainWindow.h/cpp    程序窗口 (WinMain窗口类、消息循环、UI布局)
│    
├── Core/
│   ├── Entity/             实体定义（LineEntity、EntityAttr）
│   ├── GeomKernel/         几何基础（Line、AABB）
│   └── Object/             对象基类 (RTTI 系统、ObjectFactory )
│    
└── Render/
    ├── D3D11/              图像渲染（Device、SwapChain、Renderer、RenderTarget、Basic.hlsl）
    ├── Viewport/           视图视口（Viewport、Camera（正交投影）、Grid（动态网格））
    └── ViewState           视图状态  
``` 

## 交流

视频: https://www.bilibili.com/video/BV18HQGB7Ekc

QQ群: 1090567431