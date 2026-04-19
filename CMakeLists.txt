cmake_minimum_required(VERSION 3.16)
project(LearnMiniCAD) 

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_definitions("-DUNICODE" "-D_UNICODE")                                   # Windows API 使用宽字符版本
    add_compile_options(/utf-8)                                                 # 源文件按 UTF-8 解析
endif()
    
add_library(imgui
    thirdParty/imgui/imgui.cpp
    thirdParty/imgui/imgui_draw.cpp
    thirdParty/imgui/imgui_widgets.cpp
    thirdParty/imgui/imgui_tables.cpp
    thirdParty/imgui/backends/imgui_impl_win32.cpp
    thirdParty/imgui/backends/imgui_impl_dx11.cpp
)

target_include_directories(imgui PUBLIC
    thirdParty/imgui
    thirdParty/imgui/backends
)

set(LESSON_ROOT "${CMAKE_SOURCE_DIR}/source")                                   # 1.源码根目录
                                                                                
file(GLOB LESSON_NAMES RELATIVE ${LESSON_ROOT} ${LESSON_ROOT}/*)                # 2.获取每课程的目录名（即子文件夹名），GLOB仅包括子目录
                                                                                
foreach(LESSON_NAME ${LESSON_NAMES})                                            # 3.遍历 LESSON_NAMES 为每个课程生成独立的可执行文件

    set(LESSON_DIR "${LESSON_ROOT}/${LESSON_NAME}")                             # 3.1 当前课程的完整路径   
    
    file(GLOB_RECURSE LESSON_SOURCES CONFIGURE_DEPENDS "${LESSON_DIR}/*.cpp")   # 3.2 收集课程下所有 .cpp 源文件（含子目录），GLOB_RECURSE 递归子目录   
   
    if(LESSON_SOURCES)  
        add_executable(${LESSON_NAME} ${LESSON_SOURCES})                        # 3.3 以课程名创建可执行目标
   
        target_include_directories(${LESSON_NAME} PRIVATE "${LESSON_DIR}/")     # 3.4 课程私有头文件目录

        target_link_libraries(${LESSON_NAME} PRIVATE                            # 3.5 链接所需库 
            d3d11 dxgi d3dcompiler  imgui
        ) 

        set_target_properties(${LESSON_NAME} PROPERTIES                         # 3.6 可执行文件输出目录
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${LESSON_NAME}"
        )

        file(GLOB_RECURSE HLSL_SHADERS "${LESSON_DIR}/*.hlsl")                  # 3.7 收集课程下的 HLSL 着色器文件
 
        if(HLSL_SHADERS)                                                        # 3.8 构建后将着色器拷贝到输出目录
            add_custom_command(TARGET ${LESSON_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    ${HLSL_SHADERS}
                    $<TARGET_FILE_DIR:${LESSON_NAME}>
                COMMENT "Copying HLSL shaders for ${LESSON_NAME}"
            )
        endif()
    endif()
endforeach()
