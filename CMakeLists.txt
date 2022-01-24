cmake_minimum_required(VERSION 2.8)

project(WaterSurface)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src/)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib/)

set(SRC_SHADER 
    ${SRC_DIR}shaders/simple.vert
    ${SRC_DIR}shaders/simple.frag

    ${SRC_DIR}shaders/sinewave.vert
    ${SRC_DIR}shaders/sinewave.frag
    ${SRC_DIR}shaders/height_map.frag
    ${SRC_DIR}shaders/height_map.vert
    ${SRC_DIR}shaders/skybox.vert
    ${SRC_DIR}shaders/skybox.frag
    ${SRC_DIR}shaders/tiles.vert
    ${SRC_DIR}shaders/tiles.frag

    ${SRC_DIR}shaders/vertex.vert
    ${SRC_DIR}shaders/drop.frag
    ${SRC_DIR}shaders/update.frag
    ${SRC_DIR}shaders/normal.frag
    ${SRC_DIR}shaders/water.vert
)

set(SRC_RENDER_UTILITIES
    ${SRC_DIR}RenderUtilities/BufferObject.h
    ${SRC_DIR}RenderUtilities/Shader.h
    ${SRC_DIR}RenderUtilities/Texture.h
    ${SRC_DIR}RenderUtilities/Vertex.h
)

include_directories(${INCLUDE_DIR})
include_directories(${INCLUDE_DIR}glad4.6/include/)
include_directories(${INCLUDE_DIR}glm-0.9.8.5/glm/)

add_Definitions("-D_XKEYCHECK_H")
add_definitions(-DPROJECT_DIR="${PROJECT_SOURCE_DIR}")

add_executable(WaterSurface
    ${SRC_DIR}mesh.h
    ${SRC_DIR}model.h
    ${SRC_DIR}water.h

    ${SRC_DIR}CallBacks.h
    ${SRC_DIR}ControlPoint.h
    ${SRC_DIR}Object.h
    ${SRC_DIR}Track.h
    ${SRC_DIR}WaterView.h
    ${SRC_DIR}TrainWindow.h

    ${SRC_DIR}main.cpp
    ${SRC_DIR}CallBacks.cpp
    ${SRC_DIR}ControlPoint.cpp
    ${SRC_DIR}Track.cpp
    ${SRC_DIR}WaterView.cpp
    ${SRC_DIR}TrainWindow.cpp

    ${SRC_SHADER}
    ${SRC_RENDER_UTILITIES}

    ${INCLUDE_DIR}glad4.6/src/glad.c
    ${INCLUDE_DIR}stb_image.h

)
source_group("shaders" FILES ${SRC_SHADER})
source_group("RenderUtilities" FILES ${SRC_RENDER_UTILITIES})


add_library(Utilities 
    ${SRC_DIR}Utilities/ArcBallCam.h
    ${SRC_DIR}Utilities/3DUtils.h
    ${SRC_DIR}Utilities/Pnt3f.h
    ${SRC_DIR}Utilities/ArcBallCam.cpp
    ${SRC_DIR}Utilities/3DUtils.cpp
    ${SRC_DIR}Utilities/Pnt3f.cpp)

target_link_libraries(WaterSurface 
    debug ${LIB_DIR}Debug/fltk_formsd.lib      optimized ${LIB_DIR}Release/fltk_forms.lib
    debug ${LIB_DIR}Debug/fltk_gld.lib         optimized ${LIB_DIR}Release/fltk_gl.lib
    debug ${LIB_DIR}Debug/fltk_imagesd.lib     optimized ${LIB_DIR}Release/fltk_images.lib
    debug ${LIB_DIR}Debug/fltk_jpegd.lib       optimized ${LIB_DIR}Release/fltk_jpeg.lib
    debug ${LIB_DIR}Debug/fltk_pngd.lib        optimized ${LIB_DIR}Release/fltk_png.lib
    debug ${LIB_DIR}Debug/fltk_zd.lib          optimized ${LIB_DIR}Release/fltk_z.lib
    debug ${LIB_DIR}Debug/fltkd.lib            optimized ${LIB_DIR}Release/fltk.lib
    debug ${LIB_DIR}Debug/opencv_world341d.lib optimized ${LIB_DIR}Release/opencv_world341.lib
    
    debug ${LIB_DIR}Debug/assimp-vc142-mt.lib  optimized ${LIB_DIR}Release/assimp-vc142-mt.lib
 )
    
target_link_libraries(WaterSurface 
    ${LIB_DIR}OpenGL32.lib
    ${LIB_DIR}glu32.lib
    ${LIB_DIR}common.lib
    ${LIB_DIR}ex-common.lib
    ${LIB_DIR}OpenAL32.lib
    ${LIB_DIR}alut.lib
    ${LIB_DIR}alut_static.lib
    ${LIB_DIR}assimp-vc142-mt.lib
   
 )

target_link_libraries(WaterSurface Utilities)

file(COPY 
    ${LIB_DIR}dll/alut.dll
    ${LIB_DIR}dll/OpenAL32.dll
    ${LIB_DIR}dll/opencv_world341.dll
    ${LIB_DIR}dll/opencv_world341d.dll
    ${LIB_DIR}dll/assimp-vc142-mt.dll
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    