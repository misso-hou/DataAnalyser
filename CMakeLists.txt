cmake_minimum_required(VERSION 3.12)

project(test)

#外部库加载
find_package(Python3 COMPONENTS Interpreter Development)
find_package(pybind11 REQUIRED)
find_package(matplotlibcpp17 REQUIRED)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# option(ENABLE_DISPLAY_TOOL "enable display option" ON)

# #指定路径信息
# set(PROJECT_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin")
# file(MAKE_DIRECTORY ${PROJECT_OUTPUT_DIRECTORY})
# set(CSV_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin/csvPlt")
# file(MAKE_DIRECTORY ${CSV_OUTPUT_DIRECTORY})
# # 在 CMakeLists.txt 中定义预处理宏，传递工程根目录路径
# add_definitions(-DCSV_SOURCE_DIR="${CSV_OUTPUT_DIRECTORY}")
# add_definitions(-DPROJECT_DIR="${CMAKE_SOURCE_DIR}")


#在指定位置生成执行程序
add_executable(${PROJECT_NAME} src/data_play.cpp src/animation.cpp)
# set_target_properties(${PROJECT_NAME} PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY "${PROJECT_OUTPUT_DIRECTORY}"
# )

#链接自身头文件
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

#外部库链接
target_link_libraries(${PROJECT_NAME} ${Python3_LIBRARIES} pybind11::embed
                      matplotlibcpp17::matplotlibcpp17)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

#链接其他头文件
target_include_directories(${PROJECT_NAME} PUBLIC  ${Python3_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/utilities/include)
