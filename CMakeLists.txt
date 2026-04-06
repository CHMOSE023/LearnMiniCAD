cmake_minimum_required(VERSION 3.16)
project(LearnMiniCAD) 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_definitions("-DUNICODE" "-D_UNICODE")  # Windows API 使用宽字符版本
    add_compile_options(/utf-8)                # 源文件按 UTF-8 解析
endif()

find_package(directxtk CONFIG REQUIRED)

# 1.源码根目录
set(LEARN_DIR "${CMAKE_SOURCE_DIR}/source")

# 2.获取Source下的所有文件夹放在MODULE_DIRS变量中， GLOB当前目录，不含子目录
file(GLOB MODULE_DIRS RELATIVE ${LEARN_DIR} ${LEARN_DIR}/*)

# 3.遍历 MODULE_DIRS 为每个文件夹生成 exe 文件
foreach(MODULE_DIR ${MODULE_DIRS})  
    # 3.1 当前模块路径
    set(MODULE_PATH ${LEARN_DIR}/${MODULE_DIR})

    # 3.2 收集所有*cpp；GLOB_RECURSE 包含子目录 
    file(GLOB_RECURSE SOURCE_FILES  CONFIGURE_DEPENDS  ${MODULE_PATH}/*.cpp  )

    # 3.3 创建可执行程序
    add_executable(${MODULE_DIR}   ${SOURCE_FILES})

    # 3.4 每个模块独立 include 目录（关键点）
    target_include_directories(${MODULE_DIR}  PRIVATE ${LEARN_DIR}/)

    # 3.5 连接静态库
    target_link_libraries(${MODULE_DIR}   PRIVATE  Microsoft::DirectXTK d3d11 dxgi) 

    # 3.5 输出目录
    set_target_properties(${MODULE_DIR}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${MODULE_DIR}
    )

    # 3.6 资产文件，主要是着色器程序
    file(GLOB_RECURSE ASSETS  ${MODULE_PATH}/*.hlsl)

    # 3.7 拷贝资源
    if(ASSETS)
        add_custom_command(TARGET ${MODULE_DIR} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${ASSETS}
                $<TARGET_FILE_DIR:${MODULE_DIR}>
            COMMENT "Copy assets for ${MODULE_DIR}"
        )
    endif()

endforeach()
