cmake_minimum_required(VERSION 3.4.1)

# include头文件目录
include_directories(src/main/cpp/include/
                    src/main/cpp/)

# source directory源文件目录
# 手写数字识别
file(GLOB MAIN_SRC src/main/cpp/*.h
                    src/main/cpp/*.cpp)
set(MAIN_COMPILE_CODE ${MAIN_SRC})

# 添加ncnn库
add_library(libncnn STATIC IMPORTED )
set_target_properties(libncnn
  PROPERTIES IMPORTED_LOCATION
  ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libncnn.a)

# 添加opencv库
add_library(libopencv_java4 SHARED IMPORTED)
set_target_properties(libopencv_java4 PROPERTIES IMPORTED_LOCATION
       ${CMAKE_SOURCE_DIR}/src/main/jniLibs/${ANDROID_ABI}/libopencv_java4.so)

#编译为动态库
add_library(HwDr SHARED ${MAIN_COMPILE_CODE})

#vulkan
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -Werror -D VK_USE_PLATFORM_ANDROID_KHR")

#添加工程所依赖的库
find_library(log-lib log android)
target_link_libraries( HwDr
                       libncnn
                       libopencv_java4
                       jnigraphics
#                       vulkan
                       z
                       ${log-lib}
                       android )
